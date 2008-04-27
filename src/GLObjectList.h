#ifndef GLOBJECTLIST_H
#define GLOBJECTLIST_H

#include <GL/gl.h>
#include <vector>

#include <GL/glx.h>

/**
 * Associate an openGL object (texture, display list...)
 * to a context.
 */
class GLObjectList {
public:
  /**
   * Return 0 if id doesn't exist in context ctx
   */
  GLuint hasTextureObject(GLXContext ctx, GLuint id);

  /**
   * Associate object id  to context ctx
   */
  void addTextureObject(GLXContext ctx, GLuint id);

  /**
   * Return the first texture object id assosiated
   * to context ctx
   */
  GLuint findFirstTextureObject(GLXContext ctx);

private:
  struct listEntry {
    GLuint id;
    GLXContext ctx;
  };

  std::vector<listEntry> textureObjects;
};

#endif
