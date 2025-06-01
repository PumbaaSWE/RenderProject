#pragma once
//#include "tdeTopDogEngine.h"
#include "stuff.h"
//#include <glm/gtx/norm.hpp>



namespace phys {

	struct Circle {
		glm::vec3 pos{ 0,0,0 };
		float r{ 0.5f };
	};

	struct Plane {
		glm::vec3 pos{ 0,0,0 };
		glm::vec3 n{ 0,1,0 };
	};

	//more like physics object?
	struct Rigidbody {
		phys::Circle circle; //"the collider"
		vec3_t vel;
		float bouncyness = 1.0f;
	};


	//circle vs infinite plane with one side, true if intersect x = point of contact
	bool CircleVsPlane(const Circle& c, const Plane& p, glm::vec3& x);

	////noop
	//glm::vec3 ProjectOnPlane(const glm::vec3& point, const glm::vec3& planeNormal) {

	//	return { 0,0,0 };
	//}

	bool CircleVsCircle(const Circle& c1, const Circle& c2);

	class Physics
	{

	public:
		bool useGravity = true;
		vec3_t gravity{ 0, -9.82 ,0 };
		std::vector<Rigidbody> rbs;
		std::vector<Plane> planes;
		void Step(float dt);
	};
}



