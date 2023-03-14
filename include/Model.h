#pragma once
#include<memory>
#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>      
#include <assimp/scene.h>          
#include <assimp/postprocess.h>     

#include "LoadShader.h"


using std::vector;
using glm::vec2; using glm::vec3; using glm::vec4;
using glm::mat4;
const unsigned int workGroupSize = 1024;
void printVec(vec3 v) {
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ") ";
}
void printVec(vec4 v) {
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ") ";
}

class Model {
public:
	std::string path;
	class Mesh {
	public:
		vector<vec3> vertices, normals, indices;
		vector<vec2> textureCoordinates;
		vector<GLuint> indices;
		vec3 center;

		GLfloat diagonalLength, modelScaleFactor;
		
		GLfloat modelScaleFactor;

		Mesh() {

		}
		bool render(GLuint shader) {

		}
		void boundingBox() {
			//simple implemetation calculating mesh boundary box size
			float maxX = vertices[0].x, maxY = vertices[0].y, maxZ = vertices[0].z;
			float minX = vertices[0].x, minY = vertices[0].y, minZ = vertices[0].z;

			for (int i = 1; i < vertices.size(); i++) {
				(vertices[i].x > maxX) ? maxX = vertices[i].x : 0;
				(vertices[i].y > maxY) ? maxY = vertices[i].y : 0;
				(vertices[i].z > maxZ) ? maxZ = vertices[i].z : 0;
				(vertices[i].x < minX) ? minX = vertices[i].x : 0;
				(vertices[i].y < minY) ? minY = vertices[i].y : 0;
				(vertices[i].z < minZ) ? minZ = vertices[i].z : 0;
			}
			//center of model
			this->center = glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f);
			this->diagonalLength = glm::length(glm::vec3(maxX - minX, maxY - minY, maxZ - minZ));
			this->modelScaleFactor = 1.0f / diagonalLength;
		}
		//Very rough
		float getMinDistance() {
			float minDist = glm::length(vertices[0] - vertices[1]);
			for (int i = 2; i < vertices.size(); i++) {
				float dis = glm::length(vertices[i] - vertices[0]);
				if (dis < minDist && dis>0.0f) minDist = dis;
			}
			return minDist;

		}

		void rebindSSBOs() {

			//Rebind SSBOs
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, PDBuffer);
			return;
		}
	};//End of Mesh Class.

	vector<Mesh> meshes;
	Model(std::string path) {
		
	}
	void setup() {
		
		return;
	}
	//draw function
	bool render(GLuint shader) {

	}
	bool loadAssimp() {
		Assimp::Importer importer;
		std::cout << "Loading file : " << this->path << ".\n";
		//Check post processing flags
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes /*| aiProcess_CalcTangentSpace | aiProcess_GenUVCoords*/); //aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
		if (!scene) {
			fprintf(stderr, importer.GetErrorString());
			return false;
		}
		std::cout << "Number of meshes : " << scene->mNumMeshes << ".\n";
		for (unsigned int meshId = 0; meshId < scene->mNumMeshes; meshId++) {
			
			aiMesh* mesh = scene->mMeshes[meshId];
			
			this->meshes.push_back(Mesh());

			// Fill vertices positions
			//std::cout << "Number of vertices :" << mesh->mNumVertices << "\n";
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				aiVector3D pos = mesh->mVertices[i];
				this->meshes[meshId].vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
			}

			// Fill vertices texture coordinates
			if (mesh->HasTextureCoords(0)) {
				for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
					aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
					this->meshes[meshId].textureCoordinates.push_back(glm::vec2(UVW.x, UVW.y));
				}
			}

			// Fill vertices normals
			if (mesh->HasNormals()) {
				for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
					// std::cout<<"Number of Vertices : "<<mesh->mNumVertices<<"\n";
					aiVector3D n = mesh->mNormals[i];
					this->meshes[meshId].normals.push_back(glm::normalize(glm::vec3(n.x, n.y, n.z)));
				}
			}
			else {
				//std::cout << "Model has no normals.\n";
				//mesh->
				for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
					// std::cout<<"Number of Vertices : "<<mesh->mNumVertices<<"\n";
					aiVector3D n = mesh->mNormals[i];
					this->meshes[meshId].normals.push_back(glm::normalize(glm::vec3(n.x, n.y, n.z)));
				}
			}
			// Fill face indices
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				std::array<unsigned int, 4> fac{ 0 };
				for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
					this->meshes[meshId].indices.push_back(mesh->mFaces[i].mIndices[j]);
					fac[j] = mesh->mFaces[i].mIndices[j];
				}
				this->meshes[meshId].faces.push_back(fac);
			}

			std::cout << "Number of vertices : " << this->vertices.size() << "\n";
			std::cout << "Number of normals : " << this->normals.size() << "\n";
			//std::cout << "Number of indices : " << this->indices.size() << "\n";
			std::cout << "Number of faces : " << this->faces.size() << "\n";

			this->numVertices = this->vertices.size();
			this->numNormals = this->normals.size();
			this->numFaces = this->faces.size();
			this->numIndices = this->indices.size();

			// The "scene" pointer will be deleted automatically by "importer"
		}
		return true;
	}
	
	//Finds adjacent vertices for each vertex
	void findAdjacentFaces() {
		auto start = std::chrono::high_resolution_clock::now();
		this->adjacentFaces.resize(vertices.size()); //adjacentFaces<std::array<int, 20>
		for (int i = 0; i < numVertices; i++) { adjacentFaces[i].fill(-1); }

		for (int i = 0; i < numIndices / 3; i++) { //loop each face
			int vertexIds[3] = { this->indices[3 * i],this->indices[3 * i + 1] ,this->indices[3 * i + 2] };
			for (int j = 0; j < 3; j++) { //loop each vertex on the face
				for (int k = 0; k < 10; k++) {
					if (this->adjacentFaces[vertexIds[j]][2 * k] < 0) {
						this->adjacentFaces[vertexIds[j]][2 * k] = vertexIds[(j + 1) % 3];
						this->adjacentFaces[vertexIds[j]][2 * k + 1] = vertexIds[(j + 2) % 3];
						break;
					}
				}
			}
		}
		//Computing this on the cpu is rather slow so I made a shader for it.
		glGenBuffers(1, &adjacentFacesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, adjacentFacesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, adjacentFaces.size() * sizeof(int) * 20, adjacentFaces.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, adjacentFacesBuffer);
		/*

		GLuint adjacentFacesCompute = loadComputeShader(".\\shaders\\adjacentFaces.compute");
		glUseProgram(adjacentFacesCompute);
		glUniform1ui(glGetUniformLocation(adjacentFacesCompute, "indicesSize"), this->numIndices);
		glDispatchCompute(glm::ceil((GLfloat(this->numIndices) / 3.0f) / float(workGroupSize)), 1, 1); //per face
		glMemoryBarrier(GL_ALL_BARRIER_BITS);


		*/
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "Adjacent faces calculated. Took : " << elapsed_seconds.count() << " seconds. \n";

		//glGetNamedBufferSubData(adjacentFacesBuffer, 0, adjacentFaces.size() * sizeof(GLfloat), adjacentFaces.data());
		std::cout << "First vertex's adjacent vertices : ";
		for (int j = 0; j < 10; j++) {
			std::cout << this->adjacentFaces[0][2 * j] << ", " << this->adjacentFaces[0][2 * j + 1] << ". ";
		}
		std::cout << "\n";
	}

	//destructor
	//Smart pointers should NOT be in another smart pointer.
	//Nested ownership often causes leaks

};
//end of Model class

typedef std::unique_ptr<Model> modelPtr;

