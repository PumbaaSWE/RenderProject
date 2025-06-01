#pragma once
//#include "tdeTopDogEngine.h"
#include "stuff.h"
#include "application.h"
class Camera {

public:
	float yRot = 0;
	float xRot = 0;
	float speed = 100;
	float sens = 100;
	glm::vec3 pos{ 0 };
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);

	Camera(const glm::vec3 pos) :pos(pos) {
		proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f);
		proj[1][1] *= -1;
	}

	void Update(float dt) {
		if (tde::Input::GetKeyUp(tde::KeyCode::C)) {
			pos = glm::vec3(0.0f, 0.0f, -10.0f);
			yRot = 0;
			xRot = 0;
		}
		glm::vec3 fwd = glm::vec4(0, 0, 1, 0) * view; //inverse
		glm::vec3 right = glm::vec4(1, 0, 0, 0) * view;
		if (tde::Input::GetKey(tde::KeyCode::W)) {
			pos += speed * dt * fwd;
		}
		if (tde::Input::GetKey(tde::KeyCode::S)) {
			pos -= speed * dt * fwd;
		}

		if (tde::Input::GetKey(tde::KeyCode::A)) {
			pos += speed * dt * right;
		}
		if (tde::Input::GetKey(tde::KeyCode::D)) {
			pos -= speed * dt * right;
		}

		if (tde::Input::mouseDeltaX != 0) {
			//printf("mouseDeltaX %d\n", tde::Input::mouseDeltaX);
			yRot -= tde::Input::mouseDeltaX * sens * dt;
		}
		if (tde::Input::mouseDeltaY != 0) {
			//printf("mouseDeltaY %d\n", tde::Input::mouseDeltaY);
			xRot -= tde::Input::mouseDeltaY * sens * dt;
		}
		xRot = std::clamp(xRot, glm::radians<float>(-90.0), glm::radians<float>(90.0));

		view = glm::rotate(glm::mat4(1.0f), xRot, { 1,0,0 }); //first rotate around the y axis
		view = glm::rotate(view, yRot, { 0,1,0 }); // rotate around the y axis
		view = glm::translate(view, pos);
	}

	//should be the inverse of it? what did we learn?
	glm::mat4 GetVP() {
		return (proj * view);
	}
};
