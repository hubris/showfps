#include <stdio.h>

#include "timer.h"
#include "font.h"
#include "LibraryLoader.h"

#include <map>
#include <iostream>

#include <GL/glx.h>

typedef void(*PFNGLXSWAPBUFFERS)(Display*, GLXDrawable);
void (*oldGlXSwapBuffers)(Display*,GLXDrawable);

static std::map<GLXContext, Timer> g_timers;
static std::map<GLXContext, unsigned int> g_frames;
static std::map<GLXContext, float> g_fps;

static FontGL* g_font = NULL;

#define DEFAULTFONT "/usr/share/fonts/dejavu/DejaVuSansMono-Bold.ttf"

void
__attribute__ ((constructor)) sharedLibInit()
{
  LibraryLoader loader;
  loader.open("libGL.so");
  oldGlXSwapBuffers = (PFNGLXSWAPBUFFERS)loader.getSym("glXSwapBuffers");
}

void
__attribute__ ((destructor)) sharedLibExit()
{
  FontGL::exit();
  delete g_font;
}

void
glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
  GLXContext ctx =  glXGetCurrentContext();
  g_frames[ctx]++;

  if(!g_font) {
    std::string fontName = DEFAULTFONT;
    char* envName = getenv("GLFPS_FONT");
    if(envName)
      fontName = envName;

    g_font = new FontGL(fontName.c_str(), 30);
    g_font->setColor(1.f, 0.8f, 0.1f);
  }

  if(g_timers.find(ctx) == g_timers.end())
  {
    g_timers[ctx] = Timer();
    g_fps[ctx] = 0.f;
  }

  float sec = g_timers[ctx].getElapsedSeconds();
  if(sec >= 1.f)
  {
    float fps = g_frames[ctx]/sec;
    g_fps[ctx] = fps;
    g_frames[ctx] = 0;
    g_timers[ctx].start();
  }
  g_font->printf(0, 0, "%.0f", g_fps[ctx]);

  oldGlXSwapBuffers(dpy, drawable);
}
