#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <locale.h>
#include "state.h"

void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                        uint32_t format, int32_t fd, uint32_t size) {
    client_state* state = data;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        fprintf(stderr, "unknown keyboard format");
		state->running = false;
		return;
    }

    char* buffer = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap keymap");
		state->running = false;
        return;
    }

    state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!state->xkb_context) {
        fprintf(stderr, "Failed to create xkb context\n");
		state->running = false;
        return;
    }

    state->xkb_keymap = xkb_keymap_new_from_buffer(state->xkb_context, buffer, size - 1,
                                                   XKB_KEYMAP_FORMAT_TEXT_V1,
                                                   XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(buffer, size);
    close(fd);

    state->xkb_state = xkb_state_new(state->xkb_keymap);
    if (!state->xkb_state) {
        fprintf(stderr, "Failed to create state");
    }

    struct xkb_compose_table* xkb_compose_table = xkb_compose_table_new_from_locale(
        state->xkb_context, setlocale(LC_CTYPE, NULL), XKB_COMPOSE_COMPILE_NO_FLAGS);

    if (!xkb_compose_table) {
        fprintf(stderr, "Failed to create xkb compose table");
        return;
    }

    state->xkb_compose_state = xkb_compose_state_new(xkb_compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
}

void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface,
                       struct wl_array *keys) {
}

void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface) {
}

void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                     uint32_t serial, uint32_t time, uint32_t key,
                     uint32_t key_state) {
  client_state *state = data;

  xkb_keysym_t keysym = xkb_state_key_get_one_sym(state->xkb_state, key + 8);
  if (keysym == XKB_KEY_q) {
    state->running = false;
  }
}

void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, uint32_t mods_depressed,
                           uint32_t mods_latched, uint32_t mods_locked,
                           uint32_t group) {
    client_state* state = data;
    xkb_state_update_mask(state->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                             int32_t rate, int32_t delay) {
    client_state* state = data;
    state->key_repeat_rate = rate;
    state->key_repeat_delay = delay;
}

struct wl_keyboard_listener keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};
