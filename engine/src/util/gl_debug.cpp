#include <engine/util/gl_debug.h>
#include <engine/util/log.h>
#include <iostream>

namespace Util {

void APIENTRY
glDebugOutput(GLenum source,
              GLenum type,
              unsigned int id,
              GLenum severity,
              [[maybe_unused]] GLsizei length,
              const char* message,
              [[maybe_unused]] const void* userParam)
{
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;

  Util::log_out() << "---------------\n";
  Util::log_out() << "Debug message (" << id << "): " << message << "\n";

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      Util::log_out() << "Source: API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      Util::log_out() << "Source: Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      Util::log_out() << "Source: Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      Util::log_out() << "Source: Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      Util::log_out() << "Source: Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      Util::log_out() << "Source: Other";
      break;
  }
  Util::log_out() << "\n";

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      Util::log_out() << "Type: Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      Util::log_out() << "Type: Deprecated Behaviour";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      Util::log_out() << "Type: Undefined Behaviour";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      Util::log_out() << "Type: Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      Util::log_out() << "Type: Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      Util::log_out() << "Type: Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      Util::log_out() << "Type: Push Group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      Util::log_out() << "Type: Pop Group";
      break;
    case GL_DEBUG_TYPE_OTHER:
      Util::log_out() << "Type: Other";
      break;
  }
  Util::log_out() << "\n";

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      Util::log_out() << "Severity: high";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      Util::log_out() << "Severity: medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      Util::log_out() << "Severity: low";
      break;
  }
  Util::log_out() << "\n\n";
}

void
init_gl_debug()
{
  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT)) {
    Util::log_out() << "Debug context not available.\n";
    return;
  }

  if (glfwExtensionSupported("GL_KHR_debug")) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE,
                          GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION,
                          0,
                          nullptr,
                          GL_FALSE);
    Util::log_out() << "Successfully initialized GL_KHR_debug.\n";
    return;
  }

  if (glfwExtensionSupported("GL_ARB_debug_output")) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallbackARB(glDebugOutput, nullptr);
    Util::log_out() << "Successfully initialized GL_ARB_debug_output.\n";
    return;
  }

  Util::log_out() << "Neither KHR_debug nor ARB_debug_output are supported.\n";
}

}
