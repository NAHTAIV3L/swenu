#include "graphics.h"
#include "array.h"
#include "config.h"
#include "shader.h"
#include "font.h"

void GLAPIENTRY
MessageCallback(GLenum source,
				GLenum type,
				GLuint id,
				GLenum severity,
				GLsizei length,
				const GLchar* message,
				const void* userParam) {
	if (type != GL_DEBUG_TYPE_ERROR) return;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		 ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
		 type, severity, message );
}

const char* text_vertex_shader =
	"#version 330 core\n"
	"layout (location = 0) in vec2 pos;"
	"layout (location = 1) in vec2 uv;"
	""
	"uniform vec2 u_screen_size;"
	"uniform vec2 u_offset;"
	""
	"out vec2 o_uv;"
	""
	"vec2 project(vec2 point) { "
	"	return (((2.0 * point) / u_screen_size) - vec2(1)); "
	"}"
	""
	"void main() {"
	"	o_uv = uv;"
	"	gl_Position = vec4(project(pos + u_offset), 0.0, 1.0);"
	"}";

const char* text_fragment_shader =
	"#version 330 core\n"
	"uniform sampler2D u_texture;"
	"uniform vec4 u_color;"
	"in vec2 o_uv;"
	""
	"layout (location = 0) out vec4 frag_color;"
	""
	"void main() {"
	"	frag_color = u_color * vec4(1.0, 1.0, 1.0, texture(u_texture, o_uv).r);"
	"}";

const char* box_vertex_shader =
	"#version 330 core\n"
	"uniform vec2 u_screen_size;"
	"uniform vec2 u_start;"
	"uniform vec2 u_size;"
	""
	"vec2 project(vec2 point) { "
	"	return (((2.0 * point) / u_screen_size) - vec2(1)); "
	"}"
	""
	"void main() {"
	"   uint b = uint(1 << (gl_VertexID % 6));"
	"   vec2 offset = vec2((uint(0x1C) & b) != uint(0), (uint(0x0E) & b) != uint(0));"
	"	gl_Position = vec4(project(u_start + offset * u_size), 0.0, 1.0);"
	"}";

const char* box_fragment_shader =
	"#version 330 core\n"
	"uniform vec4 u_color;"
	""
	"layout (location = 0) out vec4 frag_color;"
	""
	"void main() {"
	"	frag_color = u_color;"
	"}";

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
		fprintf(stderr, "Failed to create EGL Window\n");
		return false;
	}

	state->egl_display = eglGetDisplay(state->display);
	if (state->egl_display == EGL_NO_DISPLAY)  {
		fprintf(stderr, "Failed to create EGL Display\n");
		return false;
	}
	{
		EGLint major, minor;
		if (eglInitialize(state->egl_display, &major, &minor) != EGL_TRUE) {
			fprintf(stderr, "Failed to initalize EGL\n");
			return false;
		}

		if (state->verbose) {
			fprintf(stderr, "EGL version %u.%u\n", major, minor);
		}
	}
	eglBindAPI(EGL_OPENGL_API);

	EGLint num_configs;
	if (eglChooseConfig(state->egl_display, attrs, &state->egl_config, 1, &num_configs) != EGL_TRUE) {
		fprintf(stderr, "Failed to choose EGL Config: %d\n", eglGetError());
		return false;
	}

	state->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_config,
											 (EGLNativeWindowType)state->egl_window, NULL);
	if (state->egl_surface == EGL_NO_SURFACE) {
		fprintf(stderr, "Failed to create EGL Window Surface\n");
		return false;
	}

	state->egl_context = eglCreateContext(state->egl_display, state->egl_config,
									   EGL_NO_CONTEXT, NULL);
	if (state->egl_context == EGL_NO_CONTEXT) {
		fprintf(stderr, "Failed to create EGL Context: %x\n", eglGetError());
		return false;
	}

	eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

	if (!gladLoadGL()) {
		fprintf(stderr, "Failed to load OpenGL functions\n");
		return false;
	}

	if (state->verbose) {
		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		fprintf(stderr, "GL version %u.%u\n", major, minor);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_SCISSOR_TEST);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	// text shader
	state->text_shader = createShader(text_vertex_shader, text_fragment_shader);
	state->t_screen_size_uniform = glGetUniformLocation(state->text_shader, "u_screen_size");
	state->t_offset_uniform = glGetUniformLocation(state->text_shader, "u_offset");
	state->t_color_uniform = glGetUniformLocation(state->text_shader, "u_color");

	// box shader
	state->box_shader = createShader(box_vertex_shader, box_fragment_shader);
	state->b_screen_size_uniform = glGetUniformLocation(state->box_shader, "u_screen_size");
	state->b_start_uniform = glGetUniformLocation(state->box_shader, "u_start");
	state->b_size_uniform = glGetUniformLocation(state->box_shader, "u_size");
	state->b_color_uniform = glGetUniformLocation(state->box_shader, "u_color");

	return true;
}

void render_frame(client_state *state) {
	// set up frame
	glViewport(0, 0, state->width, state->height);
	glScissor(0, 0, state->width, state->height);
	glClearColor(background_color.r,background_color.g,background_color.b,background_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind shader and texture
	glUseProgram(state->text_shader);
	glUniform2f(state->t_screen_size_uniform, state->width, state->height);
	glUniform4f(state->t_color_uniform, text_color.r,text_color.g,text_color.b,text_color.a);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->atlas.texture);

	// calculate input buffer
	float input_y = state->line_height * state->lines;
	float input_width;
	if (state->lines > 0) { input_width = state->width; }
	else { input_width = state->width / 3.0f; }

	// draw input buffer
	if (array_size(state->filtered_items) != 0) {
		glScissor(0, input_y, input_width - state->horizontal_spacing / 2.0f, state->line_height);
	}
	else {
		glScissor(0, 0, state->width, state->height);
	}
	glBindVertexArray(state->input_buffer_grafix.vao);
	glUniform2f(state->t_offset_uniform, state->horizontal_spacing / 2.0f, input_y - state->atlas.vert_shift);
	glDrawElements(GL_TRIANGLES, state->input_buffer_grafix.num_elements, GL_UNSIGNED_INT, 0);

	// draw cursor box
	glUseProgram(state->box_shader);
	glUniform4f(state->b_color_uniform, cursor_color.r,cursor_color.g,cursor_color.b,cursor_color.a);
	glUniform2f(state->b_screen_size_uniform, state->width, state->height);
	glUniform2f(state->b_start_uniform, atlas_get_strwidth_len(state, state->input_buffer, state->cursor_index) + state->horizontal_spacing / 2.0f, input_y);
	glUniform2f(state->b_size_uniform, state->cursor_index == array_size(state->input_buffer) ? state->atlas.metrics['M'].bitmap_width : state->atlas.metrics[(int)state->input_buffer[state->cursor_index]].bitmap_width, state->line_height);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	float start;
	// set up starting pen and scissor for options
	if (state->lines > 0) {
		glScissor(0, 0, state->width, input_y);
		start = input_y - state->line_height + state->scroll;

		if (array_size(state->filtered_items) != 0) {
			// calc scroll
			float selected_bot = start - state->filtered_items[state->selected_filtered_item].offset;
			float selected_top = selected_bot + state->line_height;
			if (selected_bot < 0) {
				state->scroll -= selected_bot;
				start -= selected_bot;
			}
			float diff = selected_top - input_y;
			if (diff > 0) {
				state->scroll -= diff;
				start -= diff;
			}
		}
	} else {
		glScissor(input_width, 0, state->width, state->line_height);
		start = input_width + state->scroll;

		if (array_size(state->filtered_items) != 0) {
			// calc scroll
			item_display_t* selected = &state->filtered_items[state->selected_filtered_item];
			float selected_len = selected->item->pixel_len + state->horizontal_spacing;
			float selected_left = start + selected->offset;
			float selected_right = selected_left + selected_len;
			float right_diff = selected_right - state->width;
			float left_diff = selected_left - input_width;
			if (selected_len > state->width - input_width) {
				state->scroll -= left_diff;
				start -= left_diff;
			}
			else if (right_diff > 0) {
				state->scroll -= right_diff;
				start -= right_diff;
			}
			else if (left_diff < 0) {
				state->scroll -= left_diff;
				start -= left_diff;
			}
		}
	}

	// draw highlighted option box
	if (state->selected_filtered_item != -1) {
		item_display_t* display = &state->filtered_items[state->selected_filtered_item];
		item_t* item = display->item;

		float x,y;
		if (state->lines > 0) {
			x = 0;
			y = start - display->offset;
		}
		else {
			x = start + display->offset;
			y = 0;
		}

		glUseProgram(state->box_shader);
		glUniform4f(state->b_color_uniform, highlight_color.r,highlight_color.g,highlight_color.b,highlight_color.a);
		glUniform2f(state->b_screen_size_uniform, state->width, state->height);
		glUniform2f(state->b_start_uniform, x, y);
		glUniform2f(state->b_size_uniform, (state->lines > 0) ? state->width : item->pixel_len + state->horizontal_spacing, state->line_height);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// draw options
	glUseProgram(state->text_shader);
	glUniform2f(state->t_screen_size_uniform, state->width, state->height);
	glUniform4f(state->t_color_uniform, text_color.r,text_color.g,text_color.b,text_color.a);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->atlas.texture);
	for (uint32_t i = 0; i < array_size(state->filtered_items); ++i) {
		item_display_t* display = &state->filtered_items[i];
		item_t* item = display->item;

		float x,y;
		if (state->lines > 0) {
			x = 0;
			y = start - display->offset;
		}
		else {
			x = start + display->offset;
			y = 0;
		}

		glBindVertexArray(item->text_buffer.vao);
		glUniform2f(state->t_offset_uniform, x + state->horizontal_spacing / 2.0f, y - state->atlas.vert_shift);
		glDrawElements(GL_TRIANGLES, item->text_buffer.num_elements, GL_UNSIGNED_INT, 0);
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
		if (n == 0) buffer->pixel_len += ceil(m->advance_x); // add a second if we are the first to equal padding on the right
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
