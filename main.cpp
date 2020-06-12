#include <iostream>
#include <cassert>

#include <ft2build.h>
#include <freetype/ftglyph.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using std::cout;
using std::endl;

static const char *fontFile = "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf";

static void writePng(void *data);

class Cursor {

public:
  Cursor() : mX(0), mY(0){};
  Cursor(int x, int y) : mX(x), mY(y){};

  void moveRight(int pixels) {
    mX += pixels;
  }

  void moveDown(int pixels) {
    mY += pixels;
  }

  int x() { return mX; };
  void setX(int x) { mX = x; };
  int y() { return mY; };

private:
  int mX;
  int mY;
};

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

  error = FT_Set_Char_Size(
      fontFace, // handle to face object
      0,        // char_width in 1/64th of points
      32 * 64,  // char_height in 1/64th of points
      300,      // horizontal device resolution
      300       // vertical device resolution
  );
  assert(!error);

  uint8_t imageData[1000 * 1000 * 3] {};
  for (int i = 0; i < 1000; ++i) {
    for (int j = 0; j < 1000; ++j) {
      imageData[i * 3 * 1000 + j * 3 + 0] = i*0.25;
      imageData[i * 3 * 1000 + j * 3 + 1] = j*0.25;
      imageData[i * 3 * 1000 + j * 3 + 2] = 0xFF;
    }
  }

  const char *text = "Hello, World!\0gt...---[]{} 😆";

  Cursor cursor{140, 400};

  auto pixelModeToName = [](unsigned char pixelMode) {
    switch (pixelMode) {
      case FT_PIXEL_MODE_NONE: return "FT_PIXEL_MODE_NONE";
      case FT_PIXEL_MODE_MONO: return "FT_PIXEL_MODE_MONO";
      case FT_PIXEL_MODE_GRAY: return "FT_PIXEL_MODE_GRAY";
      case FT_PIXEL_MODE_GRAY2: return "FT_PIXEL_MODE_GRAY2";
      case FT_PIXEL_MODE_GRAY4: return "FT_PIXEL_MODE_GRAY4";
      case FT_PIXEL_MODE_LCD: return "FT_PIXEL_MODE_LCD";
      case FT_PIXEL_MODE_LCD_V: return "FT_PIXEL_MODE_LCD_V";
      case FT_PIXEL_MODE_BGRA: return "FT_PIXEL_MODE_BGRA";
      default: return "Unknown";
    }
  };

  for (const char *p = text; *p; ++p) {
    char c = *p;

    FT_UInt glyph_index = FT_Get_Char_Index(fontFace, c);

    // Load glyph image into the slot (erase previous one)
    error = FT_Load_Glyph(fontFace, glyph_index, FT_LOAD_DEFAULT);
    if (error) continue; // ignore errors

    FT_Glyph glyph;
    error = FT_Get_Glyph(fontFace->glyph, &glyph);
    if (error) continue; // ignore errors
    error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    if (error) continue; // ignore errors

    FT_GlyphSlot glyphSlot = fontFace->glyph;

    // Convert to an anti-aliased bitmap
    error = FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);
    if (error) continue; // ignore errors

    FT_Bitmap bitmap = ((FT_BitmapGlyph)glyph)->bitmap;
    cout << c << endl;
    cout << "Pixel mode: " << pixelModeToName(bitmap.pixel_mode) << endl;
    cout << "Num grays: " << bitmap.num_grays << endl;
    cout << "Width: " << bitmap.width << endl;
    cout << "Pitch: " << bitmap.pitch << endl;
    cout << "width: " << glyphSlot->metrics.width << endl;
    cout << "height: " << glyphSlot->metrics.height << endl;
    cout << "horiBearingX: " << glyphSlot->metrics.horiBearingX << endl;
    cout << "horiBearingY: " << glyphSlot->metrics.horiBearingY << endl;
    cout << "horiAdvance: " << glyphSlot->metrics.horiAdvance << endl;
    cout << "horiAdvance / 64: " << (float)glyphSlot->metrics.horiAdvance / 64.0f << endl;
    for (int i = 0; i < bitmap.rows; ++i) {
      for (int j = 0; j < bitmap.width; ++j) {
        // TODO: Risky!
        int y = 100 - glyphSlot->metrics.horiBearingY / 64 + cursor.y() + i;
        int x = cursor.x() + j;

        uint8_t color = bitmap.buffer[i * bitmap.width + j];
        float alpha = (float)color / 255.0f;
        float beta = 1 - alpha;
        color = 0xFF;
        int pixelIndex = y * 1000 * 3 + x * 3;
        imageData[pixelIndex + 0] = imageData[pixelIndex + 0] * beta + color * alpha;
        imageData[pixelIndex + 1] = imageData[pixelIndex + 1] * beta + color * alpha;
        imageData[pixelIndex + 2] = imageData[pixelIndex + 2] * beta + color * alpha;
      }
    }

    cursor.moveRight(glyphSlot->metrics.horiAdvance / 64);
  }

  writePng(imageData);

  return 0;
}

static const char *outputFile = "Test.png";

static void writePng(void *data) {
  int success = stbi_write_png(outputFile, 1000, 1000, 3, data, 1000 * 3);
  assert(success);
}
