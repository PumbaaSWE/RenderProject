#include "application.h"
#include "obj_loader.h"


class Application1 : public tde::Application
{

	float time = 0;
	int secs = 0;
	bool showTime = false;



public:
	Application1()
	{
		appName = "Woop";
		//mvp = 
	}

	void Init() override {

	}


	void FixedUpdate(float dt) override {

	}


	void Update(float dt) override {


	}
	void Render(float dt, float extrapolation) override {

	}

};





int main()
{
	
	
	//std::vector<uint16_t> indices = {
	//		0, 2, 1, 2, 0, 3
	//};

	//std::vector<obj_loader::Vertex> verts;
	//obj_loader::LoadFromFile("cube_triangulated.obj", verts, indices);
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
	
	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}