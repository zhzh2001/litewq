#pragma once

#include <glm/glm.hpp>

namespace litewq
{
	const float SPEED = 2.5f;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;

	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class Camera
	{
	public:
		Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

		glm::mat4 get_view_matrix() const;
		float get_zoom() const;
		glm::vec3 get_position() const;

		void process_keyboard(Camera_Movement direction, float delta_time);
		void process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch = true);
		void process_mouse_scroll(float y_offset);

		void toggle_fly_mode();
        glm::vec3 get_view_pos() const {
            return position_;
        }
        glm::vec3 get_view_dir() const {
            return look_front_;
        }


	private:
		glm::vec3 position_;
		glm::vec3 front_;
		glm::vec3 up_;
		glm::vec3 right_;
		glm::vec3 world_up_;
		glm::vec3 look_front_;

		float yaw_;
		float pitch_;

		float movement_speed_;
		float mouse_sensitivity_;
		float zoom_;

		bool fly_mode_;

		void update_camera_vectors();
	};
} // namespace litewq