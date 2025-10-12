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
