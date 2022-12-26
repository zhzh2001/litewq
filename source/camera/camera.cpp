#include "litewq/camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace litewq;

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: position_(position),
	  front_(glm::vec3(0.0f, 0.0f, -1.0f)),
	  world_up_(up),
	  yaw_(yaw),
	  pitch_(pitch),
	  movement_speed_(SPEED),
	  mouse_sensitivity_(SENSITIVITY),
	  zoom_(ZOOM),
	  fly_mode_(false)
{
	update_camera_vectors();
}

void Camera::update_camera_vectors()
{
	glm::vec3 front, look_front;
	front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	if (fly_mode_)
		front.y = sin(glm::radians(pitch_));
	else
		front.y = 0.0f;
	front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	look_front = front;
	look_front.y = sin(glm::radians(pitch_));
	front_ = glm::normalize(front);
	look_front_ = glm::normalize(look_front);

	right_ = glm::normalize(glm::cross(front_, world_up_));
	up_ = glm::normalize(glm::cross(right_, front_));
}

glm::mat4 Camera::get_view_matrix() const
{
	return glm::lookAt(position_, position_ + look_front_, up_);
}

float Camera::get_zoom() const
{
	return glm::radians(zoom_);
}

glm::vec3 Camera::get_position() const
{
	return position_;
}

void Camera::process_keyboard(Camera_Movement direction, float delta_time)
{
	float velocity = movement_speed_ * delta_time;
	if (direction == FORWARD)
		position_ += front_ * velocity;
	if (direction == BACKWARD)
		position_ -= front_ * velocity;
	if (direction == LEFT)
		position_ -= right_ * velocity;
	if (direction == RIGHT)
		position_ += right_ * velocity;
}

void Camera::process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch)
{
	x_offset *= mouse_sensitivity_;
	y_offset *= mouse_sensitivity_;

	yaw_ += x_offset;
	pitch_ += y_offset;

	if (constrain_pitch)
	{
		if (pitch_ > 89.0f)
			pitch_ = 89.0f;
		if (pitch_ < -89.0f)
			pitch_ = -89.0f;
	}

	update_camera_vectors();
}

void Camera::process_mouse_scroll(float y_offset)
{
	if (zoom_ >= 1.0f && zoom_ <= 45.0f)
		zoom_ -= y_offset;
	if (zoom_ <= 1.0f)
		zoom_ = 1.0f;
	if (zoom_ >= 45.0f)
		zoom_ = 45.0f;
}

void Camera::toggle_fly_mode()
{
	fly_mode_ = !fly_mode_;
}