#include "RmlSystemRaylib.h"

#include <raylib.h>

namespace rayrmlui {

double RmlSystemRaylib::GetElapsedTime() {
    return GetTime();
}

bool RmlSystemRaylib::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    int level = LOG_INFO;
    switch (type) {
        case Rml::Log::LT_ERROR:
        case Rml::Log::LT_ASSERT:   level = LOG_ERROR;   break;
        case Rml::Log::LT_WARNING:  level = LOG_WARNING; break;
        case Rml::Log::LT_INFO:     level = LOG_INFO;    break;
        case Rml::Log::LT_DEBUG:    level = LOG_DEBUG;   break;
        default:                    level = LOG_INFO;    break;
    }
    TraceLog(level, "[RmlUi] %s", message.c_str());
    return true;
}

void RmlSystemRaylib::SetMouseCursor(const Rml::String& cursor_name) {
    int c = MOUSE_CURSOR_DEFAULT;
    if (cursor_name == "pointer" || cursor_name == "hand")
        c = MOUSE_CURSOR_POINTING_HAND;
    else if (cursor_name == "text" || cursor_name == "ibeam")
        c = MOUSE_CURSOR_IBEAM;
    else if (cursor_name == "cross")
        c = MOUSE_CURSOR_CROSSHAIR;
    else if (cursor_name == "move")
        c = MOUSE_CURSOR_RESIZE_ALL;
    else if (cursor_name == "resize-ew" || cursor_name == "ew-resize")
        c = MOUSE_CURSOR_RESIZE_EW;
    else if (cursor_name == "resize-ns" || cursor_name == "ns-resize")
        c = MOUSE_CURSOR_RESIZE_NS;
    else if (cursor_name == "resize-nwse" || cursor_name == "nwse-resize")
        c = MOUSE_CURSOR_RESIZE_NWSE;
    else if (cursor_name == "resize-nesw" || cursor_name == "nesw-resize")
        c = MOUSE_CURSOR_RESIZE_NESW;
    else if (cursor_name == "unavailable" || cursor_name == "not-allowed")
        c = MOUSE_CURSOR_NOT_ALLOWED;
    ::SetMouseCursor(c);
}

void RmlSystemRaylib::SetClipboardText(const Rml::String& text) {
    ::SetClipboardText(text.c_str());
}

void RmlSystemRaylib::GetClipboardText(Rml::String& text) {
    const char* s = ::GetClipboardText();
    text = s ? s : "";
}

}
