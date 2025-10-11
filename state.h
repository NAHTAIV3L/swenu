#ifndef STATE_H_
#define STATE_H_
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wlr-layer-shell-unstable-v1.h>
#include <EGL/egl.h>
#include <stdbool.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <cursor-shape-v1.h>

#include "./glad/glad.h"

typedef struct {
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_surface* surface;

    struct zwlr_layer_shell_v1* layer_shell;
    struct zwlr_layer_surface_v1* layer_surface;

    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;

    struct wl_surface* cursor_surface;
    struct wl_cursor_image* cursor_image;
    struct wl_cursor_theme* cursor_theme;
    struct wl_buffer* cursor_buffer;
    struct wl_cursor* cursor;

    struct wl_buffer* screen_buffer;

	struct wp_cursor_shape_manager_v1* cursor_shape_manager;
	struct wp_cursor_shape_device_v1* cursor_shape_device;

    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;
    struct xkb_state* xkb_state;
	struct xkb_compose_state* xkb_compose_state;
	int key_repeat_rate, key_repeat_delay;

    struct wl_egl_window* egl_window;
    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLConfig egl_config;
    EGLContext egl_context;

    uint32_t width, height;

    bool running;

} client_state;

#endif // STATE_H_
