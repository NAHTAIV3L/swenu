#ifndef STATE_H_
#define STATE_H_
#include <wayland-client.h>

typedef struct {
	struct wl_display* display;
	struct wl_surface* surface;
} client_state;

#endif // STATE_H_
