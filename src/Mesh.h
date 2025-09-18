#pragma once
#include "opengl.h"
#include <vector>

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    size_t vertex_count = 0;
    size_t index_count = 0;

    void setup(const std::vector<GLfloat>& vertices,
               const std::vector<GLuint>& indices = {},
               const std::vector<int>& layout = {3});

    void draw() const;
    ~Mesh();
};
