#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

void windowing_error_callback(
	int error,
	const char* description
);

void GLAPIENTRY gl_debug_callback (
	GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam
);

#endif // ERRORHANDLING_H
