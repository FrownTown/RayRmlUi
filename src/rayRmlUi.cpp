#include "rayRmlUi.h"

#include "RmlRendererRaylib.h"
#include "RmlSystemRaylib.h"
#include "RmlInputRaylib.h"

#include <RmlUi/Core.h>

static rayrmlui::RmlRendererRaylib* g_renderer = nullptr;
static rayrmlui::RmlSystemRaylib*   g_system   = nullptr;
static bool                         g_ready    = false;

bool InitRmlUi(int /*width*/, int /*height*/) {
    g_renderer = new rayrmlui::RmlRendererRaylib();
    g_system   = new rayrmlui::RmlSystemRaylib();
    Rml::SetRenderInterface(g_renderer);
    Rml::SetSystemInterface(g_system);
    g_ready = Rml::Initialise();
    return g_ready;
}

void CloseRmlUi(void) {
    Rml::Shutdown();
    delete g_renderer; g_renderer = nullptr;
    delete g_system;   g_system   = nullptr;
    g_ready = false;
}

bool IsRmlUiReady(void) { return g_ready; }

// ── Input forwarding (delegates to RmlInputRaylib) ───────────────────────────

RmlInputResult PumpRmlUiInput(Rml::Context& ctx, bool mouse_over_ui) {
    auto r = rayrmlui::pumpRmlInput(ctx, mouse_over_ui);
    return RmlInputResult{ r.moved, r.discrete };
}

int GetRmlUiKeyModifiers() {
    return rayrmlui::currentKeyModifiers();
}

Rml::Input::KeyIdentifier TranslateRaylibKey(int key) {
    return rayrmlui::translateRaylibKey(key);
}
