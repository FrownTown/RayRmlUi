#pragma once

#include <RmlUi/Core/Input.h>

namespace Rml { class Context; }

namespace rayrmlui {

struct RmlInputResult {
    bool moved    = false;
    bool discrete = false;
};

RmlInputResult pumpRmlInput(Rml::Context& ctx, bool mouse_over_ui);
int currentKeyModifiers();
Rml::Input::KeyIdentifier translateRaylibKey(int raylib_key);

}
