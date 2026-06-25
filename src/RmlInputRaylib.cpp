#include "RmlInputRaylib.h"

#include <RmlUi/Core/Context.h>
#include <raylib.h>

namespace rayrmlui {

namespace {

constexpr int kRaylibMouseButtons[] = {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
};

struct MouseState {
    int x = -1, y = -1;
    bool over_ui = true;
};
MouseState g_last_mouse;

}

int currentKeyModifiers() {
    namespace K = Rml::Input;
    int mod = 0;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) mod |= K::KM_CTRL;
    if (IsKeyDown(KEY_LEFT_SHIFT)   || IsKeyDown(KEY_RIGHT_SHIFT))   mod |= K::KM_SHIFT;
    if (IsKeyDown(KEY_LEFT_ALT)     || IsKeyDown(KEY_RIGHT_ALT))     mod |= K::KM_ALT;
    if (IsKeyDown(KEY_LEFT_SUPER)   || IsKeyDown(KEY_RIGHT_SUPER))   mod |= K::KM_META;
    return mod;
}

Rml::Input::KeyIdentifier translateRaylibKey(int k) {
    namespace K = Rml::Input;
    if (k >= KEY_A && k <= KEY_Z)
        return static_cast<K::KeyIdentifier>(K::KI_A + (k - KEY_A));
    if (k >= KEY_ZERO && k <= KEY_NINE)
        return static_cast<K::KeyIdentifier>(K::KI_0 + (k - KEY_ZERO));
    if (k >= KEY_F1 && k <= KEY_F12)
        return static_cast<K::KeyIdentifier>(K::KI_F1 + (k - KEY_F1));
    if (k >= KEY_KP_0 && k <= KEY_KP_9)
        return static_cast<K::KeyIdentifier>(K::KI_NUMPAD0 + (k - KEY_KP_0));

    switch (k) {
        case KEY_SPACE:        return K::KI_SPACE;
        case KEY_ESCAPE:       return K::KI_ESCAPE;
        case KEY_ENTER:        return K::KI_RETURN;
        case KEY_TAB:          return K::KI_TAB;
        case KEY_BACKSPACE:    return K::KI_BACK;
        case KEY_INSERT:       return K::KI_INSERT;
        case KEY_DELETE:       return K::KI_DELETE;
        case KEY_LEFT:         return K::KI_LEFT;
        case KEY_RIGHT:        return K::KI_RIGHT;
        case KEY_UP:           return K::KI_UP;
        case KEY_DOWN:         return K::KI_DOWN;
        case KEY_HOME:         return K::KI_HOME;
        case KEY_END:          return K::KI_END;
        case KEY_PAGE_UP:      return K::KI_PRIOR;
        case KEY_PAGE_DOWN:    return K::KI_NEXT;
        case KEY_PAUSE:        return K::KI_PAUSE;
        case KEY_CAPS_LOCK:    return K::KI_CAPITAL;
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:  return K::KI_LSHIFT;
        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL:return K::KI_LCONTROL;
        case KEY_LEFT_ALT:
        case KEY_RIGHT_ALT:    return K::KI_LMENU;
        case KEY_APOSTROPHE:   return K::KI_OEM_7;
        case KEY_COMMA:        return K::KI_OEM_COMMA;
        case KEY_MINUS:        return K::KI_OEM_MINUS;
        case KEY_PERIOD:       return K::KI_OEM_PERIOD;
        case KEY_SLASH:        return K::KI_OEM_2;
        case KEY_SEMICOLON:    return K::KI_OEM_1;
        case KEY_EQUAL:        return K::KI_OEM_PLUS;
        case KEY_LEFT_BRACKET: return K::KI_OEM_4;
        case KEY_BACKSLASH:    return K::KI_OEM_5;
        case KEY_RIGHT_BRACKET:return K::KI_OEM_6;
        case KEY_GRAVE:        return K::KI_OEM_3;
        case KEY_KP_DECIMAL:   return K::KI_DECIMAL;
        case KEY_KP_DIVIDE:    return K::KI_DIVIDE;
        case KEY_KP_MULTIPLY:  return K::KI_MULTIPLY;
        case KEY_KP_SUBTRACT:  return K::KI_SUBTRACT;
        case KEY_KP_ADD:       return K::KI_ADD;
        case KEY_KP_ENTER:     return K::KI_NUMPADENTER;
        default:               return K::KI_UNKNOWN;
    }
}

RmlInputResult pumpRmlInput(Rml::Context& ctx, bool mouse_over_ui) {
    const int mods = currentKeyModifiers();
    RmlInputResult result;

    const int mx = GetMouseX();
    const int my = GetMouseY();
    if (mouse_over_ui) {
        if (mx != g_last_mouse.x || my != g_last_mouse.y || !g_last_mouse.over_ui) {
            ctx.ProcessMouseMove(mx, my, mods);
            g_last_mouse.x = mx;
            g_last_mouse.y = my;
            result.moved = true;
        }
    } else if (g_last_mouse.over_ui) {
        ctx.ProcessMouseMove(-100000, -100000, mods);
        result.discrete = true;
    }
    g_last_mouse.over_ui = mouse_over_ui;

    for (int i = 0; i < 3; ++i) {
        if (IsMouseButtonPressed(kRaylibMouseButtons[i]))  { ctx.ProcessMouseButtonDown(i, mods); result.discrete = true; }
        if (IsMouseButtonReleased(kRaylibMouseButtons[i])) { ctx.ProcessMouseButtonUp(i, mods);   result.discrete = true; }
    }

    const Vector2 wheel = GetMouseWheelMoveV();
    if (wheel.x != 0.0f || wheel.y != 0.0f) {
        ctx.ProcessMouseWheel(Rml::Vector2f(-wheel.x, -wheel.y), mods);
        result.discrete = true;
    }

    while (int k = GetKeyPressed()) {
        if (k == KEY_SPACE) continue;
        const auto id = translateRaylibKey(k);
        if (id != Rml::Input::KI_UNKNOWN) { ctx.ProcessKeyDown(id, mods); result.discrete = true; }
    }

    static constexpr int kPolledKeys[] = {
        KEY_SPACE, KEY_ESCAPE, KEY_ENTER, KEY_TAB, KEY_BACKSPACE,
        KEY_INSERT, KEY_DELETE, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
        KEY_HOME, KEY_END, KEY_PAGE_UP, KEY_PAGE_DOWN,
        KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL,
        KEY_LEFT_ALT, KEY_RIGHT_ALT,
    };
    for (int k : kPolledKeys) {
        if (IsKeyReleased(k)) {
            const auto id = translateRaylibKey(k);
            if (id != Rml::Input::KI_UNKNOWN) { ctx.ProcessKeyUp(id, mods); result.discrete = true; }
        }
    }

    while (int c = GetCharPressed()) {
        ctx.ProcessTextInput(static_cast<Rml::Character>(c));
        result.discrete = true;
    }

    return result;
}

}
