#include <EGL/egl.h>
#include <stdio.h>

#include "./state.h"
#include "wlr-layer-shell-unstable-v1.h"

extern struct wl_registry_listener registry_listener;
extern struct zwlr_layer_surface_v1_listener layer_surface_listener;

bool init_gl(client_state* state) {
    EGLint attrs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_CONFORMANT, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    state->egl_window = wl_egl_window_create(state->surface, state->width, state->height);
    if (!state->egl_window) {
        fprintf(stderr, "ewindow\n");
		return false;
    }


    state->egl_display = eglGetDisplay(state->display);
    if (state->egl_display == EGL_NO_DISPLAY)  {
        fprintf(stderr, "edisplay\n");
		return false;
    }
    {
        EGLint major, minor;
        if (eglInitialize(state->egl_display, &major, &minor) != EGL_TRUE) {
            fprintf(stderr, "eglinit\n");
            return false;
        }

        printf("EGL version %u.%u\n", major, minor);
    }
    eglBindAPI(EGL_OPENGL_API);

    EGLint num_configs;
    if (eglChooseConfig(state->egl_display, attrs, &state->egl_config, 1, &num_configs) != EGL_TRUE) {
        fprintf(stderr, "econfig: %d\n", eglGetError());
        return false;
    }

    state->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_config,
                                            (EGLNativeWindowType)state->egl_window, NULL);
    if (state->egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "esurface\n");
        return false;
    }

    state->egl_context = eglCreateContext(state->egl_display, state->egl_config,
                                          EGL_NO_CONTEXT, NULL);
    if (state->egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "econtext: %x\n", eglGetError());
        return false;
    }

    eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

    if (!gladLoadGL()) {
        fprintf(stderr, "gladLoadGL");
        return false;
    }

    {
        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        printf("GL version %u.%u\n", major, minor);
    }
	return true;
}

int main() {
	client_state state = {0};

	state.running = true;

	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to Connect to wayland display\n");
		return 0;
	}

	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	state.surface = wl_compositor_create_surface(state.compositor);
	state.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		state.layer_shell, state.surface, NULL,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP, "swenu");
	zwlr_layer_surface_v1_add_listener(state.layer_surface, &layer_surface_listener, &state);
	zwlr_layer_surface_v1_set_size(state.layer_surface, 600, 400);
	zwlr_layer_surface_v1_set_anchor(state.layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(state.layer_surface, -1);
	zwlr_layer_surface_v1_set_margin(state.layer_surface, 0, 0, 0, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(state.layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);
	wl_surface_commit(state.surface);
	wl_display_roundtrip(state.display);

	if (!init_gl(&state)) {
		fprintf(stderr, "Failed to Initalize EGL/OpenGL\n");
		return 1;
	}

	while (state.running) {
		wl_display_dispatch_pending(state.display);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(state.egl_display, state.egl_surface);
	}
}
