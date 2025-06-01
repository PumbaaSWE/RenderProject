#include "application.h"
#include "obj_loader.h"
#include "Model.h"

class Application1 : public tde::Application
{
	class Sphere {
	public:
		tde::Model* model;
		glm::vec3 pos;
		float radius;

		void Draw() {
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::scale(transform, { radius, radius, radius });	// Adjust scale based on the sphere model...
			transform = glm::translate(transform, pos);
			model->Draw(transform);
		}
	};

	float time = 0;
	int secs = 0;
	bool showTime = false;

	tde::Model model;
	std::vector<Sphere> spheres;

public:
	Application1()
	{
		appName = "Top Dog";
	}

	void Init() override {
		model = tde::Model(&GetRenderer(), tde::Model::cube_verts, tde::Model::cube_indices);

		// Test spheres.. but cubes right now
		spheres.push_back({ &model, { 0, 0, -5}, 1.0f});
		spheres.push_back({ &model, { 2, 0, -5 }, 2.0f });
		spheres.push_back({ &model, { 5, 0, -5 }, 3.0f });
	}


	void FixedUpdate(float dt) override {

	}


	void Update(float dt) override {


	}
	void Render(float dt, float extrapolation) override {


		mat4_t proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f); //this only change when fov or zNear/zFar changes
		proj[1][1] *= -1; //glm is flipped (OpenGL v Vulkan up? y neg up or down?)

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, { 0, 0, -10 });


		glm::mat4 identity = glm::mat4(1.0f); //this is currently not used
		renderer->SetUniformBuffer(identity, view, proj);

		for (auto& sphere : spheres)
			sphere.Draw();
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
	//obj_loader::LoadFromFile("D:\\Blender\\cube\\cube.obj", verts, indices);
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
	//	myfile << "{ \t" << verts[i].pos.x << "\t, \t" << verts[i].pos.y << "\t, \t" << verts[i].pos.z << "\t}, \t";
	//	myfile << "{ \t" << verts[i].normal.x << "\t, \t" << verts[i].normal.y << "\t, \t" << verts[i].normal.z << "\t},";
	//	myfile << "{ \t" << verts[i].uv.x << "\t, \t" << verts[i].uv.y << "\t},";
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