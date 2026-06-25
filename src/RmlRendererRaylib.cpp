#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"

#include "RmlRendererRaylib.h"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include "external/glad.h"

#include <cstddef>
#ifdef DEBUG_RML_RENDER_LOG
#include <cstdarg>
#include <cstdio>
#endif
#include <cstring>
#include <vector>

namespace rayrmlui {

namespace {

#ifdef DEBUG_RML_RENDER_LOG
void dlog(const char* fmt, ...) {
    static FILE* f = nullptr;
    if (!f) {
        f = std::fopen("rml_render.log", "w");
        if (!f) return;
    }
    va_list ap;
    va_start(ap, fmt);
    std::vfprintf(f, fmt, ap);
    va_end(ap);
    std::fputc('\n', f);
    std::fflush(f);
}
#else
void dlog(const char*, ...) {}
#endif

const char* kVertShader = R"(#version 330 core
in vec2 a_pos;
in vec2 a_uv;
in vec4 a_col;
out vec2 v_uv;
out vec4 v_col;
uniform mat4 u_mvp;
uniform vec2 u_translate;
void main() {
    v_uv  = a_uv;
    v_col = a_col;
    gl_Position = u_mvp * vec4(a_pos + u_translate, 0.0, 1.0);
}
)";

const char* kFragShader = R"(#version 330 core
in vec2 v_uv;
in vec4 v_col;
out vec4 frag;
uniform sampler2D u_tex;
uniform int u_use_tex;
void main() {
    vec4 t = (u_use_tex != 0) ? texture(u_tex, v_uv) : vec4(1.0);
    frag = t * v_col;
}
)";

void checkGLError(const char* tag) {
    GLenum e = glGetError();
    while (e != GL_NO_ERROR) {
        dlog("[GL] %s : 0x%04x", tag, (unsigned)e);
        e = glGetError();
    }
}

}  // namespace

RmlRendererRaylib::RmlRendererRaylib() = default;

RmlRendererRaylib::~RmlRendererRaylib() {
    for (auto& entry : geom_) {
        auto& g = entry.second;
        if (g.vao) rlUnloadVertexArray(g.vao);
        if (g.vbo) rlUnloadVertexBuffer(g.vbo);
        if (g.ibo) rlUnloadVertexBuffer(g.ibo);
    }
    geom_.clear();
    if (shader_id_) rlUnloadShaderProgram(shader_id_);
}

void RmlRendererRaylib::ensureShader() {
    if (shader_inited_) return;
    shader_inited_ = true;

    dlog("sizeof(Rml::Vertex) = %d", (int)sizeof(Rml::Vertex));
    dlog("  offsetof position = %d", (int)offsetof(Rml::Vertex, position));
    dlog("  offsetof colour   = %d", (int)offsetof(Rml::Vertex, colour));
    dlog("  offsetof tex_coord= %d", (int)offsetof(Rml::Vertex, tex_coord));

    checkGLError("before-shader-compile");
    unsigned int vs = rlCompileShader(kVertShader, RL_VERTEX_SHADER);
    unsigned int fs = rlCompileShader(kFragShader, RL_FRAGMENT_SHADER);
    dlog("compile vs=%u fs=%u", vs, fs);
    if (!vs || !fs) return;

    shader_id_ = rlLoadShaderProgram(vs, fs);
    dlog("linked shader id=%u", shader_id_);
    if (!shader_id_) return;

    loc_mvp_       = rlGetLocationUniform(shader_id_, "u_mvp");
    loc_translate_ = rlGetLocationUniform(shader_id_, "u_translate");
    loc_tex_       = rlGetLocationUniform(shader_id_, "u_tex");
    loc_use_tex_   = rlGetLocationUniform(shader_id_, "u_use_tex");
    attr_pos_      = glGetAttribLocation(shader_id_, "a_pos");
    attr_uv_       = glGetAttribLocation(shader_id_, "a_uv");
    attr_col_      = glGetAttribLocation(shader_id_, "a_col");

    dlog("uniforms mvp=%d translate=%d tex=%d use_tex=%d",
         loc_mvp_, loc_translate_, loc_tex_, loc_use_tex_);
    dlog("attribs  pos=%d uv=%d col=%d",
         attr_pos_, attr_uv_, attr_col_);
    checkGLError("after-shader-link");
}

Rml::CompiledGeometryHandle RmlRendererRaylib::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
    ensureShader();
    const auto handle = next_geom_id_++;
    auto& g = geom_[handle];
    g.count = (int)indices.size();
    if (g.count == 0 || vertices.empty() || shader_id_ == 0) return handle;

    bool diag = diag_compiles_ < 2;
    if (diag) {
        ++diag_compiles_;
        dlog("CompileGeometry #%d  verts=%d  idx=%d",
             diag_compiles_, (int)vertices.size(), g.count);
        for (size_t i = 0; i < vertices.size() && i < 3; ++i) {
            const auto& v = vertices[i];
            dlog("  v[%zu] pos=(%.2f,%.2f) uv=(%.4f,%.4f) col=(%u,%u,%u,%u)",
                 i, v.position.x, v.position.y, v.tex_coord.x, v.tex_coord.y,
                 v.colour.red, v.colour.green, v.colour.blue, v.colour.alpha);
            const unsigned char* raw = reinterpret_cast<const unsigned char*>(&v);
            dlog("  v[%zu] raw: "
                 "%02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x",
                 i, raw[0],raw[1],raw[2],raw[3], raw[4],raw[5],raw[6],raw[7],
                 raw[8],raw[9],raw[10],raw[11],
                 raw[12],raw[13],raw[14],raw[15], raw[16],raw[17],raw[18],raw[19]);
        }
        for (size_t i = 0; i < indices.size() && i < 6; ++i)
            dlog("  idx[%zu] = %d", i, indices[i]);
    }

    g.vao = rlLoadVertexArray();
    rlEnableVertexArray(g.vao);

    const int stride = (int)sizeof(Rml::Vertex);
    g.vbo = rlLoadVertexBuffer(vertices.data(),
                               (int)(vertices.size() * sizeof(Rml::Vertex)),
                               false);

    if (attr_pos_ >= 0) {
        rlSetVertexAttribute((unsigned)attr_pos_, 2, RL_FLOAT, false, stride,
                             (int)offsetof(Rml::Vertex, position));
        rlEnableVertexAttribute((unsigned)attr_pos_);
    }
    if (attr_uv_ >= 0) {
        rlSetVertexAttribute((unsigned)attr_uv_, 2, RL_FLOAT, false, stride,
                             (int)offsetof(Rml::Vertex, tex_coord));
        rlEnableVertexAttribute((unsigned)attr_uv_);
    }
    if (attr_col_ >= 0) {
        rlSetVertexAttribute((unsigned)attr_col_, 4, RL_UNSIGNED_BYTE, true, stride,
                             (int)offsetof(Rml::Vertex, colour));
        rlEnableVertexAttribute((unsigned)attr_col_);
    }

    g.ibo = rlLoadVertexBufferElement(indices.data(),
                                      (int)(indices.size() * sizeof(int)),
                                      false);

    rlDisableVertexArray();
    if (diag) {
        dlog("  vao=%u vbo=%u ibo=%u  stride=%d",
             g.vao, g.vbo, g.ibo, stride);
        checkGLError("after-CompileGeometry");
    }
    return handle;
}

void RmlRendererRaylib::RenderGeometry(Rml::CompiledGeometryHandle geometry,
                                       Rml::Vector2f translation,
                                       Rml::TextureHandle texture) {
    auto it = geom_.find(geometry);
    if (it == geom_.end()) return;
    const auto& g = it->second;
    if (g.vao == 0 || g.count == 0 || shader_id_ == 0) return;

    bool diag = diag_renders_ < 3;
    if (diag) {
        ++diag_renders_;
        dlog("RenderGeometry #%d  vao=%u count=%d  trans=(%.1f,%.1f) tex=%u",
             diag_renders_, g.vao, g.count, translation.x, translation.y,
             (unsigned)texture);
    }

    rlDrawRenderBatchActive();
    if (diag) checkGLError("after-flush");

    Matrix mv  = rlGetMatrixModelview();
    Matrix prj = rlGetMatrixProjection();
    Matrix mvp = MatrixMultiply(mv, prj);

    if (diag) {
        dlog("  mvp row0 = %.3f %.3f %.3f %.3f", mvp.m0, mvp.m4, mvp.m8, mvp.m12);
        dlog("  mvp row1 = %.3f %.3f %.3f %.3f", mvp.m1, mvp.m5, mvp.m9, mvp.m13);
        dlog("  mvp row2 = %.3f %.3f %.3f %.3f", mvp.m2, mvp.m6, mvp.m10, mvp.m14);
        dlog("  mvp row3 = %.3f %.3f %.3f %.3f", mvp.m3, mvp.m7, mvp.m11, mvp.m15);
    }

    rlEnableShader(shader_id_);
    rlSetUniformMatrix(loc_mvp_, mvp);
    float t[2] = { translation.x, translation.y };
    rlSetUniform(loc_translate_, t, RL_SHADER_UNIFORM_VEC2, 1);

    const unsigned int tex_id =
        texture ? (unsigned int)texture : rlGetTextureIdDefault();
    int use_tex = 1;
    rlActiveTextureSlot(0);
    rlEnableTexture(tex_id);
    int tex_unit = 0;
    rlSetUniform(loc_tex_, &tex_unit, RL_SHADER_UNIFORM_INT, 1);
    rlSetUniform(loc_use_tex_, &use_tex, RL_SHADER_UNIFORM_INT, 1);

    if (diag) checkGLError("before-draw");

    rlEnableVertexArray(g.vao);
    glDrawElements(GL_TRIANGLES, g.count, GL_UNSIGNED_INT, nullptr);
    rlDisableVertexArray();

    if (diag) checkGLError("after-draw");

    rlDisableTexture();
    rlDisableShader();
}

void RmlRendererRaylib::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    auto it = geom_.find(geometry);
    if (it == geom_.end()) return;
    auto& g = it->second;
    if (g.vao) rlUnloadVertexArray(g.vao);
    if (g.vbo) rlUnloadVertexBuffer(g.vbo);
    if (g.ibo) rlUnloadVertexBuffer(g.ibo);
    geom_.erase(it);
}

Rml::TextureHandle RmlRendererRaylib::LoadTexture(Rml::Vector2i& texture_dimensions,
                                                  const Rml::String& source) {
    auto ext = source.rfind('.');
    if (ext != Rml::String::npos && source.substr(ext) == ".svg") {
        NSVGimage* svg = nsvgParseFromFile(source.c_str(), "px", 96.0f);
        if (!svg) return 0;
        const int w = (int)(svg->width  * 2.0f);
        const int h = (int)(svg->height * 2.0f);
        NSVGrasterizer* rast = nsvgCreateRasterizer();
        std::vector<unsigned char> px(w * h * 4, 0);
        nsvgRasterize(rast, svg, 0, 0, 2.0f, px.data(), w, h, w * 4);
        nsvgDeleteRasterizer(rast);
        nsvgDelete(svg);
        for (int i = 0; i < w * h; ++i) {
            const unsigned char a = px[i * 4 + 3];
            px[i * 4 + 0] = (unsigned char)((px[i * 4 + 0] * a) / 255u);
            px[i * 4 + 1] = (unsigned char)((px[i * 4 + 1] * a) / 255u);
            px[i * 4 + 2] = (unsigned char)((px[i * 4 + 2] * a) / 255u);
        }
        Image img{};
        img.data    = px.data();
        img.width   = w;
        img.height  = h;
        img.mipmaps = 1;
        img.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        texture_dimensions = {w, h};
        Texture2D tex = LoadTextureFromImage(img);
        if (tex.id == 0) return 0;
        SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
        return static_cast<Rml::TextureHandle>(tex.id);
    }

    Image img = LoadImage(source.c_str());
    if (img.data == nullptr) return 0;
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageAlphaPremultiply(&img);
    Texture2D tex = LoadTextureFromImage(img);
    texture_dimensions.x = img.width;
    texture_dimensions.y = img.height;
    UnloadImage(img);
    if (tex.id == 0) return 0;
    return static_cast<Rml::TextureHandle>(tex.id);
}

Rml::TextureHandle RmlRendererRaylib::GenerateTexture(Rml::Span<const Rml::byte> source,
                                                      Rml::Vector2i source_dimensions) {
    Image img{};
    img.data = const_cast<Rml::byte*>(source.data());
    img.width = source_dimensions.x;
    img.height = source_dimensions.y;
    img.mipmaps = 1;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    Texture2D tex = LoadTextureFromImage(img);
    dlog("GenerateTexture id=%u  size=%dx%d", tex.id,
         source_dimensions.x, source_dimensions.y);
    if (tex.id == 0) return 0;
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    return static_cast<Rml::TextureHandle>(tex.id);
}

void RmlRendererRaylib::ReleaseTexture(Rml::TextureHandle texture) {
    if (!texture) return;
    Texture2D t{};
    t.id = static_cast<unsigned int>(texture);
    UnloadTexture(t);
}

void RmlRendererRaylib::EnableScissorRegion(bool enable) {
    if (!enable) EndScissorMode();
}

void RmlRendererRaylib::SetScissorRegion(Rml::Rectanglei region) {
    BeginScissorMode(region.Left(), region.Top(), region.Width(), region.Height());
}

}
