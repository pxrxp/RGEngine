#include "Repo.h"
ShaderProgramRepo::~ShaderProgramRepo() {
  for (auto& program : m_resources) {
    glDeleteProgram(program.second->get_id());
  }
  m_resources.clear();
}
