#pragma once
#include "opengl.h"

namespace Util {
void APIENTRY
glDebugOutput(GLenum source,
              GLenum type,
              unsigned int id,
              GLenum severity,
              GLsizei length,
              const char* message,
              const void* userParam);

void
init_gl_debug();
}
