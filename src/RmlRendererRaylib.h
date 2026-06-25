#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <unordered_map>
#include <vector>

namespace rayrmlui {

class RmlRendererRaylib final : public Rml::RenderInterface {
public:
    RmlRendererRaylib();
    ~RmlRendererRaylib() override;

    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                                Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
                        Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                   const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source,
                                       Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

private:
    struct GeomData {
        unsigned int vao   = 0;
        unsigned int vbo   = 0;
        unsigned int ibo   = 0;
        int          count = 0;
    };
    std::unordered_map<Rml::CompiledGeometryHandle, GeomData> geom_;
    Rml::CompiledGeometryHandle next_geom_id_ = 1;

    unsigned int shader_id_     = 0;
    int          loc_mvp_       = -1;
    int          loc_translate_ = -1;
    int          loc_tex_       = -1;
    int          loc_use_tex_   = -1;
    int          attr_pos_      = -1;
    int          attr_uv_       = -1;
    int          attr_col_      = -1;
    bool         shader_inited_ = false;
    int          diag_compiles_ = 0;
    int          diag_renders_  = 0;

    void ensureShader();
};

}
