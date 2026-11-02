#include <engine/Shader.h>
#include <engine/repos/ShaderRepo.h>
#include <engine/util/filesystem.h>
#include <engine/util/log.h>
#include <engine/util/opengl.h>
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
  if (auto shader = RepoBase::get(path); shader.has_value()) {
    return shader.value();
  }

  if (path.empty())
    return {};

  const auto shader_type = shader_map.find(path.back());
  if (shader_type == shader_map.end()) {
    Util::log_err() << "Unknown shader type for " << path << '\n';
    return {};
  }

  const std::string shader_src = Util::read_file("shaders/" + path + ".glsl");
  const char* shader_src_cstr = shader_src.c_str();
  const GLuint shader = glCreateShader(shader_type->second);

  GLint success = GL_FALSE;
  glShaderSource(shader, 1, &shader_src_cstr, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    char info[512] = {};
    glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
    Util::log_err() << "Error while compiling " << path << ":\n"
                    << info << '\n';
    glDeleteShader(shader);
    return {};
  }

  return RepoBase::create(path, shader);
}
