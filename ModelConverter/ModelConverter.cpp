#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <chrono>

struct vec2
{
	vec2(const float x, const float y)
		:x(x), y(y)
	{

	}

	float x;
	float y;
};

struct vec3
{
	vec3(const float x, const float y, const float z)
		:x(x), y(y), z(z)
	{

	}

	float x;
	float y;
	float z;
};

template<typename Out>
void split(const std::string &s, const char delim, Out result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string& s, const char delim)
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

using namespace std;

int main(int argc, char * argv[])
{
	if (argc == 1)
	{
		cerr << "Usage ModelConverter <filename>" << endl;
		return -1;
	}

	const auto t1 = std::chrono::high_resolution_clock::now();

	vector<float> positions;
	vector<unsigned> indices;
	vector<float> textures;
	vector<float> normals;
	string filename = argv[1];

	{
		fstream file(filename);

		if (!file.is_open())
		{
			cerr << "File error. Couldn't open file '" << filename << "'";
			return -1;
		}

		vector<vec2> texturesUnordered;
		vector<vec3> normalsUnordered;

		string line;
		auto lineNumber = 0;

		while (getline(file, line))
		{
			lineNumber++;

			if (strncmp(line.c_str(), "v ", 2) == 0)
			{
				float data[3];
				sscanf_s(line.c_str(), "v %f %f %f", &data[0], &data[1], &data[2]);

				for (size_t i = 0; i < 3; i++)
				{
					positions.push_back(data[i]);
				}
			}
			else if (strncmp(line.c_str(), "vt ", 2) == 0)
			{
				float data[2];
				sscanf_s(line.c_str(), "vt %f %f", &data[0], &data[1]);

				texturesUnordered.emplace_back(data[0], data[1]);
			}
			else if (strncmp(line.c_str(), "vn ", 2) == 0)
			{
				float data[3];
				sscanf_s(line.c_str(), "vn %f %f %f", &data[0], &data[1], &data[2]);

				normalsUnordered.emplace_back(data[0], data[1], data[2]);
			}
			else if (strncmp(line.c_str(), "f ", 2) == 0)
			{
				textures.resize(positions.size() / 3 * 2);
				normals.resize(positions.size());
				break;
			}
		}
		do
		{
			if (strncmp(line.c_str(), "f ", 2) != 0)
				continue;

			unsigned vertexIndex[3], uvIndex[3], normalIndex[3];

			const auto matches = sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
			{
				cerr << "Error wih obj file '" << filename << "'. Faulty line " << lineNumber;
				return -1;
			}

			for (auto i = 0; i < 3; i++)
			{
				auto index = vertexIndex[i] - 1;
				const auto texture = texturesUnordered[uvIndex[i] - 1];
				const auto normal = normalsUnordered[normalIndex[i] - 1];

				indices.push_back(index);
				textures[index * 2] = texture.x;
				textures[index * 2 + 1] = 1.f - texture.y;
				normals[index * 3] = normal.x;
				normals[index * 3 + 1] = normal.y;
				normals[index * 3 + 2] = normal.z;
			}

			lineNumber++;
		} while (getline(file, line));

		file.close();
	}

	//Store in new model file
	/*
	 * numVertices : uint
	 * numIndices  : uint
	 * position data...
	 * texture data...
	 * normal data...
	 * indices data...
	 */

	filename = split(filename, '.')[0];
	filename += string(".model");

	ofstream file(filename, ios::out | ios::binary /*| ios::trunc*/);
	const auto numVertices = positions.size() / 3;
	const auto numIndices = indices.size();

	file.write(reinterpret_cast<const char*>(&numVertices), sizeof(numVertices));
	file.write(reinterpret_cast<const char*>(&numIndices), sizeof(numIndices));

	file.write(reinterpret_cast<const char*>(positions.data()), positions.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(textures.data()), textures.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(normals.data()), normals.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(unsigned));

	const auto t2 = std::chrono::high_resolution_clock::now();

	cout << numVertices << " vertices and " << numIndices << " indices written to " << filename << " in " << chrono::duration_cast<chrono::duration<double>>(t2 - t1).count() << " seconds" << endl;
	file.close();

	return 0;
}

