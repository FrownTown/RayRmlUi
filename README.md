# rayRmlUi

rayRmlUi is a [Raylib](https://github.com/raysan5/raylib) module providing a backend for [RmlUi](https://github.com/mikke89/RmlUi) — the HTML/CSS-based UI library. Provides render, system, and input interfaces so RmlUi can run inside a Raylib application with minimal extra effort.

rayRmlUi was originally developed as part of a larger Raylib project and later extracted into this standalone reusable library.

The API follows Raylib's `Init/Close/IsReady` convention for ease of use.

## Features

- Render backend — draws RmlUi geometry via Raylib (OpenGL 4.3)
- System backend — clock, logging, and file-loading via Raylib
- Input backend — translates Raylib keyboard/mouse/scroll events into RmlUi input
- SVG texture support via [NanoSVG](https://github.com/memononen/nanosvg)
- Dependencies fetched automatically via CMake `FetchContent`

## Dependencies

| Library | Version |
|---------|---------|
| raylib  | 5.5     |
| RmlUi   | 6.2     |
| FreeType| 2.13.3  |
| NanoSVG | master  |

## Building

```sh
cmake -B build
cmake --build build
```

CMake 3.20+ and a C++20 compiler are required. All dependencies are fetched automatically.

## Performance Considerations

As RmlUi is based on the conventions of modern web-design, each RmlUi context is a tree containing nested trees beneath it. This can cause performance to choke if a large DOM is being walked through repeatedly. This can be easily mitigated by avoiding some common pitfalls of GUI design using RmlUi:

### Rendering

- RmlUi will happily re-render its entire context on every Render() call, while we would rather skip it if nothing changed. This is best accomplished using a framebuffer object and a dirty flag on each variable. You can skip the entire call if no variables are dirtied.
- rlDrawRenderBatchActive() should always be called before glDrawElements. Otherwise, Raylib's batched geometry will share GL state with the draw call and produce garbage.

### Layout / context

- If any parts of a context do not need to be aware of each other, they should be split into separate contexts. Many small contexts can be rendered every frame cheaply, but a single large context can cause severe performance issues because it doesn't benefit from GPU parallelization.
- Data-For loops are inherently poorly performing and should be avoided in large DOM trees. If you want to nest a Data-For loop producing text inside a larger DOM without the performance impact, you can make the Data-For a separate context, then use SuppressAutoLayout() on the text rendered by the Data-For loop and manually position it relative to the DOM you want appearing as its container.
- SuppressAutoLayout() should also be used on any text element that gets frequently updated and any high-frequency text updates should sit in fixed-size boxes. This allows the renderer to replace just that data row rather than walking the entire DOM to reflow the layout.
- Recommend gating layout recalculation and GPU rendering with separate dirty flags so you can recalculate layout without issuing a GPU render and vice versa.

### Input

- Always make sure to cache the last mouse position, and only call ProcessMouseMove when the position actually changes. Otherwise you will be feeding it every frame even when the mouse is still, which marks the hovered context as dirty unnecessarily and can cause the layout to rebuild itself.
- Suppress hover when the cursor isn't over an RmlUi element. You can instead send a sentinel position (-100000, -100000) to clear hover state rather than continuing to send real coordinates. This stops mouse movement over the viewport or unrelated UI elements from triggering every RmlUi context to re-render.
- If you choose to use a framebuffer as suggested, gate the FBO dirtying based on input result, not just hover state. A discrete split like this allows you to ensure that a pure mouse movement only needs a re-render if hover state actually changed (like hovering over an element with a hover animation) rather than doing it unconditionally just because the user passed the mouse over the element.

### Data model

- You can use a state struct to handle variable dirtying. A function such as `pushState()` can compare the current UIState against a `prev_state_` tracked by the struct and only call the data model's dirty notification for fields that actually changed. This is especially useful for reclaiming performance from UI elements that *can* change rapidly, but usually don't.

## Usage

```cpp
InitWindow(1280, 720, "My App");
InitRmlUi(1280, 720);

Rml::LoadFontFace("assets/font.ttf");
Rml::Context* ctx = Rml::CreateContext("main", Rml::Vector2i(1280, 720));
ctx->LoadDocument("assets/ui.rml")->Show();

while (!WindowShouldClose()) {
    PumpRmlUiInput(*ctx);
    ctx->Update();

    BeginDrawing();
    ClearBackground(RAYWHITE);
    ctx->Render();
    EndDrawing();
}

CloseRmlUi();
CloseWindow();
```

### API

| Function | Description |
| -------- | ----------- |
| `InitRmlUi(w, h)` | Register backends and call `Rml::Initialise` |
| `CloseRmlUi()` | Call `Rml::Shutdown` and destroy backends |
| `IsRmlUiReady()` | Returns `true` after a successful `InitRmlUi` |
| `PumpRmlUiInput(ctx, mouse_over_ui)` | Forward all pending Raylib input to an RmlUi context |
| `GetRmlUiKeyModifiers()` | Current modifier state as `Rml::Input::KeyModifier` bitmask |
| `TranslateRaylibKey(key)` | Convert a `KEY_*` constant to `Rml::Input::KeyIdentifier` |

Pass `mouse_over_ui = false` to `PumpRmlUiInput` to suppress hover events when the cursor is over non-UI content (e.g. a game world).

## License

MIT License

Copyright (c) 2026 J. Max Levin AKA FrownTown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
