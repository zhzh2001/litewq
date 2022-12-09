#include "litewq/scent/scent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <random>

using namespace litewq;

Scent::Scent(std::default_random_engine &generator, GLShader &shader)
	: generator_(generator), shader_(shader)
{
	std::uniform_real_distribution<float> distribution(-10.0f, 10.0f);
	source_ = glm::vec3(distribution(generator_), 0.5, distribution(generator_));
	dest_ = glm::vec3(distribution(generator_), 0.5, distribution(generator_));
	std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);
	color_ = glm::vec3(color_distribution(generator_), color_distribution(generator_), color_distribution(generator_));
	mesh_ = TriMesh::create_sphere(0.1, 10, 10);
}

void Scent::initGL()
{
	mesh_->initGL();
}

void Scent::render(glm::vec3 camera_direction)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int distance = glm::distance(source_, dest_);
	glm::vec3 direction = glm::normalize(dest_ - source_);
	if (glm::dot(direction, camera_direction) < 0)
		for (int i = 0; i < distance; i++)
		{
			glm::vec3 position = source_ + direction * (1.0f * i);
			shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
			mesh_->render();
		}
	else
		for (int i = distance; i > 0; i--)
		{
			glm::vec3 position = source_ + direction * (1.0f * i);
			shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
			mesh_->render();
		}
	glDisable(GL_BLEND);
}