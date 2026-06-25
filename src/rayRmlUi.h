#pragma once
// rayRmlUi is a Raylib-RmlUi platform backend / bridge.
// This file mirrors Raylib's Init/Close/IsReady convention.
//
// Usage:
//   InitWindow(...);
//   InitRmlUi(w, h);           // registers render+system interfaces, calls Rml::Initialise
//   Rml::Context* ctx = Rml::CreateContext(...);
//   // each frame:
//   PumpRmlUiInput(*ctx, mouse_over_ui);
//   ctx->Update();
//   ctx->Render();
//   // teardown:
//   CloseRmlUi();              // calls Rml::Shutdown and destroys backend objects

#include <RmlUi/Core/Input.h>

namespace Rml { class Context; }

// ── Init / Destroy ─────────────────────────────────────────────────────────

bool InitRmlUi(int width, int height);
void CloseRmlUi(void);
bool IsRmlUiReady(void);

// ── Input ───────────────────────────────────────────────────────────

struct RmlInputResult {
    bool moved    = false;  // a mouse-move was processed
    bool discrete = false;  // button/wheel/key/text event or UI enter/leave
};

// Pumps all pending raylib input into context. Pass mouse_over_ui=false to suppress
// hover when the cursor is over non-UI content (e.g. a game world).
RmlInputResult PumpRmlUiInput(Rml::Context& ctx, bool mouse_over_ui = true);

// Current raylib modifier state packed as Rml::Input::KeyModifier bitmask.
int GetRmlUiKeyModifiers();

// Translate a raylib KEY_* code to an Rml::Input::KeyIdentifier.
Rml::Input::KeyIdentifier TranslateRaylibKey(int raylib_key);
