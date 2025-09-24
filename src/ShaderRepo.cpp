#include "Repo.h"
#include "opengl.h"
#include "util.h"
#include <iostream>

#include "debug.h"

std::unordered_map<char, int> shader_map = {
    {'t', GL_VERTEX_SHADER},         // .vert
    {'g', GL_FRAGMENT_SHADER},       // .frag
    {'o', GL_GEOMETRY_SHADER},       // .geo
    {'p', GL_COMPUTE_SHADER},        // .comp
    {'l', GL_TESS_CONTROL_SHADER},   // .ctrl
    {'a', GL_TESS_EVALUATION_SHADER} // .eva
};

GLuint ShaderRepo::load_and_store_shader(const std::string& path) {
  std::string shader_src = read_file("shaders/" + path);
  const char *shader_src_cstr = shader_src.c_str();
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

  m_shaders[path] = shader;
  return shader;
}


GLuint ShaderRepo::get_shader(const std::string& path) {
  bool not_found = m_shaders.find(path) == m_shaders.end();
  if (not_found) {
    return load_and_store_shader(path);
  } else {
    return m_shaders[path];
  }
}


ShaderRepo::~ShaderRepo() {
    for (auto& [path, shader] : m_shaders)
        glDeleteShader(shader);
}

void ShaderRepo::remove_shader(const std::string& path) {
    auto it = m_shaders.find(path);
    if (it != m_shaders.end()) {
        glDeleteShader(it->second);
        m_shaders.erase(it);
    }
}
