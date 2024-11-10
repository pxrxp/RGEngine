#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include <memory>

#include <Core/ServiceLocator.h>
#include <Core/SystemInitializer.h>

#include <Components/CameraComponent.h>

int main() {
	Window window = ServiceLocator::get_window_manager()
		.create_window(
			"RivalGrounds",
			glfwGetPrimaryMonitor(),
			DEFAULT_DIMENSIONS
		);

	INITIALIZE_SYSTEMS()
	
	constexpr float FOV = 45.0f;
	constexpr float NEAR = 45.0f;
	constexpr float FAR = 45.0f;

	Entity camera = ServiceLocator::get_entity_manager().create_entity();

	auto camera_component = std::make_unique<CameraComponent>();
	camera_component->up = glm::vec3(0, 1, 0);
	camera_component->projection = glm::perspectiveLH(glm::radians(FOV), window.get_aspect_ratio(), NEAR, FAR);

	ServiceLocator::get_component_manager()
		.set_component<CameraComponent>(camera, std::move(camera_component));

	GLuint vao, vbo, ebo;
	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glCreateBuffers(1, &ebo);

	GLfloat vertices[] = {
		-0.5, -0.5, -0.5, 1, 0, 0, // FBL
		-0.5, 0.5, -0.5, 0, 1, 0,  //FTL
		0.5, 0.5, -0.5, 0, 0, 1, //FTR
		0.5, -0.5, -0.5, 0, 1, 0,//FBR
		-0.5, -0.5, -1, 0, 0, 1,
		-0.5, 0.5, -1, 1, 0,0,
		0.5, 0.5, -1, 0, 1, 0,
		0.5, -0.5, -1, 1, 0, 0
	};

	GLuint indices[] = {
		0, 1, 2,
		2, 3, 0,
		0, 1, 5,
		5, 4, 0,
		4, 5, 6,
		6, 7, 4,
		6, 7, 3,
		3, 2, 6
	};

	glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(ebo, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(vao, 1);
	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat));

	glVertexArrayVertexBuffer(vao, 0, vbo, 0, 6 * sizeof(GLfloat));
	glVertexArrayElementBuffer(vao, ebo);

	while (window.is_open()) {
		window.init_graphics();

		std::clog << ServiceLocator::get_component_manager().get_component<CameraComponent>(camera)->up.y;

		ServiceLocator::get_system_manager().update();

		glUseProgram(ServiceLocator::get_shader_manager().get("cube"));
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);

		window.render();
	}
}
