#pragma once
#include <glm/glm.hpp>
#include <random>
#include <litewq/mesh/TriMesh.h>
#include <litewq/platform/OpenGL/GLShader.h>
#include <vector>

namespace litewq
{
	class Scent
	{
	public:
		Scent(std::default_random_engine &generator, GLShader &shader, float z);
		void initGL();
		void render(glm::vec3 camera_direction, glm::mat4 view, glm::mat4 projection);

	private:
		glm::vec3 source_;
		glm::vec3 direction_;
		glm::vec3 color_;
		// almost like a line?
		std::default_random_engine &generator_;
		std::unique_ptr<Mesh> mesh_;
		GLShader &shader_;
		std::vector<glm::vec3> scent_;
		int distance_;
		double last_time_;
	};
}