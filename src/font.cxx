#ifdef WIN32
#include <windows.h>
#define vsnprintf(b, n, f, l) vsprintf(b, f, l)
#endif

#include <GL/glu.h>
#include <string>
#include <cassert>
#include <clocale>
#include <cctype>
#include <cstdarg>
#include <stdio.h>

#include "font.h"

#include <iostream>

FT_Library FontGL::ftLibrary = 0;

FontGL::FontGL(const char *filename, int pixelSize, int textureSize)
  : textureSize(textureSize)
{
  m_maxHeight = 0;
  if(!ftLibrary)
    initLibrary();

  setColor(1, 0, 0);
  //Load the font file
  FT_Error error = FT_New_Face(ftLibrary, filename, 0, &face);
  if(error) {
    std::string mess("FontGL: Can't load font face for ");
    std::cerr << mess << filename << std::endl;
    std::cerr << "Set the GLFPS_FONT environement variable to a true type font on your system" << std::endl;
  }

  //Set char size
  error = FT_Set_Pixel_Sizes(face, pixelSize, pixelSize);
  if(error)
    std::cerr << "FontGL: Can't set char size." << std::endl;

  //  generateTexture();
  //setlocale(LC_ALL, "");
}

FontGL::~FontGL()
{
  FT_Done_Face(face);
}

void
FontGL::generateTexture()
{
  using namespace std;
  FT_Glyph glyph;
  int u = 0, v = 0, maxHeight = 0;

  GLXContext ctx = glXGetCurrentContext();

  GLuint texCurrent;
  glGenTextures(1, &texCurrent);
  glBindTexture(GL_TEXTURE_2D, texCurrent);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, textureSize, textureSize,
               0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  textures.addTextureObject(ctx, texCurrent);
  glBindTexture(GL_TEXTURE_2D, texCurrent);

  for(int c = 0; c < 255; c++) {
    if(!isprint(c))
      continue;
    FT_Error error = FT_Load_Glyph(face, FT_Get_Char_Index(face, c),
                                   FT_LOAD_DEFAULT);
    if(error)
      std::cerr << "FontGL: Can't load gyph.\n";

    error = FT_Get_Glyph(face->glyph, &glyph);
    if(error)
      std::cerr << "FontGL: Can't get gyph.\n";

    error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 0);
    if(error)
      std::cerr << "FontGL: Can't convert gyph to bitmap.\n";

    FT_Bitmap bmp = ((FT_BitmapGlyph)glyph)->bitmap;

    //Do we need to go to the next line
    if(u+bmp.width >= textureSize) {
      u = 0;
      v += maxHeight;
      maxHeight = 0;
    }
    //Do we need to go to the next texture
    if(v+bmp.rows >= textureSize) {
      u = v = maxHeight = 0;
      glGenTextures(1, &texCurrent);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, textureSize, textureSize,
                   0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      textures.addTextureObject(ctx, texCurrent);
      glBindTexture(GL_TEXTURE_2D, texCurrent);
    }
    maxHeight = max(maxHeight, bmp.rows);

    m_maxHeight = max(maxHeight, m_maxHeight);

    addLetter(c, glyph, bmp.width, bmp.rows, u, v, ctx, texCurrent);

    glTexSubImage2D(GL_TEXTURE_2D, 0, u, v, bmp.width, bmp.rows,
                    GL_ALPHA, GL_UNSIGNED_BYTE, bmp.buffer);
    u += bmp.width;

    FT_Done_Glyph(glyph);
  }
}

void
FontGL::addLetter(int c, FT_Glyph glyph, unsigned int width,
                  unsigned int height, int u, int v,
                  GLXContext ctx, GLuint texture)
{
  letter *tmp = &letters[c];
  FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &tmp->bbox);
  FT_BitmapGlyph bmp = (FT_BitmapGlyph)glyph;
  tmp->left = bmp->left;
  tmp->top = bmp->top;
  tmp->minU = (float)u/(float)textureSize;
  tmp->minV = (float)v/(float)textureSize;
  tmp->maxU = (float)(u+width)/(float)textureSize;
  tmp->maxV = (float)(v+height)/(float)textureSize;
  tmp->width = width; tmp->height = height;
  tmp->advance = face->glyph->advance;
  tmp->texture.addTextureObject(ctx, texture);
}

/*Save openGL states, set color, set matrix*/
void
FontGL::begin()
{
  currentCtx = glXGetCurrentContext();
  //Gen textures if needed
  if(textures.findFirstTextureObject(currentCtx) == 0)
    generateTexture();

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  GLint numClip;
  glGetIntegerv(GL_MAX_CLIP_PLANES, &numClip);
  for(int i = 0; i < numClip; i++)
    glDisable(GL_CLIP_PLANE0+i);

  glDepthMask(GL_FALSE);
  glStencilMask(GL_FALSE);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_POLYGON_STIPPLE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_TEXTURE_3D);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glColor3fv(color);
}

/*Restor openGL states and proj matrix*/
void
FontGL::end()
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void
FontGL::drawChar(float x, float y, int c)
{
  letter *tmp = &letters[c];
  GLuint activeTex = tmp->texture.findFirstTextureObject(currentCtx);
  if(!currentTexture || currentTexture != activeTex)
  {
    currentTexture = activeTex;
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_BLEND);
  }

  x += tmp->left;
  y -= tmp->height-tmp->top;

  glLoadIdentity();
  glBegin(GL_QUADS);
  glTexCoord2f(tmp->minU, tmp->maxV);
  glVertex2f(x, y);

  glTexCoord2f(tmp->minU, tmp->minV);
  glVertex2f(x, y+tmp->height);

  glTexCoord2f(tmp->maxU, tmp->minV);
  glVertex2f(x+tmp->width, y+tmp->height);

  glTexCoord2f(tmp->maxU, tmp->maxV);
  glVertex2f(x+tmp->width, y);
  glEnd();
}

void
FontGL::printf(float x, float y, const char *fmt, ...)
{
  char buffer[MAXSTRINGLEN];
  va_list liste;

  va_start(liste, fmt);
  vsnprintf(buffer, MAXSTRINGLEN, fmt, liste);
  va_end(liste);

  buffer[MAXSTRINGLEN-1] = '\0';


  begin();
  drawLine(x, y, buffer);
  end();

//   glColor4f(1., 0., 0., 1.f);

//   drawLine(x, y, buffer);
}
/*
void
FontGL::getBBox(float &xmin, float &ymin, float &xmax, float &ymax,
                const char *fmt, ...)
{
  char buffer[MAXSTRINGLEN];
  va_list liste;

  va_start(liste, fmt);
  vsnprintf(buffer, MAXSTRINGLEN, fmt, liste);
  va_end(liste);

  buffer[MAXSTRINGLEN-1] = '\0';
  getLineBBox(xmin, ymin, xmax, ymax, buffer);
}

void
FontGL::getLineBBox(float x, float y, const char *str)
{
  FT_UInt prevGlyph = 0;

  FT_Long useKerning = FT_HAS_KERNING(face);

  for(;*str != '\0';str++) {
    if(!isprint(*str))
      continue;
    FT_UInt curGlyph = FT_Get_Char_Index(face, *str);
    if(useKerning&&prevGlyph) {
      FT_Vector kern;
      FT_Get_Kerning(face, prevGlyph, curGlyph, FT_KERNING_UNFITTED, &kern);
      x += kern.x >> 6;
    }
    drawChar(x, y, *str);
    prevGlyph = curGlyph;
    x += letters[(unsigned int)*str].advance.x>>6;
  }
}*/

void
FontGL::drawLine(float x, float y, const char *str)
{
  FT_UInt prevGlyph = 0;

  FT_Long useKerning = FT_HAS_KERNING(face);
  currentTexture = 0;

  for(;*str != '\0';str++) {
    if(!isprint(*str))
      continue;
    FT_UInt curGlyph = FT_Get_Char_Index(face, *str);
    if(useKerning&&prevGlyph) {
      FT_Vector kern;
      FT_Get_Kerning(face, prevGlyph, curGlyph, FT_KERNING_UNFITTED, &kern);
      x += kern.x >> 6;
    }
    drawChar(x, y, *str);
    prevGlyph = curGlyph;
    x += letters[(unsigned int)*str].advance.x>>6;
  }
}

void
FontGL::setColor(float r, float g, float b)
{
  color[0] = r; color[1] = g; color[2] = b;
}

/*****************************************************************************/
/* Static functions for init/exit of freetype lib                            */
/*****************************************************************************/
void
FontGL::initLibrary()
{
  FT_Error error = FT_Init_FreeType(&ftLibrary);
  if (error)
    std::cerr << "Can't initializate Freetype\n";
}

void
FontGL::exit()
{
  if(ftLibrary)
    FT_Done_FreeType(ftLibrary);
}


/*****************************************************************************/
/*DEBUG                                                                      */
/*****************************************************************************/
void
FontGL::testRender()
{
  GLXContext ctx = glXGetCurrentContext();

  if(textures.findFirstTextureObject(ctx) == 0)
    generateTexture();

  begin();
  glBindTexture(GL_TEXTURE_2D, textures.findFirstTextureObject(ctx));
  glEnable(GL_BLEND);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(0,0);

  glTexCoord2f(0, 1);
  glVertex2f(0, (float)textureSize);

  glTexCoord2f(1, 1);
  glVertex2f((float)textureSize, (float)textureSize);

  glTexCoord2f(1, 0);
  glVertex2f((float)textureSize, 0);
  glEnd();
  end();
}
