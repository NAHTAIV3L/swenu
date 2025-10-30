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
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
	float x, y, dx, dy;
} rect_t;

typedef struct {
	GLuint vbo, ebo, vao;
	uint32_t num_elements;
	size_t pixel_len;
} text_buffer_t;

typedef struct {
	float advance_x, advance_y;
	uint32_t bitmap_width, bitmap_height;
	int32_t bearing_x, bearing_y;
	float texture_x_start;
	float texture_x_end;
} metric_t;

typedef struct {
	uint32_t width, height;
	GLuint texture;
	int32_t vert_shift;
	metric_t metrics[128];
} atlas_t;

typedef struct {
	char* text;
	size_t pixel_len;
	text_buffer_t text_buffer;
} item_t;

typedef struct {
	item_t* item;
	rect_t r;
} item_display_t;

typedef struct {
	// wayland
	struct wl_display* display;
	struct wl_registry* registry;
	struct wl_compositor* compositor;
	struct wl_surface* surface;
	struct zwlr_layer_shell_v1* layer_shell;
	struct zwlr_layer_surface_v1* layer_surface;
	struct wl_seat* seat;
	struct wl_data_device_manager *data_device_manager;
	struct wl_data_device *data_device;
	struct wl_data_offer *data_offer;
	struct wl_keyboard* keyboard;
	struct wl_pointer* pointer;
	struct wp_cursor_shape_manager_v1* cursor_shape_manager;
	struct wp_cursor_shape_device_v1* cursor_shape_device;
	char* clipboard;
	size_t clipboard_size;

	// xkb
	struct xkb_context* xkb_context;
	struct xkb_keymap* xkb_keymap;
	struct xkb_state* xkb_state;
	int key_repeat_rate, key_repeat_delay;
	int key_repeat_timer_fd;
	xkb_keysym_t repeat_key;

	// egl
	struct wl_egl_window* egl_window;
	EGLDisplay egl_display;
	EGLSurface egl_surface;
	EGLConfig egl_config;
	EGLContext egl_context;

	// graphics
	GLuint text_shader;
	GLuint t_screen_size_uniform;
	GLuint t_offset_uniform;
	GLuint t_color_uniform;
	GLuint box_shader;
	GLuint b_screen_size_uniform;
	GLuint b_start_uniform;
	GLuint b_size_uniform;
	GLuint b_color_uniform;
	text_buffer_t input_buffer_grafix;
	text_buffer_t prompt_text_buffer;

	// fonts
	FT_Library ft_library;
	FT_Face ft_face;
	FT_Int32 load_flags;
	atlas_t atlas;
	uint32_t line_height;

	uint32_t required_width;
	float horizontal_spacing;

	// arguments
	int lines;
	char* prompt;
	bool exact_match;
	bool center;
	bool verbose;
	bool password;

	// input
	item_t* items;

	// state
	bool running;
	int exit_code;
	uint32_t width, height;
	item_display_t* filtered_items;
	int selected_filtered_item;
	char* input_buffer;
	size_t cursor_index;

	// scrolling
	float scroll_offset;
	size_t displayed_item_start;
	size_t displayed_item_end;
	// page scroll
	size_t* page_indices; // page indices into filtered items
	size_t current_page;

} client_state;

#endif // STATE_H_
