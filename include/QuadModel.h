//For rendering QUAD MESHES
#pragma once
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <omp.h>
#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>      
#include <assimp/scene.h>          
#include <assimp/postprocess.h>     

#include "LoadShader.h"


using std::vector;
using glm::vec2; using glm::vec3; using glm::vec4;
using glm::mat4;
using glm::dot; using glm::cross; using glm::normalize;
const unsigned int workGroupSize = 1024;
void printVec(vec3 v) {
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ") ";
}
void printVec(vec4 v) {
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ") ";
}

class QuadModel {
public:
	std::string path;
	unsigned int numMeshes=0,numFaces=0;

	GLfloat diagonalLength=0,modelScaleFactor=0;
	vec3 center{ 0.0f };

	class Mesh {
	public:
		//Vertex Data
		vector<vec3> vertices, normals;
		vector<vec2> textureCoordinates;
		vector<GLuint> indices;
		vector< std::array<GLuint, 4> > faces;
		vector< std::array<int, 20>>adjacentFaces;
		unsigned int numVertices, numNormals, numFaces, numIndices, numIndexPerFace;

		//Geometry
		vec3 center;
		GLfloat diagonalLength, modelScaleFactor;

		//GL buffers
		GLuint VAO, positionVBO, normalVBO, textureVBO, EBO;

		Mesh() {
			
		}
		void render(GLuint shader) {
			if (!this->isSet) this->setup();
			glBindVertexArray(VAO);
			glUseProgram(shader);
			//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
			//For renering quads
			glDrawElements(GL_LINES_ADJACENCY, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0); 
			//GL_LINES_ADJACENCY to send 4 vertices each primitive.
			//when using currently bound index buffer, the last parameter 0 is the offset of where we are rendering.
			return;
		}
		void boundingBox() {
			//simple implemetation calculating mesh boundary box size
			float maxX = vertices[0].x, maxY = vertices[0].y, maxZ = vertices[0].z;
			float minX = vertices[0].x, minY = vertices[0].y, minZ = vertices[0].z;
#pragma omp parallel for
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
		//Finds adjacent vertices for each vertex
		void findAdjacentFaces() {
			auto start = std::chrono::high_resolution_clock::now();
			this->adjacentFaces.resize(vertices.size()); //adjacentFaces<std::array<int, 20>
			for (size_t i = 0; i < numVertices; i++) { adjacentFaces[i].fill(-1); }
#pragma omp parallel for
			for (size_t i = 0; i < numFaces; i++) { //loop each face
				int vertexIds[3] = { this->indices[3 * i],this->indices[3 * i + 1] ,this->indices[3 * i + 2] };
				for (size_t j = 0; j < 3; j++) { //loop each vertex on the face
					for (size_t k = 0; k < 10; k++) {
						if (this->adjacentFaces[vertexIds[j]][2 * k] < 0) {
							this->adjacentFaces[vertexIds[j]][2 * k] = vertexIds[(j + 1) % 3];
							this->adjacentFaces[vertexIds[j]][2 * k + 1] = vertexIds[(j + 2) % 3];
							break;
						}
					}
				}
			}
			//Computing this on the cpu is rather slow..?
			/*
			glGenBuffers(1, &adjacentFacesBuffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, adjacentFacesBuffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, adjacentFaces.size() * sizeof(int) * 20, adjacentFaces.data(), GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, adjacentFacesBuffer);
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

		void setup() {
			glGenVertexArrays(1, &VAO); //vertex array object
			glGenBuffers(1, &positionVBO); //vertex buffer object
			glGenBuffers(1, &normalVBO); //vertex buffer object
			glGenBuffers(1, &textureVBO); //vertex buffer object

			glGenBuffers(1, &EBO);
			//VAO  
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, positionVBO);//Bound VBOs are attachted to the activated VAO. (Same for EBOs)
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

			//EBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
			// glVertexAttribPointer(index, size, type, normalized(bool), stride(byte offset between), pointer(offset))
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


			this->isSet = true;
			return;
		};
		private:
	bool isSet = false;
	};//End of Mesh Class.

	class Dual {

	};//End of Dual Class
	
	vector<Mesh> meshes;
	//initializer list
	QuadModel(std::string _path) : path(_path) {
		this->loadAssimp();
		this->boundingBox();
	}
	//draw function
	void render(GLuint shader) {
		for (unsigned int i = 0; i < this->numMeshes; i++) {
			this->meshes[i].render(shader);
		}
	}
	bool loadAssimp() {
		Assimp::Importer importer;
		std::cout << "Loading file : " << this->path << ".\n";
		//Check post processing flags
		const aiScene* scene = importer.ReadFile(path, aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals| aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes |aiProcess_GenUVCoords);
		// |aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_CalcTangentSpace);
		if (!scene) {
			fprintf(stderr, importer.GetErrorString());
			return false;
		}
		omp_set_num_threads(this->getOMPMaxThreads());

		this->numMeshes = scene->mNumMeshes;
		unsigned int totalFaces = 0;
		std::cout << "Number of meshes : " << this->numMeshes << ".\n";
		for (size_t meshId = 0; meshId < scene->mNumMeshes; meshId++) {
			
			aiMesh* mesh = scene->mMeshes[meshId];
			
			this->meshes.push_back(Mesh());

			// Fill vertices positions
			//std::cout << "Number of vertices :" << mesh->mNumVertices << "\n";
#pragma omp parallel for
			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				aiVector3D pos = mesh->mVertices[i];
				this->meshes[meshId].vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
			}

			// Fill vertices texture coordinates
			if (mesh->HasTextureCoords(0)) {
#pragma omp parallel for
				for (size_t i = 0; i < mesh->mNumVertices; i++) {
					aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
					this->meshes[meshId].textureCoordinates.push_back(glm::vec2(UVW.x, UVW.y));
				}
			}

			// Fill vertices normals
			if (mesh->HasNormals()) {
#pragma omp parallel for
				for (size_t i = 0; i < mesh->mNumVertices; i++) {
					// std::cout<<"Number of Vertices : "<<mesh->mNumVertices<<"\n";
					aiVector3D n = mesh->mNormals[i];
					this->meshes[meshId].normals.push_back(glm::normalize(glm::vec3(n.x, n.y, n.z)));
				}
			}
			else {
				//std::cout << "Model has no normals.\n";
				//mesh->
#pragma omp parallel for
				for (size_t i = 0; i < mesh->mNumVertices; i++) {
					// std::cout<<"Number of Vertices : "<<mesh->mNumVertices<<"\n";
					aiVector3D n = mesh->mNormals[i];
					this->meshes[meshId].normals.push_back(glm::normalize(glm::vec3(n.x, n.y, n.z)));
				}
			}
			totalFaces += mesh->mNumFaces;
			// Fill face indices
#pragma omp parallel for
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				if (mesh->mFaces[i].mNumIndices == 4){
					//std::cout << "Loading Quad Mesh\n";
					std::array<unsigned int, 4> face{ 0 };
					for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
						face[j] = mesh->mFaces[i].mIndices[j];
					}
					//For quad elements, the ordering is important as we are rendering with traingle strips.
					//We will reorder the indices in counter clockwise order, using normals to define orientation, from the centroid.
				  
					vec3 faceNormal = (this->meshes[meshId].normals[face[0]] + this->meshes[meshId].normals[face[1]] 
						+ this->meshes[meshId].normals[face[2]] + this->meshes[meshId].normals[face[3]]) / 4.0f;
					vector<vec3> edgesFromp0; 
					edgesFromp0.push_back(this->meshes[meshId].vertices[face[1]] - this->meshes[meshId].vertices[face[0]]);
					edgesFromp0.push_back(this->meshes[meshId].vertices[face[2]] - this->meshes[meshId].vertices[face[0]]);
					edgesFromp0.push_back(this->meshes[meshId].vertices[face[3]] - this->meshes[meshId].vertices[face[0]]);
					//order between 12 13 23
					/*
					vector<vec3> crossProducts;
					crossProducts.push_back(cross(edgesFromp0[0], edgesFromp0[1])); //12
					crossProducts.push_back(cross(edgesFromp0[0], edgesFromp0[2])); //13
					crossProducts.push_back(cross(edgesFromp0[1], edgesFromp0[2])); //23
					*/
					
					//swap indices that aren't counter clockwise
					vec3 crossBetween = cross(edgesFromp0[0], edgesFromp0[1]);
					if(dot(crossBetween,faceNormal)<0){ //12 are clockwise
						std::swap(face[1], face[2]);
					}
					crossBetween = cross(edgesFromp0[0], edgesFromp0[2]);
					if (dot(crossBetween, faceNormal) < 0) { //13 are clockwise
						std::swap(face[1], face[3]);
					}
					crossBetween = cross(edgesFromp0[1], edgesFromp0[2]);
					if (dot(crossBetween, faceNormal) < 0) { //23 are clockwise
						std::swap(face[2], face[3]);
					}
					for (size_t j = 0; j < 4; j++) {
						this->meshes[meshId].indices.push_back(face[j]);
					}
					 
					this->meshes[meshId].faces.push_back(face);
					}
				else {
					//if tri mesh
					std::array<unsigned int, 4> fac{ 0 };
					for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
						this->meshes[meshId].indices.push_back(mesh->mFaces[i].mIndices[j]);
						fac[j] = mesh->mFaces[i].mIndices[j];
					}
					this->meshes[meshId].faces.push_back(fac);
				}
			}//for every face
			std::cout << "Number of vertices : " << this->meshes[meshId].vertices.size() << "\n";
			std::cout << "Number of normals : " << this->meshes[meshId].normals.size() << "\n";
			std::cout << "Number of indices : " << this->meshes[meshId].indices.size() << "\n";
			std::cout << "Number of faces : " << this->meshes[meshId].faces.size() << "\n";

			this->meshes[meshId].numVertices = this->meshes[meshId].vertices.size();
			this->meshes[meshId].numNormals = this->meshes[meshId].normals.size();
			this->meshes[meshId].numFaces = this->meshes[meshId].faces.size();
			this->meshes[meshId].numIndices = this->meshes[meshId].indices.size();
			this->meshes[meshId].numIndexPerFace = mesh->mFaces[0].mNumIndices;
			this->meshes[meshId].boundingBox();
			// The "scene" pointer will be deleted automatically by "importer"
		}
		this->numFaces = totalFaces;
		return true;
	}
	void boundingBox() {
		//simple implemetation calculating mesh boundary box size
		float maxX = this->meshes[0].vertices[0].x, maxY = this->meshes[0].vertices[0].y, maxZ = this->meshes[0].vertices[0].z;
		float minX = this->meshes[0].vertices[0].x, minY = this->meshes[0].vertices[0].y, minZ = this->meshes[0].vertices[0].z;
		for (int meshId = 0; meshId < this->numMeshes; meshId++) {
#pragma omp parallel for
			for (size_t i = 0; i < this->meshes[meshId].vertices.size(); i++) {
				(this->meshes[meshId].vertices[i].x > maxX) ? maxX = this->meshes[meshId].vertices[i].x : 0;
				(this->meshes[meshId].vertices[i].y > maxY) ? maxY = this->meshes[meshId].vertices[i].y : 0;
				(this->meshes[meshId].vertices[i].z > maxZ) ? maxZ = this->meshes[meshId].vertices[i].z : 0;
				(this->meshes[meshId].vertices[i].x < minX) ? minX = this->meshes[meshId].vertices[i].x : 0;
				(this->meshes[meshId].vertices[i].y < minY) ? minY = this->meshes[meshId].vertices[i].y : 0;
				(this->meshes[meshId].vertices[i].z < minZ) ? minZ = this->meshes[meshId].vertices[i].z : 0;
			}
		}
		//center of model
		this->center = glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f);
		this->diagonalLength = glm::length(glm::vec3(maxX - minX, maxY - minY, maxZ - minZ));
		this->modelScaleFactor = 1.0f / diagonalLength;
	}
	
	//destructor
	//Smart pointers should NOT be in another smart pointer.
	//Nested ownership often causes leaks
	static int getOMPMaxThreads() {
		//only runs once
		static int maxThreads = omp_get_max_threads();
		return maxThreads;
	}
};
//end of Model class

typedef std::unique_ptr<QuadModel> modelPtr;

