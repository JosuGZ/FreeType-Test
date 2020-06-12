#include <iostream>
#include <cassert>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using std::cout;
using std::endl;

static const char *fontFile = "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf";

static void testPng();

int main() {
  FT_Library freeType;
  FT_Error error;

  error = FT_Init_FreeType(&freeType);

  if (error) {
    cout << "FreeType ERROR" << endl;
    abort();
  }

  cout << "FreeType initialized!\n";

  // Loading a Font Face
  FT_Face fontFace;
  error = FT_New_Face(freeType, fontFile, 0, &fontFace);

  if (error == FT_Err_Unknown_File_Format) {
    cout << "The font file could be opened and read, but it appears that its font format is unsupported" << endl;
    abort();
  } else if (error) {
    cout << "Another error code means that the font file could not be opened or read, or that it is broken..." << endl;
    abort();
  }

  cout << "FontFace initialized!\n";

  cout << "family_name: " << fontFace->family_name << endl;
  cout << "style_name: " << fontFace->style_name << endl;
  cout << "num_glyphs: " << fontFace->num_glyphs << endl;
  cout << "num_charmaps: " << fontFace->num_charmaps << endl;
  cout << "num_faces: " << fontFace->num_faces << endl;
  cout << "num_fixed_sizes: " << fontFace->num_fixed_sizes << endl;
  cout << "available_sizes: " << fontFace->available_sizes << endl;

  testPng();

  return 0;
}

static const char *outputFile = "Test.png";

static void testPng() {
  uint8_t data[1000 * 1000 * 3] {};
  for (int i=0; i<1000; ++i) {
    for (int j=0; j<1000; ++j) {
      data[i * 3 * 1000 + j * 3 + 0] = i;
      data[i * 3 * 1000 + j * 3 + 1] = j;
      data[i * 3 * 1000 + j * 3 + 2] = 0xFF;
    }
  }
  int success = stbi_write_png(outputFile, 1000, 1000, 3, data, 1000 * 3);
  assert(success);
}
