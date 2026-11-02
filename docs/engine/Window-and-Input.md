# Window and input

`Window` (`engine/include/engine/Window.h`) wraps a `GLFWwindow` and is only constructible through `App` (`friend class App`).

## Queries

```cpp
Window& window = app.get_window();

window.get_height();       // framebuffer height
window.get_aspect_ratio(); // width / height

window.is_key_pressed(GLFW_KEY_W);                 // polled, GLFW_PRESS
window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
window.get_delta_mouse();                          // mouse movement since last App::update()

window.set_cursor_mode(GLFW_CURSOR_DISABLED);
window.get_cursor_mode();

window.clear(RGBA_BLACK, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
window.swap_buffers();
```

Input is polled, not event-driven. `App::update()` resets `get_delta_mouse()` to zero each frame, so mouse deltas are per-frame values.

## Callbacks

`App::register_callbacks()` installs exactly two GLFW callbacks in `engine/src/callbacks.cpp`:

- framebuffer size: updates `Window` dimensions, the GL viewport, and the camera projection aspect ratio;
- cursor position: accumulates mouse delta and handles the initial "first mouse" jump.

Keyboard and mouse buttons are read through polling only. The engine does not map keys to actions; a game decides what a key means.

## Cursor policy

The engine never changes the cursor mode. A first-person game typically does:

```cpp
app.get_window().set_cursor_mode(GLFW_CURSOR_DISABLED);
```

and releases it for menus. `Camera::update_lazy()` is a no-op while the cursor is not disabled.
