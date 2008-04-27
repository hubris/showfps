#include "GLObjectList.h"

GLuint
GLObjectList::hasTextureObject(GLXContext ctx, GLuint id)
{
  size_t size = textureObjects.size();
  for(size_t i = 0; i < size; i++) {
    listEntry &entry = textureObjects[i];
    if(entry.ctx == ctx && entry.id == id)
      return id;
  }
  return 0;
}

void
GLObjectList::addTextureObject(GLXContext ctx, GLuint id)
{
  if(hasTextureObject(ctx, id))
    return;

  listEntry e;
  e.id = id;
  e.ctx =ctx;
  textureObjects.push_back(e);
}

GLuint
GLObjectList::findFirstTextureObject(GLXContext ctx)
{
 size_t size = textureObjects.size();
 for(size_t i = 0; i < size; i++) {
   listEntry &entry = textureObjects[i];
   if(entry.ctx == ctx)
     return entry.id;
 }
 return 0;
}
