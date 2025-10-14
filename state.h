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
#include <sys/timerfd.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./glad/glad.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
	float advance_x, advance_y;
	uint32_t bitmap_width, bitmap_height;
	float texture_offset;
	float texture_size;
} metric_t;

typedef struct {
	uint32_t width, height;
	GLuint texture;
	metric_t metrics[128];
} atlas_t;

typedef struct {
	// wayland
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct zwlr_layer_shell_v1* layer_shell;
    struct zwlr_layer_surface_v1* layer_surface;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
	struct wp_cursor_shape_manager_v1* cursor_shape_manager;
	struct wp_cursor_shape_device_v1* cursor_shape_device;

	// xkb
	struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;
    struct xkb_state* xkb_state;
	struct xkb_compose_state* xkb_compose_state;
	int key_repeat_rate, key_repeat_delay;
    int key_repeat_timer_fd;
	xkb_keysym_t repeat_key;

	// egl
    struct wl_egl_window* egl_window;
    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLConfig egl_config;
    EGLContext egl_context;

	// state
	char* input_buffer;
    uint32_t width, height;

	FT_Library ft_library;
	FT_Face ft_face;
	FT_Int32 load_flags;

	atlas_t atlas;

    bool running;

} client_state;

#endif // STATE_H_
