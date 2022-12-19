#include "litewq/scent/scent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <random>
#include "GLFW/glfw3.h"

using namespace litewq;

Scent::Scent(std::default_random_engine &generator, GLShader &shader, float z)
	: generator_(generator), shader_(shader), last_time_(glfwGetTime())
{
	std::uniform_real_distribution<float> distribution(-10.0f, 10.0f);
	source_ = glm::vec3(distribution(generator_), z, distribution(generator_));
	glm::vec3 dest_ = glm::vec3(distribution(generator_), z, distribution(generator_));
	std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);
	color_ = glm::vec3(color_distribution(generator_), color_distribution(generator_), color_distribution(generator_));
	mesh_ = TriMesh::create_sphere(0.1, 10, 10);
	distance_ = glm::distance(source_, dest_);
	direction_ = glm::normalize(dest_ - source_);
}

void Scent::initGL()
{
	mesh_->initGL();
}

void Scent::render(glm::vec3 camera_direction, glm::mat4 view, glm::mat4 projection)
{
	/*
	if (glm::dot(direction_, camera_direction) < 0)
		for (int i = 0; i < distance_; i++)
		{
			glm::vec3 position = source_ + direction_ * (1.0f * i);
			glm::vec3 center = glm::project(position, view, projection, glm::vec4(0, 0, 800, 600));
			glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, glm::vec4(0, 0, 800, 600));
			shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
			shader_.updateUniformFloat3("center", center);
			shader_.updateUniformFloat3("outer", outer);
			mesh_->render();
		}
	else
		for (int i = distance_; i > 0; i--)
		{
			glm::vec3 position = source_ + direction_ * (1.0f * i);
			glm::vec3 center = glm::project(position, view, projection, glm::vec4(0, 0, 800, 600));
			glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, glm::vec4(0, 0, 800, 600));
			shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
			shader_.updateUniformFloat3("center", center);
			shader_.updateUniformFloat3("outer", outer);
			mesh_->render();
		}
	*/
	double current_time = glfwGetTime();
	std::uniform_real_distribution<float> distribution(-0.05f, 0.5f);
	glm::vec3 wind(distribution(generator_), 0.5 * std::sin(current_time), distribution(generator_));

	// update scent
	for (int i = 0; i < scent_.size(); i++)
		scent_[i] += wind * (float)(current_time - last_time_);

	// render grounded track
	for (int i = 0; i < distance_; i++)
	{
		glm::vec3 position = source_ + direction_ * (1.0f * i);
		glm::vec3 center = glm::project(position, view, projection, glm::vec4(0, 0, 800, 600));
		glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, glm::vec4(0, 0, 800, 600));
		shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
		shader_.updateUniformFloat3("center", center);
		shader_.updateUniformFloat3("outer", outer);
		mesh_->render();

		// update scent
		std::uniform_real_distribution<float> emit_distribution(0.0f, 1.0f);
		if (emit_distribution(generator_) < 0.01)
			scent_.push_back(position + wind);
	}

	// render floating scent
	glEnable(GL_BLEND);
	for (int i = 0; i < scent_.size(); i++)
	{
		glm::vec3 position = scent_[i];
		glm::vec3 center = glm::project(position, view, projection, glm::vec4(0, 0, 800, 600));
		glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, glm::vec4(0, 0, 800, 600));
		shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
		shader_.updateUniformFloat3("center", center);
		shader_.updateUniformFloat3("outer", outer);
		mesh_->render();
	}
	glDisable(GL_BLEND);

	if (scent_.size() > 1000)
		scent_.erase(scent_.begin(), scent_.begin() + 100);

	last_time_ = current_time;
}