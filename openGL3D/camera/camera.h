#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	ASCEND,
	DESCEND
};

//default camera values:
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

//player movement vars:
extern float verticalVelocity;
extern bool isJumping;
extern float groundHeight;

class Camera {
public:
	//camera attributes:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 Worldup;
	//euler angles
	float Yaw;
	float Pitch;
	//camera options
	float MoveSpeed;
	float MouseSens;
	float Zoom;

	//constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MoveSpeed(SPEED), MouseSens(SENSITIVITY), Zoom(ZOOM) {
		Position = position;
		Worldup = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	//constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MoveSpeed(SPEED), MouseSens(SENSITIVITY), Zoom(ZOOM) {
		Position = glm::vec3(posX, posY, posZ);
		Worldup = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	//returns the view matrix calculated using euler angles and the lookat matrix:
	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position + Front, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool DEBUG) {
		if (DEBUG) return; // we want to use DebugProcessKeyboard in Opengl3d.cpp for debug mode so we can fly around instead of walking.
		float velocity = MoveSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (!isJumping) // keep player on ground
			Position.y = groundHeight;
	}

	void DebugProcessKeyboard(Camera_Movement direction, float deltaTime, bool DEBUG) {
		if (DEBUG) {
			float velocity = MoveSpeed * deltaTime;
			if (direction == FORWARD)
				Position += Front * velocity;
			if (direction == BACKWARD)
				Position -= Front * velocity;
			if (direction == LEFT)
				Position -= glm::normalize(glm::cross(Front, Up)) * velocity;
			if (direction == RIGHT)
				Position += glm::normalize(glm::cross(Front, Up)) * velocity;
			if (direction == ASCEND)
				Position += Up * velocity;
			if (direction == DESCEND)
				Position -= Up * velocity;
		}
	}

	void HandleJump(float deltaTime, bool DEBUG) {
		if (DEBUG) return;
		if (!isJumping) {
			isJumping = true;
			verticalVelocity = 5.0f;
		}
	}

	void updatePhysics(float deltaTime, bool DEBUG) {
		if (DEBUG) return;
		if (isJumping) {

			// apply gravity
			verticalVelocity -= 9.8f * deltaTime;

			// position update
			Position.y += verticalVelocity * deltaTime;

			//grounded check
			if (Position.y <= groundHeight) {
				Position.y = groundHeight;
				isJumping = false;
				verticalVelocity = 0.0f;
			}
		}
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		xoffset *= MouseSens;
		yoffset *= MouseSens;

		Yaw += xoffset;
		Pitch += yoffset;

		//make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, Worldup));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif