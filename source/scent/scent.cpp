#include "litewq/scent/scent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <random>
#include "GLFW/glfw3.h"
#include <vector>

using namespace litewq;

Scent::Scent(std::default_random_engine &generator, GLShader &shader, float z)
	: generator_(generator), shader_(shader), last_time_(glfwGetTime())
{
	std::uniform_real_distribution<float> distribution(-50.0f, 50.0f);
	source_ = glm::vec3(distribution(generator_), z, distribution(generator_));
	glm::vec3 dest_ = glm::vec3(distribution(generator_), z, distribution(generator_));
	std::uniform_real_distribution<float> color_distribution(0.0f, 1.0f);
	color_ = glm::vec3(color_distribution(generator_), color_distribution(generator_), color_distribution(generator_));
	mesh_ = TriMesh::create_sphere(0.1, 10, 10);
	distance_ = glm::distance(source_, dest_);
	direction_ = glm::normalize(dest_ - source_);

	std::uniform_real_distribution<float> wind_dist(-0.5f, 0.5f);
	wind_ = glm::vec3(wind_dist(generator_), 0, wind_dist(generator_));
}

void Scent::initGL()
{
	mesh_->initGL();
}

void Scent::render(glm::vec3 camera_pos, glm::mat4 view, glm::mat4 projection, glm::vec4 viewport)
{
	double current_time = glfwGetTime();
	std::uniform_real_distribution<float> distribution(-0.1f, 0.1f);
	wind_ += glm::vec3(distribution(generator_), 0, distribution(generator_)) * (float)(current_time - last_time_);

	// update scent
	for (int i = 0; i < scent_.size(); i++)
		scent_[i] += wind_ * (float)(current_time - last_time_) * 4.0f;

	// render grounded track
    shader_.Bind();
    for (int i = 0; i < distance_; i++) {
        glm::vec3 position = source_ + direction_ * (1.0f * i);
        glm::vec3 center = glm::project(position, view, projection, glm::vec4(0, 0, 800, 600));
		glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, glm::vec4(0, 0, 800, 600));
		shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
		shader_.updateUniformFloat3("center", center);
		shader_.updateUniformFloat3("outer", outer);
		mesh_->render();

		// update scent
		std::uniform_real_distribution<float> emit_distribution(0.0f, 1.0f);
		if (emit_distribution(generator_) > std::exp(-0.05 * (current_time - last_time_)))
			scent_.push_back(position + wind_ + glm::vec3(0, 0.6, 0));
    }

    // render floating scent
    std::vector<std::pair<float, int>> scent_distance;
	for (int i = 0; i < scent_.size(); i++)
		scent_distance.push_back(std::make_pair(glm::distance(scent_[i], camera_pos), i));
	glEnable(GL_BLEND);
	for (int i = 0; i < scent_.size(); i++)
	{
		glm::vec3 position = scent_[scent_distance[i].second] + glm::vec3(0, 0.5 * std::sin(current_time * 2.0f + i), 0);
		glm::vec3 center = glm::project(position, view, projection, viewport);
		glm::vec3 outer = glm::project(position + direction_ * 0.1f, view, projection, viewport);
		shader_.updateUniformMat4("model", glm::translate(glm::mat4(1.0f), position));
		shader_.updateUniformFloat3("center", center);
		shader_.updateUniformFloat3("outer", outer);
		mesh_->render();
	}
	glDisable(GL_BLEND);
    shader_.UnBind();

    if (scent_.size() > 1000)
        scent_.erase(scent_.begin(), scent_.begin() + 100);

    last_time_ = current_time;
}