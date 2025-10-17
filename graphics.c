#include "graphics.h"
#include "array.h"
#include "shader.h"

void GLAPIENTRY
MessageCallback(GLenum source,
				GLenum type,
				GLuint id,
				GLenum severity,
				GLsizei length,
				const GLchar* message,
				const void* userParam) {
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		 ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
		 type, severity, message );
}

const char* vertexShader = "#version 330 core\n\
layout (location = 0) in vec2 pos;\n\
layout (location = 1) in vec2 uv;\n\
\n\
uniform vec2 u_screen_size;\n\
uniform vec2 u_offset;\n\
\n\
out vec2 o_uv;\n\
\n\
vec2 project(vec2 point) { \n\
    return (((2.0 * point) / u_screen_size) - vec2(1)); \n\
} \n\
\n\
void main() {\n\
	o_uv = uv;\n\
	gl_Position = vec4(project(pos + u_offset), 0.0, 1.0);\n\
}";

const char* fragmentShader = "#version 330 core\n\
uniform sampler2D u_texture;\n\
in vec2 o_uv;\n\
\n\
layout (location = 0) out vec4 frag_color;\n\
\n\
void main() {\n\
	float d = texture(u_texture, o_uv).r;\n\
	float aaf = fwidth(d);\n\
	float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);\n\
	frag_color = vec4(1.0, 1.0, 1.0, alpha);\n\
}";

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_SCISSOR_TEST);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	state->text_shader = createShader(vertexShader, fragmentShader);
	state->screen_size_uniform = glGetUniformLocation(state->text_shader, "u_screen_size");
	state->offset_uniform = glGetUniformLocation(state->text_shader, "u_offset");

	return true;
}

void render_frame(client_state *state) {
	// set up frame
	glViewport(0, 0, state->width, state->height);
	glScissor(0, 0, state->width, state->height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind shader and texture
	glUseProgram(state->text_shader);
	glUniform2f(state->screen_size_uniform, state->width, state->height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->atlas.texture);

	float horizontal_spacing = state->line_height / 2.0f;
	float pen_x = horizontal_spacing / 2.0f;
	float pen_y = state->line_height * state->lines - state->atlas.vert_shift;

	// draw input buffer
	if (state->lines > 0) {
		glScissor(pen_x, pen_y + state->atlas.vert_shift, state->width, state->line_height);
	}
	else {
		glScissor(pen_x, pen_y + state->atlas.vert_shift, state->width / 3.0f - pen_x, state->line_height);
	}
	glBindVertexArray(state->input_buffer_grafix.vao);
	glUniform2f(state->offset_uniform, pen_x, pen_y);
	glDrawElements(GL_TRIANGLES, state->input_buffer_grafix.num_elements, GL_UNSIGNED_INT, 0);

	// set up drawing for options
	if (state->lines > 0) {
		glScissor(0, 0, state->width, pen_y);
		pen_y -= state->line_height;
	}
	else {
		pen_x += state->width / 3.0f;
		glScissor(pen_x, 0, state->width, state->line_height);
		pen_x += horizontal_spacing / 2.0f;
	}

	// draw options
	array_for_all(item_t, item, state->items) {
		glBindVertexArray(item->text_buffer.vao);
		glUniform2f(state->offset_uniform, pen_x, pen_y);
		glDrawElements(GL_TRIANGLES, item->text_buffer.num_elements, GL_UNSIGNED_INT, 0);

		// shift pen
		if (state->lines > 0) {
			pen_y -= state->line_height;
		}
		else {
			pen_x += item->pixel_len + horizontal_spacing;
		}
	}

	// present screen
	eglSwapBuffers(state->egl_display, state->egl_surface);
}

typedef struct {
	float x, y, u, v;
} vert_t;

void init_text_buffer(client_state* state, text_buffer_t* buffer, char* text, size_t text_len) {
	buffer->pixel_len = 0;

	// generate openg bullshit
	glGenVertexArrays(1, &buffer->vao);
	glBindVertexArray(buffer->vao);
	glGenBuffers(1, &buffer->vbo);
	glGenBuffers(1, &buffer->ebo);

	// TODO for unicode - check for missing characters, iterate over "unicode characters" not chars
	// TODO for all fonts - possibly kerning
	// generate vertices
	vert_t vertices[text_len * 4];
	uint32_t indices[text_len * 6];
	buffer->num_elements = text_len * 6;
	float pen_x = 0;
	for (int n = 0; n < text_len; n++) {
		char c = text[n];
		metric_t* m = &state->atlas.metrics[(int)c];

		float start_x = pen_x + m->bearing_x;
		float start_y = m->bearing_y;
		uint32_t base_vert = n * 4;
		uint32_t base_idx = n * 6;

		// bottom left vert
		vertices[base_vert + 0] = (vert_t){
			.x=start_x,
			.y=start_y,
			.u=m->texture_x_start,
			.v=0.0f,
		};
		// top left vert
		vertices[base_vert + 1] = (vert_t){
			.x=start_x,
			.y=start_y - m->bitmap_height,
			.u=m->texture_x_start,
			.v=(m->bitmap_height / (float)state->atlas.height),
		};
		// top right vert
		vertices[base_vert + 2] = (vert_t){
			.x=start_x + m->bitmap_width,
			.y=start_y - m->bitmap_height,
			.u=m->texture_x_end,
			.v=(m->bitmap_height / (float)state->atlas.height),
		};
		// bottom right vert
		vertices[base_vert + 3] = (vert_t){
			.x=start_x + m->bitmap_width,
			.y=start_y,
			.u=m->texture_x_end,
			.v=0,
		};

		// indices
		indices[base_idx + 0] = base_vert + 0;
		indices[base_idx + 1] = base_vert + 1;
		indices[base_idx + 2] = base_vert + 2;
		indices[base_idx + 3] = base_vert + 0;
		indices[base_idx + 4] = base_vert + 2;
		indices[base_idx + 5] = base_vert + 3;

		// go forward
		pen_x += m->advance_x;
		buffer->pixel_len += ceil(m->advance_x);
	}

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vert_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert_t), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void destroy_text_buffer(text_buffer_t* buffer) {
	glDeleteVertexArrays(1, &buffer->vao);
    glDeleteBuffers(1, &buffer->vbo);
    glDeleteBuffers(1, &buffer->ebo);
}
