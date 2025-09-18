#pragma once
#include <vector>

struct Mesh {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    size_t vertex_count = 0;
    size_t index_count = 0;

    void setup(const std::vector<float>& vertices,
               const std::vector<unsigned int>& indices = {},
               const std::vector<int>& layout = {3});

    void draw() const;
    ~Mesh();
};
