#include <fontconfig/fontconfig.h>
#include "font.h"

char* get_font(const char* font_name) {
	FcPattern* pattern = FcNameParse((const FcChar8*)font_name);
	if (!pattern) {
		fprintf(stderr, "Failed to create font pattern\n");
		return NULL;
	}

    FcConfigSubstitute(0, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

	FcResult result = 0;
	FcFontSet* font_patterns = FcFontSort(0, pattern, FcTrue, 0, &result);
	if (!font_patterns || !font_patterns->nfont) {
		fprintf(stderr, "No fonts installed on system\n");
		return NULL;
	}

	FcFontSet* fs = FcFontSetCreate();
	FcPattern* font_pattern  = FcFontRenderPrepare(0, pattern, font_patterns->fonts[0]);
	if (!font_pattern) {
		fprintf(stderr, "Could not prepare matched font for loading\n");
		return NULL;
	}
	FcFontSetAdd(fs, font_pattern);

	FcFontSetSortDestroy(font_patterns);
	FcPatternDestroy(pattern);

	if (!fs || !font_patterns->nfont)  {
		fprintf(stderr, "Could not prepare font set\n");
	}
	FcObjectSet* os = FcObjectSetBuild(FC_FILE, (char*)0);
	FcValue v = { 0 };

	FcPattern* fontp = FcPatternFilter(fs->fonts[0], os);
	FcPatternGet(fontp, FC_FILE, 0, &v);
	char* font = strdup((char*)v.u.f);

	FcPatternDestroy(fontp);
	FcFontSetDestroy(fs);
	FcObjectSetDestroy(os);

	return font;
}

bool init_freetype(client_state* state, const char* file) {
	if (FT_Init_FreeType(&state->ft_library)) {
		fprintf(stderr, "Failed to intalize FreeType Library\n");
		return false;
	}
	if (FT_New_Face(state->ft_library, file, 0, &state->ft_face)) {
		fprintf(stderr, "Failed to create font\n");
		return false;
	}
	if (FT_Set_Char_Size(state->ft_face, 0, 12 << 6, 0, 0)) {
		fprintf(stderr, "Failed to set font size\n");
		return false;
	}
	return true;
}

bool init_atlas(client_state* state) {
    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
	for (int i = 32; i < 128; i++) {
		if (FT_Load_Char(state->ft_face, i, load_flags)) {
			fprintf(stderr, "Failed to render character %c\n", (char)i);
		}
		printf("%c: %u, %u\n", (char)i, state->ft_face->glyph->bitmap.width, state->ft_face->glyph->bitmap.rows);
		state->atlas.width += state->ft_face->glyph->bitmap.width;
		state->atlas.height = MAX(state->atlas.height, state->ft_face->glyph->bitmap.rows);
	}

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &state->atlas.texture);
    glBindTexture(GL_TEXTURE_2D, state->atlas.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        (GLsizei) state->atlas.width,
        (GLsizei) state->atlas.height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        NULL);

	uint32_t x = 0;
	for (int i = 32; i < 128; i++) {
		if (FT_Load_Char(state->ft_face, i, load_flags)) {
			fprintf(stderr, "Failed to render character %c\n", (char)i);
		}

        if (FT_Render_Glyph(state->ft_face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);
        }

		state->atlas.metrics[i].advance_x = (float)state->ft_face->glyph->advance.x / 64;
		state->atlas.metrics[i].advance_y = (float)state->ft_face->glyph->advance.y / 64;
		state->atlas.metrics[i].bitmap_width = state->ft_face->glyph->bitmap.width;
		state->atlas.metrics[i].bitmap_height = state->ft_face->glyph->bitmap.rows;
		state->atlas.metrics[i].texture_offset = x;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            x,
            0,
            state->ft_face->glyph->bitmap.width,
            state->ft_face->glyph->bitmap.rows,
            GL_RED,
            GL_UNSIGNED_BYTE,
            state->ft_face->glyph->bitmap.buffer);
        x += state->ft_face->glyph->bitmap.width;
	}

	return true;
}
