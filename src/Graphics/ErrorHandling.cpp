#include <Graphics/ErrorHandling.h>
#include <iostream>

void windowing_error_callback(int error, const char* description) {
	std::cerr << "Error: %s\n" << description << std::endl;
}

void GLAPIENTRY gl_debug_callback (
	GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam
) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131222) return;
	/*if (type == GL_DEBUG_TYPE_PERFORMANCE) return;*/

	std::clog << "---------------" << std::endl;
	std::clog << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::clog << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::clog << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::clog << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::clog << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::clog << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::clog << "Source: Other"; break;
	} std::clog << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::clog << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::clog << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::clog << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::clog << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::clog << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::clog << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::clog << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::clog << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::clog << "Type: Other"; break;
	} std::clog << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::clog << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::clog << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::clog << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::clog << "Severity: notification"; break;
	} std::clog << std::endl;
	std::clog << std::endl;
}
