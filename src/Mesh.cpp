#include "Mesh.h"
#include "opengl.h"

static int layout_sum(const std::vector<int>& layout) {
    int sum = 0;
    for (int x : layout) sum += x;
    return sum;
}

static int stride_bytes(const std::vector<int>& layout) {
    return layout_sum(layout) * sizeof(float);
}

void Mesh::setup(const std::vector<float>& vertices,
                 const std::vector<unsigned int>& indices,
                 const std::vector<int>& layout) 
{
    vertex_count = vertices.size() / layout_sum(layout);
    index_count = indices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    int stride = stride_bytes(layout);
    int offset = 0;
    for (unsigned int i = 0; i < layout.size(); i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
        offset += layout[i] * sizeof(float);
    }

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    if (index_count > 0) {
        glDrawElements(GL_TRIANGLES, static_cast<int>(index_count), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertex_count));
    }
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}
