#ifndef LIBRARYLOADER_H
#define LIBRARYLOADER_H

#include <string>

class LibraryLoader {
public:
  LibraryLoader():handle(0) {}

  void open(const char *libFilename);
  void *getSym(const char *symName);
  void close();

  static void *getNextSym(const char *symName);
  static void *getNextSym(const std::string& symName);

private:
  void *handle;
};

#endif
