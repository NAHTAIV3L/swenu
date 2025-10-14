#include "graphics.h"
#include "array.h"

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

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, state->atlas.texture);

	// set up matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double)state->width, (double)state->height, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw text
	glBegin(GL_QUADS);
	float pen_x = 300;
	float pen_y = 200;
	// TODO for unicode - check for missing characters, iterate over "unicode characters"
	for ( int n = 0; n < array_size(state->input_buffer); n++ )
	{
		char c = state->input_buffer[n];
		metric_t* m = &state->atlas.metrics[(int)c];

		float start_x = pen_x + m->bearing_x;
		float start_y = pen_y - m->bearing_y;
		glTexCoord2f(m->texture_x_start, 0.0f); glVertex2f(start_x, start_y);
		glTexCoord2f(m->texture_x_start, 1.0f); glVertex2f(start_x, start_y + m->bitmap_height);
		glTexCoord2f(m->texture_x_end, 1.0f); glVertex2f(start_x + m->bitmap_width, start_y + m->bitmap_height);
		glTexCoord2f(m->texture_x_end, 0.0f); glVertex2f(start_x + m->bitmap_width, start_y);
		
		pen_x += m->advance_x;
	}
	glEnd();

    glDisable(GL_TEXTURE_2D);

	eglSwapBuffers(state->egl_display, state->egl_surface);
}
