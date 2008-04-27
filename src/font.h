#ifndef FONT_H
#define FONT_H

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <GL/gl.h>
#include <GL/glx.h>

#include "GLObjectList.h"

class FontGL {
public:
  FontGL(const char *filename, int pixelSize = 14, int textureSize = 512);
  ~FontGL();

  void setColor(float r, float g, float b);
  /*MODEL_VIEW & PROJECTION are saved*/
  void printf(float x, float y, const char *fmt, ...);

  //Need to be called at the end of the application.
  static void exit();

  void testRender();

private:
  FontGL() {}
  static const int MAXSTRINGLEN = 1024;

  static FT_Library ftLibrary;
  void initLibrary();

  void generateTexture();

  float color[3];
  FT_Face face;

  //This structure holds info about location and geometry of a letter
  struct letter {
    GLObjectList texture;
    float minU, minV, maxU, maxV; //pos of the letter in the texture
    unsigned int width, height;
    int left, top;
    FT_Vector advance;
    FT_BBox bbox;
  };
  letter letters[256];
  void addLetter(int c, FT_Glyph glyph, unsigned int width,
                 unsigned int height, int u, int v,
                 GLXContext ctx, GLuint texture);

  /*This textures store all characters*/
  int textureSize;
  int currentTexture; //Helps to avoid state changes
  GLXContext currentCtx;
//  std::vector<GLuint> textures;
  GLObjectList textures;

  void begin();
  void end();

  void drawChar(float x, float y, int c);
  void drawLine(float x, float y, const char *str);

  int m_maxHeight;
};

#endif
