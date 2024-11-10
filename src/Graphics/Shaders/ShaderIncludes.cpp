/*
 * This file is auto-generated. Do not modify it manually.
 * Any changes made will be overwritten the next time the script is run.
 */

#include <Graphics/Shaders/ShaderIncludes.h>
#include <Graphics/Shaders/ShaderManager.h>
#include <glad/gl.h>

ShaderManager::ShaderManager() {
	GLuint cube_vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(cube_vert_shader, 1, &cube_vert_glsl, nullptr);
	glCompileShader(cube_vert_shader);

	GLuint cube_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(cube_frag_shader, 1, &cube_frag_glsl, nullptr);
	glCompileShader(cube_frag_shader);


	GLuint cube_program = glCreateProgram();
	glAttachShader(cube_program, cube_vert_shader);
	glAttachShader(cube_program, cube_frag_shader);
	glLinkProgram(cube_program);
	database["cube"] = cube_program;


	glDeleteShader(cube_frag_shader);
	glDeleteShader(cube_vert_shader);
}