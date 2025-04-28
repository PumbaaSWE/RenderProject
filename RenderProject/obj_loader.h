#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>

namespace obj_loader {

	using vec3_t = glm::vec3;
	using vec2_t = glm::vec2;

	struct Vertex
	{
		vec3_t pos;
		vec3_t normal;
		vec2_t uv;
	};

	struct Index
	{
		uint16_t v;
		uint16_t vn;
		uint16_t vu;
	};

static bool LoadFromFile(const std::string& fileName, std::vector<Vertex>& verticies, std::vector<uint16_t>& indicies) {

	std::ifstream f(fileName);
	if (!f.is_open()) return false;


	std::vector<vec3_t> pos;
	std::vector<vec3_t> normals;
	//std::vector<vec2_t> uvs;
	//std::vector<uint16_t> tris;
	verticies.clear();
	indicies.clear();

	std::string line;
	std::string idx_pos, idx_norm, idx_uv;
	while (std::getline(f, line))
	{
		std::stringstream s;

		s << line;

		std::string begin;
		s >> begin;

		vec3_t v;

		//starts with v_BLANK_ ...bugs... vn starts with v too -.- dont add verts on that!!!
		if (line.starts_with("v ")) {
			s >> v.x >> v.y >> v.z;
			pos.push_back(v);
			Vertex a;
			a.pos = v;
			a.normal = { 1,1,1 };
			verticies.push_back(a);
		}
		else if (line.starts_with("f")) //face i.e. triangle index
		{
			//this is a pain! f can be v1 OR v1/v2 OR v1/v2/v3 OR v1//v3 depending... 1 is index, 2 is texture index, 3 is normal index
			//as of now all things needs to be turned off when exporting to get the first version!!!
			//https://en.wikipedia.org/wiki/Wavefront_.obj_file
			//https://stackoverflow.com/questions/52824956/how-can-i-parse-a-simple-obj-file-into-triangles
			std::vector<Index> indexes;
			std::string index_string;
			//PRINT("line string = " << line);

			while (s >> index_string) {
				//PRINTL("Index string = " << index_string);
				std::istringstream index_stream(index_string);
				std::getline(index_stream, idx_pos, '/');
				std::getline(index_stream, idx_uv, '/');
				std::getline(index_stream, idx_norm, '/');
				//PRINTL("i-v = " << idx_pos << ", i-vn = " << idx_norm << ", i-vu = " << idx_uv);
				uint16_t v = std::atoi(idx_pos.c_str());
				uint16_t vn = std::atoi(idx_norm.c_str());
				uint16_t vu = std::atoi(idx_uv.c_str());

				indexes.push_back({ v,vn,vu });

				//PRINTL("v = " << v << ", vn = " << vn << ", vu = " << vu);


				indicies.push_back(v - 1);




			}

			//triangulation
			for (size_t i = 1; i + 1 < indexes.size(); i++)
			{
				const Index* p[3] = { &indexes[0], &indexes[i], &indexes[i + 1] };

				// http://www.opengl.org/wiki/Calculating_a_Surface_Normal
				glm::vec3 U(pos[(p[1]->v) - 1] - pos[(p[0]->v) - 1]);
				glm::vec3 V(pos[(p[2]->v) - 1] - pos[(p[0]->v) - 1]);
				glm::vec3 faceNormal = glm::normalize(glm::cross(U, V));

				for (size_t j = 0; j < 3; ++j)
				{

					Vertex& temp = verticies[(p[j]->v) - 1];
					//temp.uv = glm::vec2(uvs[(p[j]->vt) - 1]); //if not 0
					int normal_idx = (p[j]->vn) - 1;
					//PRINTL("normal_idx= " << normal_idx);

					auto n = faceNormal;
					if (normal_idx >= 0) {
						//PRINTL("normals size= " << normals.size());
						//PRINTL("normal_idx= " << normal_idx);
						n = glm::normalize(normals[normal_idx]);
					}

					temp.normal = n;
					//temp.normal = (normal_idx >= 0 ? normals[normal_idx] : faceNormal);
				}
			}

			//int f[3];
			//s >> f[0] >> f[1] >> f[2];
			//indicies.push_back(f[0] - 1);
			//indicies.push_back(f[1] - 1);
			//indicies.push_back(f[2] - 1);
		}
		else if (line.starts_with("vn")) //vertex normals
		{
			s >> v.x >> v.y >> v.z;
			normals.push_back(v);
		}
		else if (line.starts_with("vt")) //vertex texture coords (assumed 2d!)
		{
			//s >> uv.x >> uv.y;
			//uvs.push_back(v);
		}
	}

	//size_t len = pos.size();
	//std::vector<Vertex> verts(len);

	//bool hasNormals = len == normals.size();
	//vec3_t n{ 1,1,1 };


	//verticies.resize(len);

	//for (size_t i = 0; i < len; i++)
	//{
	//	verticies[i] = { pos[i], hasNormals ? normals[i] : n };
	//}

	return true;
}
}