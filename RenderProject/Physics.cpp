#include "Physics.h"

void phys::Physics::Step(float dt)
{
	if (useGravity) {
		vec3_t g{ gravity * dt };
		for (auto& rb : rbs) {
			rb.vel += g;
		}
	}


	vec3_t hit{ 0, 0 ,0 };
	for (auto& rb : rbs) {


		for (auto& pl : planes)
		{
			if (phys::CircleVsPlane(rb.circle, pl, hit)) {
				rb.vel = glm::reflect(rb.vel, pl.n);
			}

		}

	}

	for (auto& rb : rbs) {
		rb.circle.pos += rb.vel * dt;
	}

}

//circle vs infinite plane with one side, true if intersect x = point of contact
bool phys::CircleVsPlane(const Circle& c, const Plane& p, glm::vec3& x) {

	glm::vec3 delta = c.pos - p.pos;
	float dist = glm::dot(p.n, delta);
	if (dist <= c.r) { //circle is in contact with plane or "behind it"
		//project on plane
		x = c.pos - (p.n * dist);
		return true;
	}
	return false;
}

bool phys::CircleVsCircle(const Circle& c1, const Circle& c2) {
	glm::vec3 delta = c2.pos - c1.pos;
	float sq_dist = glm::length2(delta); //if "0" just bleh..
	float r = (c1.r + c2.r);
	return sq_dist <= r * r;
}
