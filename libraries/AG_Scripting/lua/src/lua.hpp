// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#include <AG_Filesystem/AG_Filesystem.h>
#include <AG_Filesystem/posix_compat.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
