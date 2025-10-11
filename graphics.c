#include "graphics.h"

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

void render_frame(client_state *state) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(state->egl_display, state->egl_surface);
}
