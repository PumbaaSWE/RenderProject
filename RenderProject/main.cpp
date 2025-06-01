#include "application.h"
#include "obj_loader.h"
#include "Model.h"
#include "Physics.h"
#include "Camera.h"

class Application1 : public tde::Application
{
	phys::Physics physics;

	tde::Model cube;
	tde::Model sphere;
	tde::Model plane;

	std::unique_ptr<Camera> camera;

public:
	Application1()
	{
		appName = "Simple Physics Simulation";
	}

	void Init() override {

		camera = std::make_unique<Camera>(glm::vec3(0.0f, -10.0f, -10.0f));
		//camera->sens = 10;

		cube = tde::Model(&GetRenderer(), tde::Model::cube_verts, tde::Model::cube_indices);
		sphere = tde::Model(&GetRenderer(), tde::Model::sphere_verts, tde::Model::sphere_indices);
		plane = tde::Model(&GetRenderer(), tde::Model::plane_verts, tde::Model::plane_indices);

		//Load a model from file
		std::vector<obj_loader::Vertex> v;
		std::vector<uint16_t> i;
		if (obj_loader::LoadFromFile("cow.obj", v, i))
		{
			std::vector<tde::Vertex> vertices;
			vertices.reserve(v.size());
			for (const auto& vertex : v)
			{
				tde::Vertex tdeVertex;
				tdeVertex.pos = vertex.pos;
				tdeVertex.normal = vertex.normal;
				vertices.push_back(tdeVertex);
			}
			sphere = tde::Model(&GetRenderer(), vertices, i);
		}

		//Add some rigidbodies (spheres)
		for (size_t i = 0; i < 15; i++)
		{
			physics.rbs.emplace_back();
			//physics.rbs[i].circle.pos = { i, 15, 0 };
			physics.rbs[i].circle.pos = { rand() % 20 - 10, rand() % 15 + 15, rand() % 20 - 10 };
		}

		//Add some planes
		physics.planes.resize(2);
		physics.planes[0].pos = { 0,0,0 };
		physics.planes[0].n = glm::normalize(vec3_t{ -1, 1, 0 });
		physics.planes[1].pos = { 0,0,0 };
		physics.planes[1].n = glm::normalize(vec3_t{ 1, 1, 0 });
	}

	void FixedUpdate(float dt) override {
		physics.Step(dt);
	}

	void Update(float dt) override {
		camera->Update(dt);
	}

	void Render(float dt, float extrapolation) override {

		renderer->SetUniformBuffer(glm::mat4(1.0f), camera->view, camera->proj);

		//Render spheres
		for (auto& rb : physics.rbs) {
			glm::mat4 t = glm::mat4(1.0f);
			t = glm::translate(t, rb.circle.pos);

			t = glm::scale(t, glm::vec3(0.25f,
										0.25f,
										0.25f)); // cow scale
			sphere.Draw(t);
		}

		//Render planes
		for (auto& p : physics.planes) {
			glm::vec3 forward = -glm::normalize(p.n); // since quad model faces -Z

			// Pick a suitable up vector not parallel to forward
			glm::vec3 up = glm::abs(forward.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

			// Build orthonormal basis
			glm::vec3 xAxis = glm::normalize(glm::cross(up, forward));   // right
			glm::vec3 yAxis = glm::normalize(glm::cross(forward, xAxis)); // up
			glm::vec3 zAxis = forward; // forward direction for the quad (-normal)

			// Build rotation matrix
			glm::mat4 rotation(1.0f);
			rotation[0] = glm::vec4(xAxis, 0.0f);
			rotation[1] = glm::vec4(yAxis, 0.0f);
			rotation[2] = glm::vec4(zAxis, 0.0f);

			// Add translation
			glm::mat4 t = glm::translate(glm::mat4(1), p.pos);
			t = t * rotation;

			t = glm::translate(t, glm::vec3(0.0f,
											25.0f,
											0.0f)); // Translate after rotating to move in local space

			t = glm::scale(t, glm::vec3(25.0f,
										25.0f,
										25.0f)); // Scale the quad

			plane.Draw(t);
		}
	}
};

int main()
{
	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}