#include "LibraryLoader.h"

#include <dlfcn.h>
#include <sstream>
#include <cassert>
#include <iostream>

void
LibraryLoader::open(const char *libFilename)
{
  using namespace std;

  handle = dlopen(libFilename, RTLD_LAZY);
  if(!handle) {
    std::cerr << "Libraryloader::open(): dlopen() error: " << dlerror() << ".";
  }
}

void *
LibraryLoader::getSym(const char *symName)
{
  assert(handle);
  void *addr = dlsym(handle, symName);
  char *error = dlerror();
  if(error) {
    std::cerr << "Libraryloader::getSym(): dlsym() error: " << error << ".";
  }
  return addr;
}

void *
LibraryLoader::getNextSym(const char *symName)
{
  void *addr = dlsym(RTLD_NEXT, symName);
  char *error = dlerror();
  if(error) {
    std::cerr << "Libraryloader::getSym(): dlsym() error: " << error << ".";
  }
  return addr;
}

void *
LibraryLoader::getNextSym(const std::string& symName)
{
  getNextSym(symName.c_str());
}

void
LibraryLoader::close()
{
  assert(handle);
  dlclose(handle);
  handle = 0;
}
