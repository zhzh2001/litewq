#pragma once
#include <glm/glm.hpp>
#include <random>
#include <litewq/mesh/TriMesh.h>
#include <litewq/platform/OpenGL/GLShader.h>

namespace litewq
{
	class Scent
	{
	public:
		Scent(std::default_random_engine &generator, GLShader &shader);
		void initGL();
		void render(glm::vec3 camera_direction);

	private:
		glm::vec3 source_;
		glm::vec3 dest_;
		glm::vec3 color_;
		// almost like a line?
		std::default_random_engine &generator_;
		std::unique_ptr<Mesh> mesh_;
		GLShader &shader_;
	};
}