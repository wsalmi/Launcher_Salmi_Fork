#include "fs_wrapper.h"

#include <SD.h>
#include <globals.h>

namespace fswrap {
namespace {
fs::FS *g_backend = &SDM;
}

fs::FS &primary() { return *g_backend; }

bool exists(const char *path) { return primary().exists(path); }

bool exists(const String &path) { return exists(path.c_str()); }

bool mkdir(const char *path) { return primary().mkdir(path); }

bool mkdir(const String &path) { return mkdir(path.c_str()); }

File open(const char *path, const char *mode) { return primary().open(path, mode); }

File open(const char *path, const char *mode, bool create) {
    (void)create;
    return primary().open(path, mode);
}

File open(const String &path, const char *mode) { return open(path.c_str(), mode); }

File open(const String &path, const char *mode, bool create) { return open(path.c_str(), mode, create); }

} // namespace fswrap
