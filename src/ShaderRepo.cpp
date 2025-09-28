#include "ShaderRepo.h"
#include "util/filesystem.h"
#include <iostream>

std::unordered_map<char, int> shader_map = {
  { 't', GL_VERTEX_SHADER },         // .vert
  { 'g', GL_FRAGMENT_SHADER },       // .frag
  { 'o', GL_GEOMETRY_SHADER },       // .geo
  { 'p', GL_COMPUTE_SHADER },        // .comp
  { 'l', GL_TESS_CONTROL_SHADER },   // .ctrl
  { 'a', GL_TESS_EVALUATION_SHADER } // .eva
};

ShaderStrongPtr
ShaderRepo::load_shader(const std::string& path)
{
  std::string shader_src = Util::read_file("shaders/" + path);
  const char* shader_src_cstr = shader_src.c_str();
  GLuint shader = glCreateShader(shader_map[path[path.size() - 6]]);

  GLint success;
  glShaderSource(shader, 1, &shader_src_cstr, nullptr);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(shader, 512, nullptr, info);
    std::cerr << "Error while compiling " << path << ":\n" << info << "\n";
  }

  return RepoBase::create(path, shader);
}

ShaderStrongPtr
ShaderRepo::get_shader(const std::string& path)
{
  if (auto shader = RepoBase::get(path); shader.has_value()) {
    return shader.value();
  } else {
    return load_shader(path);
  }
}
