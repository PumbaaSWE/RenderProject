#include "application.h"
#include "obj_loader.h"
#include "Model.h"
#include "Physics.h"
#include "Camera.h"

class Application1 : public tde::Application
{
	class Sphere {
	public:
		tde::Model* model;
		glm::vec3 pos;
		float radius;

		void Draw() {
			glm::mat4 transform = glm::mat4(1.0f);
			float radius = this->radius * 0.25f; // Adjust scale based on the sphere model...
			transform = glm::translate(transform, pos);
			transform = glm::scale(transform, { radius, radius, radius });
			model->Draw(transform);
		}
	};

	float time = 0;
	int secs = 0;
	bool showTime = false;

	tde::Model model;
	std::vector<Sphere> spheres;

	phys::Physics physics;
	tde::Model cube;
	tde::Model sphere;
	tde::Model plane;
	std::unique_ptr<Camera> camera;

public:
	Application1()
	{
		appName = "Top Dog";
	}

	void Init() override {

		camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, -10.0f));
		//camera->sens = 10;

		std::vector<obj_loader::Vertex> v;
		std::vector<uint16_t> i;

		if (obj_loader::LoadFromFile("sphere.obj", v, i))
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
			model = tde::Model(&GetRenderer(), vertices, i);
		}

		// Test spheres
		spheres.push_back({ &model, { 0, 0, -5}, 1.0f});
		spheres.push_back({ &model, { 2, 0, -5 }, 2.0f });
		spheres.push_back({ &model, { 5, 0, -5 }, 3.0f });



		cube = tde::Model(&GetRenderer(), tde::Model::cube_verts, tde::Model::cube_indices);
		sphere = tde::Model(&GetRenderer(), tde::Model::sphere_verts, tde::Model::sphere_indices);
		plane = tde::Model(&GetRenderer(), tde::Model::plane_verts, tde::Model::plane_indices);

		for (size_t i = 0; i < 10; i++)
		{
			physics.rbs.emplace_back();
			physics.rbs[i].circle.pos = { i, 15, 0 };
		}
		//add some planes
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


		mat4_t proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f); //this only change when fov or zNear/zFar changes
		proj[1][1] *= -1; //glm is flipped (OpenGL v Vulkan up? y neg up or down?)

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, { 0, 0, -10 });


		glm::mat4 identity = glm::mat4(1.0f); //this is currently not used
		renderer->SetUniformBuffer(identity, camera->view, camera->proj);

		for (auto& sphere : spheres)
			sphere.Draw();


		for (auto& rb : physics.rbs) {
			glm::mat4 t = glm::mat4(1.0f);
			t = glm::translate(t, rb.circle.pos);
			sphere.Draw(t); //or don't draw... 
		}

		for (auto& p : physics.planes) {
			glm::vec3 forward = -glm::normalize(p.n); // since quad faces -Z

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
			t = glm::translate(t, glm::vec3(0.0f, 25.0f, 0.0f)); // Translate after rotating to move in local space
			t = glm::scale(t, glm::vec3(25.0f, 25.0f, 25.0f)); // Scale the quad

			plane.Draw(t);
		}
	}
};

std::vector<char> read_file(const char* filePath) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		//return nullptr;
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();

	std::vector<char> buffer(fileSize );

	// put file cursor at beginning
	file.seekg(0);

	// load the entire file into the buffer
	file.read(buffer.data(), fileSize);

	// now that the file is loaded into the buffer, we can close it
	file.close();

	return buffer;
}



int main()
{
	
	
	//std::vector<uint16_t> indices = {
	//		0, 2, 1, 2, 0, 3
	//};

	//std::vector<obj_loader::Vertex> verts;
	//obj_loader::LoadFromFile("sphere.obj", verts, indices);
	//std::ofstream myfile;
	//myfile.open("example.txt");
	//
	//myfile << std::fixed << std::setprecision(3);


	//auto res = [](float x) {
	//	return x >= 0 ? ' ' + x : x;
	//};

	//myfile << "std::vector<vec3_t> verts = {" << std::endl;
	//for (size_t i = 0; i < verts.size(); i++)
	//{
	//	myfile << "{{ \t" << verts[i].pos.x << "\t, \t" << verts[i].pos.y << "\t, \t" << verts[i].pos.z << "\t}, \t";
	//	myfile << "{ \t" << verts[i].normal.x << "\t, \t" << verts[i].normal.y << "\t, \t" << verts[i].normal.z << "\t},";
	//	myfile << "{ \t" << verts[i].uv.x << "\t, \t" << verts[i].uv.y << "\t}},";
	//	myfile << std::endl;
	//}
	//myfile << "};" << std::endl;
	//myfile << "std::vector<int> indices = {" << std::endl;
	//for (size_t i = 0; i < indices.size(); i+=3)
	//{
	//	myfile << "\t" << indices[i] << "\t, \t" << indices[i+1] << "\t, \t" << indices[i+2] << ",";
	//	myfile << std::endl;
	//}
	//myfile << "};";
	//myfile.close();
	//
	//return 0;


	//std::ofstream myfile;
	//myfile.open("example.txt");

	//auto frag = read_file("shaders/frag.spv");
	//auto vert = read_file("shaders/vert.spv");

	//std::string s;
	//s.reserve(frag.size());

	//myfile << "{ ";
	//for (size_t i = 0; i < frag.size(); i++) {
	//	//s.push_back(frag[i]);
	//	myfile << (int)frag[i];
	//	if(i < frag.size() - 1 )myfile << ", ";
	//}
	//myfile << " };";

	//myfile.close();
	//
	//return 0;

	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}