#pragma once

#include <FS.h>
#include <WString.h>

namespace fswrap {

using File = fs::File;

// Access the active filesystem backend (defaults to SDM / SD)
fs::FS &primary();

bool exists(const char *path);
bool exists(const String &path);
bool mkdir(const char *path);
bool mkdir(const String &path);

File open(const char *path, const char *mode);
File open(const char *path, const char *mode, bool create);
File open(const String &path, const char *mode);
File open(const String &path, const char *mode, bool create);

} // namespace fswrap
