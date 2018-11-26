#define _CRT_SECURE_NO_WARNINGS
// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

//include transformation functions
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "CGobject.h"
#include "Cloth.h"
#include "Physics.h"
#include "..\Dependencies\OBJ_Loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"

using namespace std;
using namespace CGCommon;
using namespace Physics;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLfloat rotate_angle = 0.0f;

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1100;

// Shaders
GLuint programID;
GLuint lightingID;
GLuint waterParticleID;
//GLuint flagID;

// Shader attribute locations
GLuint loc1;
GLuint loc2;
GLuint loc3;
//GLuint loc4;
//GLuint loc5;
//GLuint loc6;

// Uniform locations
int model_mat_location;
int view_mat_location;
int proj_mat_location;

//int model_mat_location_flag;
//int view_mat_location_flag;
//int proj_mat_location_flag;

// Buffers
GLuint VBO;
GLuint flagVBO;
GLuint IBO;
GLuint flagIBO;
GLuint groundVAO;
GLuint mountainVAO;
GLuint footballwVAO;
GLuint footballbVAO;
GLuint lightVAO;
GLuint treeVAO;
GLuint flagpostVAO;
GLuint waterParticleVAO;
GLuint flagVAO;

// ---- water particles vbo --- //
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;


int n_vbovertices = 0;
int n_ibovertices = 0;


bool firstMouse = true;
float myyaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float mypitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -6.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 3.0f, 0.0f);

//fountain position based off camer
glm::vec3 cameraPosFountain = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFrontFountain = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUpFountain = glm::vec3(0.0f, 1.0f, 0.0f);

//glm::mat4 projectionfountain = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
//glm::mat4 viewfountain = glm::lookAt(cameraPosFountain, cameraPosFountain + cameraFrontFountain, cameraUpFountain);
// objects
CGObject ground, mountain;
CGObject footballw, footballb, tree, flagpost;
CGObject footballw2, footballb2, footballw3, footballb3;
Cloth flag = Cloth(2.5, 1.5, 10, 10);

CGObject *sceneObjects[] = { &ground, &mountain, &tree, &footballw, &footballb, &footballw2, &footballb2, &footballw3, &footballb3, &flagpost };  // include objects that are subject to Physics
																																				  // timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//lighting position
glm::vec3 lightPos(1.0f, 1.0f, 3.0f);

bool rotateCubes = false;

//--------WATER PARTICLE SYSTEM-------
// NOTE: The particle system was created by following a tutorial from opengl-tutorials
// It can be found here: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
// CPU representation of a particle

static GLfloat* g_particule_position_size_data;
static GLubyte* g_particule_color_data;

static GLfloat* g_particule_position_size_data2;
static GLubyte* g_particule_color_data2;

GLuint TextureID;
GLuint CameraRight_worldspace_ID, CameraUp_worldspace_ID, ViewProjMatrixID,
projection_waterparticle, view_waterparticle, model_waterparticle;

unsigned int texture;

struct WaterParticle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const WaterParticle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 10000;
WaterParticle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

const int MaxParticles2 = 10000;
WaterParticle ParticlesContainer2[MaxParticles2];
int LastUsedParticle2 = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle() {

	for (int i = LastUsedParticle; i < MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

int FindUnusedParticle2() {

	for (int i = LastUsedParticle2; i < MaxParticles2; i++) {
		if (ParticlesContainer2[i].life < 0) {
			LastUsedParticle2 = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle2; i++) {
		if (ParticlesContainer2[i].life < 0) {
			LastUsedParticle2 = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void SortParticles2() {
	std::sort(&ParticlesContainer2[0], &ParticlesContainer2[MaxParticles2]);
}

void initialiseWaterParticles() {
	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i < MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}
}

void initialiseWaterParticles2() {
	g_particule_position_size_data2 = new GLfloat[MaxParticles2 * 4];
	g_particule_color_data2 = new GLubyte[MaxParticles2 * 4];

	for (int i = 0; i < MaxParticles2; i++) {
		ParticlesContainer2[i].life = -1.0f;
		ParticlesContainer2[i].cameradistance = -1.0f;
	}
}


//GLuint Texture = loadDDS("particle.DDS");

void initialiseWaterTexture() {
	// load and create a texture 
	// -------------------------

	// texture 1
	// ---------
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("../CGcommon/Particles/whitelight.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

}

//---------------------------------------------------------------------------------------------------------//

void bindVertexAttribute(int location, int locationSize, int startVBO, int offsetVBO)
{
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(location, locationSize, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void*)(startVBO * 8 * sizeof(float) + BUFFER_OFFSET(offsetVBO * sizeof(GLfloat))));
}

void linkCurrentBuffertoShader(CGCommon::CGObject *cg_object)
{
	glBindVertexArray(cg_object->VAO);

	bindVertexAttribute(loc1, 3, cg_object->startVBO, 0);
	bindVertexAttribute(loc2, 3, cg_object->startVBO, 3);
	bindVertexAttribute(loc3, 3, cg_object->startVBO, 6);// shench bindVertexAttribute(loc3, 2, cg_object.startVBO, 6);

	//IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
}

void linkFlagBuffertoShader()
{
	glBindVertexArray(flagVAO);
	
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO); 
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO); 
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(flag.NumParticlesWidth() * flag.NumParticlesHeight() * 3 * sizeof(float)));

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO); 
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void*)(flag.NumParticlesWidth() * flag.NumParticlesHeight() * 6 * sizeof(float)));

	//IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flagIBO);
}

void addToFlagBuffer()  //MeshType meshType, int startVBO, int n_vertices, float *vertices)
{
	int numFlagParticles = flag.NumParticlesWidth() * flag.NumParticlesHeight();
	int numWidth = flag.NumParticlesWidth();
	int numHeight = flag.NumParticlesHeight();

	// flag
	glm::vec3* flagPositions = new glm::vec3[flag.NumParticlesHeight() * flag.NumParticlesWidth()];
	glm::vec3* flagNormals = new  glm::vec3[flag.NumParticlesHeight() * flag.NumParticlesWidth()];
	glm::vec3* flagColors = new glm::vec3[flag.NumParticlesHeight() * flag.NumParticlesWidth()];

	int third = numWidth / 3;

	for (int i = 0; i < numWidth; i++)
	{
		for (int j = 0; j < numHeight; j++)
		{
			flagPositions[j * numWidth + i] = flag.GetParticle(i, j)->getPos();
			flagNormals[j * numWidth + i] = flag.GetParticle(i, j)->getNormal(); //glm::vec3(0.0f, 0.0f, 1.0f);

			if (i < third)
			{
				flagColors[j * numWidth + i] = glm::vec3(0.0f, 1.0f, 0.0f);
			}
			else if (i < 2 * third)
			{
				flagColors[j * numWidth + i] = glm::vec3(1.0f, 1.0f, 1.0f);
			}
			else
			{
				flagColors[j * numWidth + i] = glm::vec3(1.0f, 1.0f, 0.0f);
			}
		}
	}
	
	glGenBuffers(1, &flagVBO);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO);
	glBufferData(GL_ARRAY_BUFFER, numFlagParticles * 9 * sizeof(float), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, numFlagParticles * 3 * sizeof(GLfloat), flagPositions);
	glBufferSubData(GL_ARRAY_BUFFER, numFlagParticles * 3 * sizeof(GLfloat), numFlagParticles * 3 * sizeof(GLfloat), flagNormals);
	glBufferSubData(GL_ARRAY_BUFFER, numFlagParticles * 6 * sizeof(GLfloat), numFlagParticles * 3 * sizeof(GLfloat), flagColors);

	delete flagPositions;
	delete flagNormals;
	delete flagColors;

	flagPositions = nullptr;
	flagNormals = nullptr;
	flagColors = nullptr;
}

void addToObjectBuffer(CGCommon::CGObject *cg_object, GLuint VAO)  //MeshType meshType, int startVBO, int n_vertices, float *vertices)
{
	glBufferSubData(GL_ARRAY_BUFFER, cg_object->startVBO * 8 * sizeof(GLfloat), cg_object->Mesh.Vertices.size() * 8 * sizeof(GLfloat), &cg_object->Mesh.Vertices[0].Position.X);

	linkCurrentBuffertoShader(cg_object);
}

void updateUniformVariables(glm::mat4 model)
{
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, &model[0][0]);

	//mat4 normalsTransform = transpose(inverse(model));
	//glUniformMatrix4fv(normals_location, 1, GL_FALSE, normalsTransform.m);
	//glUniformMatrix4fv(worldNormal, 1, GL_FALSE, normalsTransform.m);
}

void updateUniformVariables(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
{
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
	updateUniformVariables(model);
}
//
//void updateUniformVariablesFlag(glm::mat4 model, glm::mat4 view, glm::mat4 persp_proj)
//{
//	glUniformMatrix4fv(proj_mat_location_flag, 1, GL_FALSE, &persp_proj[0][0]);
//	glUniformMatrix4fv(view_mat_location_flag, 1, GL_FALSE, &view[0][0]);
//	glUniformMatrix4fv(model_mat_location_flag, 1, GL_FALSE, &model[0][0]);
//}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("impossible to open %s. are you in the right directory ? don't forget to read the faq !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);
	printf("Finished linking\n");


	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void createShaders()
{
	// Create and compile our shaders
	programID = LoadShaders("../CGcommon/shaders/shader.vs", "../CGcommon/shaders/shader.fs");
	lightingID = LoadShaders("../CGcommon/shaders/lighting.vs", "../CGcommon/shaders/lighting.fs");
	waterParticleID = LoadShaders("../CGcommon/shaders/Particle.vertexshader", "../CGcommon/shaders/Particle.fragmentshader");
//	flagID = LoadShaders("../CGcommon/shaders/flag.vs", "../CGcommon/shaders/flag.fs");
}

void setupUniformVariables()
{
	//Declare your uniform variables that will be used in your shader
	model_mat_location = glGetUniformLocation(programID, "model");
	view_mat_location = glGetUniformLocation(programID, "view");
	proj_mat_location = glGetUniformLocation(programID, "projection");

	/*model_mat_location_flag = glGetUniformLocation(flagID, "modelFlag");
	view_mat_location_flag = glGetUniformLocation(flagID, "viewFlag");
	proj_mat_location_flag = glGetUniformLocation(flagID, "projectionFlag");*/
}

objl::Mesh groundMesh()
{
	objl::Vertex point1, point2, point3, point4, point5, point6, point7, point8 = objl::Vertex();

	float size = 20.0f;

	point1.Position = objl::Vector3(-size, -0.5f, -size);
	point1.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point2.Position = objl::Vector3(-size, 0.5f, -size);
	point2.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point3.Position = objl::Vector3(-size, -0.5f, size);
	point3.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point4.Position = objl::Vector3(-size, 0.5f, size);
	point4.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point5.Position = objl::Vector3(size, -0.5f, size);
	point5.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point6.Position = objl::Vector3(size, 0.5f, size);
	point6.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point7.Position = objl::Vector3(size, -0.5f, -size);
	point7.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point8.Position = objl::Vector3(size, 0.5f, -size);
	point8.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);

	point1.TextureCoordinate = point2.TextureCoordinate = point3.TextureCoordinate = point4.TextureCoordinate =
		point5.TextureCoordinate = point6.TextureCoordinate = point7.TextureCoordinate = point8.TextureCoordinate = objl::Vector2(0.013400, 0.997700);  //green
	std::vector<objl::Vertex> vertices = std::vector<objl::Vertex>{ point1, point2, point3, point4, point5, point6, point7, point8 };

	std::vector<unsigned int> indices = std::vector<unsigned int>{ 0, 1, 2,
																	1, 2, 3,
																	2, 3, 4,
																	3, 4, 5,
																	4, 5, 6,
																	5, 6, 7,
																	6, 7, 0,
																	7, 0, 1,
																	0, 2, 6,
																	2, 4, 6,
																	1, 3, 7,
																	3, 5, 7
	};

	objl::Mesh mesh = objl::Mesh(vertices, indices);

	return mesh;
}

bool findVertex(std::map<unsigned int, objl::Vector3> middlePoints, objl::Vector3 position, unsigned int& outIndex)
{
	float threshold = 0.02; // arbitary threashold 							

	for (auto const& x : middlePoints)
	{
		if (abs(x.second.X - position.X) < threshold && abs(x.second.Y - position.Y) < threshold && abs(x.second.Z - position.Z) < threshold)
		{
			outIndex = x.first;
			return true;
		}
	}

	return false;
}

objl::Mesh mountainMesh()
{
	// start with 5 points 
	objl::Vertex point1, point2, point3, point4, point5 = objl::Vertex();

	float size = 3.0f;

	// Ignore normals for now
	point1.Position = objl::Vector3(-size, 0.0f, -size);
	point1.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point2.Position = objl::Vector3(size, 0.0f, -size);
	point2.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point3.Position = objl::Vector3(size, 0.0f, size);
	point3.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point4.Position = objl::Vector3(-size, 0.0f, size);
	point4.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point5.Position = objl::Vector3(0.0f, 2 * size, 0.0f);
	point5.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);

	std::vector<objl::Vertex> vertices = std::vector<objl::Vertex>{ point1, point2, point3, point4, point5 };

	// Only drawing the side triangles - not drawing bottom 
	std::vector<unsigned int> indices = std::vector<unsigned int>{ 0, 1, 4,
		1, 2, 4,
		2, 3, 4,
		3, 0, 4
	};

	int nIterations = 1; // number of interations 
	int n = 0;

	while (n < nIterations)
	{
		// Re-write indices in each iteration
		std::vector<unsigned int> tempIndices = std::vector<unsigned int>();
		std::map<unsigned int, objl::Vector3> middlePoints = std::map<unsigned int, objl::Vector3>();

		// For each triangle
		for (unsigned int i = 0; i < indices.size(); i = i + 3)
		{
			unsigned int currentVerticesSize = vertices.size();

			// get 3 points 
			objl::Vector3 position1 = (vertices[indices[i]]).Position;
			objl::Vector3 position2 = (vertices[indices[i + 1]]).Position;
			objl::Vector3 position3 = (vertices[indices[i + 2]]).Position;

			float scale = (position1.X - position2.X) / 8;

			// Divide into 4 triangles
			objl::Vertex vertex1, vertex2, vertex3 = objl::Vertex();
		
			vertex1.Position = objl::Vector3((position1.X + position2.X) / 2, (position1.Y + position2.Y) / 2, (position1.Z + position2.Z) / 2);
			vertex1.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);

			// Find if this point exists already
			unsigned int index1;
			if (!findVertex(middlePoints, vertex1.Position, index1))
			{
				index1 = currentVerticesSize;
				// need to add the vertex as we did not find it				
				middlePoints.insert(std::pair<unsigned int, objl::Vector3>(index1, vertex1.Position));
				vertex1.Position.X += (rand() % 100 / 100.0f);
				vertex1.Position.Z += (rand() % 100 / 100.0f);
				if (vertex1.Position.Y != 0)
				{
					vertex1.Position.Y += (rand() % 100 / 100.0f);
				}
				vertices.push_back(vertex1);
			}

			vertex2.Position = objl::Vector3((position2.X + position3.X) / 2, (position2.Y + position3.Y) / 2, (position2.Z + position3.Z) / 2);
			vertex2.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);

			unsigned int index2;
			if (!findVertex(middlePoints, vertex2.Position, index2))
			{
				index2 = currentVerticesSize + 1;
				// need to add the vertex as we did not find it
				middlePoints.insert(std::pair<unsigned int, objl::Vector3>(index2, vertex2.Position));
				vertex2.Position.X += (rand() % 100 / 100.0f);
				vertex2.Position.Z += (rand() % 100 / 100.0f);
				if (vertex2.Position.Y != 0)
				{
					vertex2.Position.Y += (rand() % 100 / 100.0f);
				}
				vertices.push_back(vertex2);
			}

			vertex3.Position = objl::Vector3((position1.X + position3.X) / 2, (position1.Y + position3.Y) / 2, (position1.Z + position3.Z) / 2);
			vertex3.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);

			unsigned int index3;
			if (!findVertex(middlePoints, vertex3.Position, index3))
			{
				// need to add the vertex as we did not find it			
				index3 = currentVerticesSize + 2;
				middlePoints.insert(std::pair<unsigned int, objl::Vector3>(index3, vertex3.Position));
				vertex3.Position.X += (rand() % 100 / 100.0f);
				vertex3.Position.Z += (rand() % 100 / 100.0f);
				if (vertex3.Position.Y != 0)
				{
					vertex3.Position.Y += (rand() % 100 / 100.0f);
				}
				vertices.push_back(vertex3);
			}

			std::vector<unsigned int> indicesToAdd = std::vector<unsigned int>{ indices[i],  index1,  index3,
				index3, index1, index2,
				index1, indices[i + 1], index2,
				index3, index2, indices[i + 2] };

			tempIndices.insert(std::end(tempIndices), std::begin(indicesToAdd), std::end(indicesToAdd));
		}

		// Set normals

		// Replace indices
		indices = tempIndices;

		n++;
	}

	objl::Mesh mesh = objl::Mesh(vertices, indices);

	return mesh;
}

objl::Mesh PlaneMesh()
{
	float planeSize = 0.8f;

	// Create plane or ground mesh	
	objl::Vertex point1, point2, point3, point4 = objl::Vertex();
	point1.Position = objl::Vector3(-planeSize, 0.0f, -planeSize);
	point2.Position = objl::Vector3(planeSize, 0.0f, -planeSize);
	point3.Position = objl::Vector3(planeSize, 0.0f, planeSize);
	point4.Position = objl::Vector3(-planeSize, 0.0f, planeSize);

	point1.Normal = point2.Normal = point3.Normal = point4.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point1.TextureCoordinate = point2.TextureCoordinate = point3.TextureCoordinate = point4.TextureCoordinate = objl::Vector2(0.0, 0.0); //objl::Vector2(0.013400, 0.997700);  //green

	std::vector<objl::Vertex> vertices = std::vector<objl::Vertex>{ point1, point2, point3, point4 };
	std::vector<unsigned int> indices = std::vector<unsigned int>{ 0, 1, 2,
		0, 2, 3 };

	objl::Mesh mesh = objl::Mesh(vertices, indices);
	mesh.MeshName = "plane";
	return mesh;
}

std::vector<objl::Mesh> loadMeshes(const char* objFileLocation)
{
	objl::Loader obj_loader;

	bool result = obj_loader.LoadFile(objFileLocation);
	if (result && obj_loader.LoadedMeshes.size() > 0)
	{
		return obj_loader.LoadedMeshes;
	}
	else
		throw new exception("Could not load mesh");
}

CGCommon::CGObject loadObjObject(objl::Mesh mesh, bool addToBuffers, GLuint VAO, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	CGCommon::CGObject object = CGCommon::CGObject();
	object.Mesh = mesh;
	object.VAO = VAO;
	object.subjectToGravity = subjectToGravity;
	object.initialTranslateVector = initTransformVector;
	object.position = initTransformVector;
	object.initialScaleVector = initScaleVector;
	object.color = color;
	object.coef = coef;
	object.Parent = parent;
	object.startVBO = n_vbovertices;
	object.startIBO = n_ibovertices;

	if (addToBuffers)
	{
		n_vbovertices += object.Mesh.Vertices.size();
		n_ibovertices += object.Mesh.Indices.size();
	}

	return object;
}

void createObjects()
{
	// Shader Attribute locations
	loc1 = glGetAttribLocation(programID, "position");
	loc2 = glGetAttribLocation(programID, "normal");
	loc3 = glGetAttribLocation(programID, "texture");

	//loc4 = glGetAttribLocation(flagID, "flagPosition");
	//loc5 = glGetAttribLocation(flagID, "flagNormal");
	//loc6 = glGetAttribLocation(flagID, "flagColor");
	
	// Vertex array objects
	glGenVertexArrays(1, &footballwVAO);
	glGenVertexArrays(1, &footballbVAO);
	glGenVertexArrays(1, &groundVAO);
	glGenVertexArrays(1, &mountainVAO);
	glGenVertexArrays(1, &treeVAO);
	glGenVertexArrays(1, &flagpostVAO);
	glGenVertexArrays(1, &flagVAO);

	// ADD GROUND
	ground = loadObjObject(groundMesh(), true, groundVAO, false, vec3(0.0f, -0.5f, 0.0f), vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), 0.0f, NULL);  //top of ground is now at 0

	// ADD MOUNTAIN
	mountain = loadObjObject(mountainMesh(), true, mountainVAO, false, vec3(5.0f, 0.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), 0.0f, NULL);

	// Add footballs
	const char* footballFileName = "../CGCommon/meshes/Football/football3.obj";
	vector<objl::Mesh> meshes = loadMeshes(footballFileName);   // returns 2
	footballw = loadObjObject(meshes[0], true, footballwVAO, true, vec3(0.0f, 1.0f, 0.0f), vec3(0.3f, 0.3f, 0.3f), vec3(1.0f, 1.0f, 1.0f), 0.8f, NULL);
	footballb = loadObjObject(meshes[1], true, footballbVAO, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, &footballw);
	footballw2 = loadObjObject(meshes[0], false, footballwVAO, true, vec3(-2.0f, 1.0f, 1.0f), vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 0.0f), 0.75f, NULL);
	footballb2 = loadObjObject(meshes[1], false, footballbVAO, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, &footballw2);
	footballw3 = loadObjObject(meshes[0], false, footballwVAO, true, vec3(-2.0f, 1.0f, -1.0f), vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f), 0.83f, NULL);
	footballb3 = loadObjObject(meshes[1], false, footballbVAO, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), 0.0f, &footballw3);
	footballw.mass = 0.5f;
	footballw2.mass = 0.7f;
	footballw3.mass = 0.6f;

	// This is a hack - Need to update startVBO and startIBO - as these are created to start after the first football
	footballw2.startVBO = footballw3.startVBO = footballw.startVBO;
	footballw2.startIBO = footballw3.startIBO = footballw.startIBO;
	footballb2.startVBO = footballb3.startVBO = footballb.startVBO;
	footballb2.startIBO = footballb3.startIBO = footballb.startIBO;

	// tree
	const char* treeFileName = "../CGCommon/meshes/DeadTree/DeadTree.obj";
	vector<objl::Mesh> treemeshes = loadMeshes(treeFileName);
	tree = loadObjObject(treemeshes[0], true, treeVAO, false, vec3(-0.75f, -0.1f, 0.0f), vec3(0.1f, 0.2f, 0.1f), vec3(0.139f, 0.69f, 0.19f), 0.0f, NULL);

	// add flagpost
	const char* flagPostFileName = "../CGCommon/meshes/Cylinder/cylinder.obj";
	flagpost = loadObjObject(loadMeshes(flagPostFileName)[0], true, flagpostVAO, false, vec3(-5.0f, 0.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.6f, 0.5f, 0.2f), 0.0f, NULL);

	// Create VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
		
	glBufferData(GL_ARRAY_BUFFER, n_vbovertices * 8 * sizeof(float), NULL, GL_STATIC_DRAW);  // Vertex contains 8 floats: position (vec3), normal (vec3), texture (vec2)

	// Start addition objects to containerVAO	
	addToObjectBuffer(&ground, groundVAO);
	addToObjectBuffer(&mountain, mountainVAO);
	addToObjectBuffer(&footballw, footballwVAO);
	addToObjectBuffer(&footballb, footballbVAO);
	addToObjectBuffer(&tree, treeVAO);
	addToObjectBuffer(&flagpost, flagpostVAO);

	// Create IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_ibovertices * sizeof(unsigned int), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, ground.startIBO * sizeof(unsigned int), sizeof(unsigned int) * ground.Mesh.Indices.size(), &ground.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, mountain.startIBO * sizeof(unsigned int), sizeof(unsigned int) * mountain.Mesh.Indices.size(), &mountain.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, footballw.startIBO * sizeof(unsigned int), sizeof(unsigned int) * footballw.Mesh.Indices.size(), &footballw.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, footballb.startIBO * sizeof(unsigned int), sizeof(unsigned int) * footballb.Mesh.Indices.size(), &footballb.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, tree.startIBO * sizeof(unsigned int), sizeof(unsigned int) * tree.Mesh.Indices.size(), &tree.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, flagpost.startIBO * sizeof(unsigned int), sizeof(unsigned int) * flagpost.Mesh.Indices.size(), &flagpost.Mesh.Indices[0]);


	/// ------------ WATER PARTICLES --------

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};

	glGenVertexArrays(1, &waterParticleVAO);
	glBindVertexArray(waterParticleVAO);

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);

	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))	
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0); // Note that we skip over the normal vectors
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
		
	/// ------------ FLAG PARTICLES --------	
	int numFlagParticles = flag.NumParticlesWidth() * flag.NumParticlesHeight();
	int numWidth = flag.NumParticlesWidth();
	int numHeight = flag.NumParticlesHeight();

	// indices
	unsigned int* flagIndices = new unsigned int[(numHeight - 1) * (numWidth - 1) * 6];

	for (int i = 0; i < numWidth - 1; i++)
	{
		for (int j = 0; j < numHeight - 1; j++)
		{	
			flagIndices[j * 6 * (numWidth - 1) + 6 * i] = (unsigned int)(j *  numWidth + i);
			flagIndices[j * 6 * (numWidth - 1) + 6 * i + 1] = (unsigned int)(j *  numWidth + i + 1);
			flagIndices[j * 6 * (numWidth - 1) + 6 * i + 2] = (unsigned int)((j + 1) *  numWidth + i);
			flagIndices[j * 6 * (numWidth - 1) + 6 * i + 3] = (unsigned int)(j *  numWidth + i + 1);
			flagIndices[j * 6 * (numWidth - 1) + 6 * i + 4] = (unsigned int)((j + 1) *  numWidth  + i + 1);
			flagIndices[j * 6 * (numWidth - 1) + 6 * i + 5] = (unsigned int)((j + 1) *  numWidth + i);
		}
	}

	glBindVertexArray(flagVAO);
	addToFlagBuffer();

	// FLAG IBO
	glGenBuffers(1, &flagIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flagIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (numHeight - 1) * (numWidth - 1) * 2 * 3 * sizeof(unsigned int), flagIndices, GL_STATIC_DRAW);
	
	linkFlagBuffertoShader();

	//glEnableVertexAttribArray(0);
	//glBindVertexArray(0);
	
	delete flagIndices;
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	createShaders();

	initialiseWaterTexture();
	initialiseWaterParticles();

	initialiseWaterParticles2();

	// Vertex shader
	CameraRight_worldspace_ID = glGetUniformLocation(waterParticleID, "CameraRight_worldspace");
	CameraUp_worldspace_ID = glGetUniformLocation(waterParticleID, "CameraUp_worldspace");
	ViewProjMatrixID = glGetUniformLocation(waterParticleID, "VP");
	projection_waterparticle = glGetUniformLocation(waterParticleID, "projection");
	view_waterparticle = glGetUniformLocation(waterParticleID, "view");
	model_waterparticle = glGetUniformLocation(waterParticleID, "model");

	TextureID = glGetUniformLocation(waterParticleID, "myTextureSampler");

	setupUniformVariables();

	createObjects();
}

void display()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	//deltaTime /= 10;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glLoadIdentity();
	// activate shader
	glUseProgram(programID);

	// pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &projection[0][0]);

	// camera/view transformation
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);

	//root camera
	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, &global1[0][0]);

	glUseProgram(programID);
	GLint objectColorLoc = glGetUniformLocation(programID, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(programID, "lightColor");
	GLint lightPosLoc = glGetUniformLocation(programID, "lightPos");
	GLint viewPosLoc = glGetUniformLocation(programID, "viewPos");
	
	/*GLint objectColorLocFlag = glGetUniformLocation(programID, "objectColorFlag");
	GLint lightColorLocFlag = glGetUniformLocation(flagID, "lightColorFlag");
	GLint lightPosLocFlag = glGetUniformLocation(flagID, "lightPosFlag");
	GLint viewPosLocFlag = glGetUniformLocation(flagID, "viewPosFlag");*/

	glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	//glUniform3f(lightColorLocFlag, 1.0f, 1.0f, 1.0f);
	//glUniform3f(lightPosLocFlag, lightPos.x, lightPos.y, lightPos.z);
	//glUniform3f(viewPosLocFlag, cameraPos.x, cameraPos.y, cameraPos.z);

	// DRAW objects
	for (int i = 0; i < sizeof(sceneObjects) / sizeof(sceneObjects[0]); i++)     // TODO : need to fix this hardcoding
	{
		mat4 globalCGObjectTransform = sceneObjects[i]->createTransform();
		updateUniformVariables(globalCGObjectTransform);
		sceneObjects[i]->globalTransform = globalCGObjectTransform; // keep current state		

		linkCurrentBuffertoShader(sceneObjects[i]);
		glUniform3f(objectColorLoc, sceneObjects[i]->color.r, sceneObjects[i]->color.g, sceneObjects[i]->color.b);
		sceneObjects[i]->Draw();

		// Only objects that have no Parent need Physics - otherwise parent is subject to physics
		if (sceneObjects[i]->Parent == NULL && sceneObjects[i]->subjectToGravity)
		{
			updatePhysics(deltaTime, sceneObjects[i]);
		}
	}
	glPopMatrix();


	////----------------------DRAW FLAG ------------------------------------------------

//	glUseProgram(flagID);
	glPushMatrix();

	glm::mat4 flagLocation = glm::mat4(1.0f);
	updateUniformVariables(glm::translate(flagLocation, vec3(-5.0f, 3.5f, 5.0f)), view, projection);	
	flag.addForce(glm::vec3(0, Physics::forces(1), 0) * deltaTime); //* 0.5f * 0.5f); //deltaTime); // add gravity
	flag.windForce(glm::vec3(1.0f * rand()/100.0f, 0, 1.0f * rand() / 100.0f) * deltaTime); // generate some wind each frame
	flag.timeStep(deltaTime);

	//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
	for (int x = 0; x<flag.NumParticlesWidth() - 1; x++)
	{
		for (int y = 0; y<flag.NumParticlesHeight() - 1; y++)
		{
			glm::vec3 normal = Cloth::calcTriangleNormal(flag.GetParticle(x + 1, y), flag.GetParticle(x, y), flag.GetParticle(x, y + 1));
			flag.GetParticle(x + 1, y)->addToNormal(normal);
			flag.GetParticle(x, y)->addToNormal(normal);
			flag.GetParticle(x, y + 1)->addToNormal(normal);

			normal = Cloth::calcTriangleNormal(flag.GetParticle(x + 1, y + 1), flag.GetParticle(x + 1, y), flag.GetParticle(x, y + 1));
			flag.GetParticle(x + 1, y + 1)->addToNormal(normal);
			flag.GetParticle(x + 1, y)->addToNormal(normal);
			flag.GetParticle(x, y + 1)->addToNormal(normal);
		}
	}

	addToFlagBuffer();

	linkFlagBuffertoShader();

	glUniform3f(objectColorLoc, 0.0f, 0.0f, 1.0f);  // hardcode for now
	glDrawElements(GL_TRIANGLES, (flag.NumParticlesHeight() - 1) * (flag.NumParticlesWidth() - 1) * 3 * 2, GL_UNSIGNED_INT, 0);    //flag.Draw();

	glPopMatrix();

	//ASSERT(glGetError() == GL_NO_ERROR);

	////--------------------DRAW WATER ----------------------------------------------------

	// Use water shader
	glUseProgram(waterParticleID);


	// Generate 10 new particule each millisecond,
    // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
    // newparticles will be huge and the next frame even longer.
	int newparticles = (int)(deltaTime*1000.0);
	if (newparticles > (int)(0.016f*1000.0))
		newparticles = (int)(0.016f*1000.0);

	for (int i = 0; i < newparticles; i++) {
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 2.5f; // This particle will live 5 seconds.
		ParticlesContainer[particleIndex].pos = glm::vec3(0, -1.0f, -10.0f);

		float spread = 0.8f;
		glm::vec3 maindir = glm::vec3(0.0f, 7.0f, 0.0f);
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;


		// Very bad way to generate a random color
		ParticlesContainer[particleIndex].r = 135;//rand() % 256;
		ParticlesContainer[particleIndex].g = 206; //rand() % 256;
		ParticlesContainer[particleIndex].b = 250;// rand() % 256;
		ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

		ParticlesContainer[particleIndex].size = (rand() % 1000) / 18000.0f + 0.1f;

	}

	// Simulate all particles
	int ParticlesCount = 0;
	for (int i = 0; i < MaxParticles; i++) {

		WaterParticle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= deltaTime;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.speed += glm::vec3(0.0f, Physics::forces(1), 0.0f) * (float)deltaTime * 0.5f;
				p.pos += p.speed * (float)deltaTime;
				p.cameradistance = glm::length(p.pos - cameraPos);
				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

				// Fill the GPU buffer
				g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
				g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
				g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

				g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				g_particule_color_data[4 * ParticlesCount + 0] = p.r;
				g_particule_color_data[4 * ParticlesCount + 1] = p.g;
				g_particule_color_data[4 * ParticlesCount + 2] = p.b;
				g_particule_color_data[4 * ParticlesCount + 3] = p.a;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			ParticlesCount++;

		}
	}

	SortParticles();
	
	//printf("%d ", ParticlesCount);


	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU, 
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming


	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	// -------------------------- TEXTURE --------------------------



	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);
	// Same as the billboards tutorial

	glm::mat4 projectionfountain = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glUniformMatrix4fv(projection_waterparticle, 1, GL_FALSE, &projectionfountain[0][0]);

	// camera/view transformation
	glUniformMatrix4fv(view_waterparticle, 1, GL_FALSE, &view[0][0]);
	glm::vec3 fountainPos(1.2f, 1.0f, 2.0f);
	//root camera
	glm::mat4 local1fountain(1.0f);
	local1fountain = glm::translate(local1fountain, fountainPos);
	glm::mat4 global1fountain = local1fountain;
	glUniformMatrix4fv(model_waterparticle, 1, GL_FALSE, &global1fountain[0][0]);



	glUniform3f(CameraRight_worldspace_ID, view[0][0], view[1][0], view[2][0]);
	glUniform3f(CameraUp_worldspace_ID, view[0][1], view[1][1], view[2][1]);
	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &projectionfountain[0][0]);

	//printf("view 00 %f ", view[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	// Draw the particules !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);



	// Generate 10 new particule each millisecond,
// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
// newparticles will be huge and the next frame even longer.

	int newparticles2 = (int)(deltaTime*1000.0);
	if (newparticles2 > (int)(0.016f*1000.0))
		newparticles2 = (int)(0.016f*1000.0);

	for (int i = 0; i < newparticles2; i++) {
		int particleIndex2 = FindUnusedParticle2();
		ParticlesContainer2[particleIndex2].life = 2.0f; // This particle will live 5 seconds.
		ParticlesContainer2[particleIndex2].pos = glm::vec3(0, -1.0f, -10.0f);

		float spread = 1.3f;
		glm::vec3 maindir = glm::vec3(0.0f, 2.5f, 0.0f);
		// Very bad way to generate a random direction; 
		// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		// combined with some user-controlled parameters (main direction, spread, etc)
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		ParticlesContainer2[particleIndex2].speed = maindir + randomdir * spread;



		// Very bad way to generate a random color
		//ParticlesContainer2[particleIndex2].r = 135;//rand() % 256;
		//ParticlesContainer2[particleIndex2].g = 206; //rand() % 256;
		//ParticlesContainer2[particleIndex2].b = 250;// rand() % 256;
		ParticlesContainer2[particleIndex2].a = (rand() % 256) / 3;

		ParticlesContainer2[particleIndex2].size = (rand() % 1000) / 4000.0f + 0.1f;

	}

	// Simulate all particles
	int horizontal_ticker = 0;

  horizontal_ticker += 1;
  horizontal_ticker = horizontal_ticker % 360;
	int ParticlesCount2 = 0;
	for (int i = 0; i < MaxParticles2; i++) {

		WaterParticle& p = ParticlesContainer2[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= deltaTime;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.speed += glm::vec3(0.0f, 0.5, 0.0f) * (float)deltaTime;

				

				if (p.life < 0.4f) {
					p.r = 100;
					p.g = 100;
					p.b = 100;
				}
				else if (p.life < 0.8f) {
					p.r = 255;
					p.g = 99;
					p.b = 71;
				}
				else if (p.life < 1.0f) {
					p.r = 255;
					p.g = 140;
					p.b = 0;
				}
				else if (p.life < 1.3f) {
					p.r = 255;
					p.g = 165;
					p.b = 0;
				}
				else if (p.life < 1.7f) {
					p.r = 255;
					p.g = 215;
					p.b = 0;
				}
				else if (p.life < 2.0) {
					p.r = 250;
					p.g = 250;
					p.b = 250;
				}

				p.pos += p.speed * (float)deltaTime;
				p.cameradistance = glm::length(p.pos - cameraPos);
				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

				// Fill the GPU buffer
				g_particule_position_size_data2[4 * ParticlesCount2 + 0] = p.pos.x;
				g_particule_position_size_data2[4 * ParticlesCount2 + 1] = p.pos.y;
				g_particule_position_size_data2[4 * ParticlesCount2 + 2] = p.pos.z;

				g_particule_position_size_data2[4 * ParticlesCount2 + 3] = p.size;

				g_particule_color_data2[4 * ParticlesCount2 + 0] = p.r;
				g_particule_color_data2[4 * ParticlesCount2 + 1] = p.g;
				g_particule_color_data2[4 * ParticlesCount2 + 2] = p.b;
				g_particule_color_data2[4 * ParticlesCount2 + 3] = p.a;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			ParticlesCount2++;

		}
	}

	SortParticles2();

	//printf("%d ", ParticlesCount);


	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU, 
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming


	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles2 * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount2 * sizeof(GLfloat) * 4, g_particule_position_size_data2);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles2 * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount2 * sizeof(GLubyte) * 4, g_particule_color_data2);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	// -------------------------- TEXTURE --------------------------



	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);
	// Same as the billboards tutorial

	//glm::mat4 projectionfountain = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	//glUniformMatrix4fv(projection_waterparticle, 1, GL_FALSE, &projectionfountain[0][0]);

	// camera/view transformation
	glUniformMatrix4fv(view_waterparticle, 1, GL_FALSE, &view[0][0]);
	glm::vec3 fountainPos2(5.3f, 6.8f, 15.0f);
	//root camera
	glm::mat4 local1fountain2(1.0f);
	local1fountain2 = glm::translate(local1fountain2, fountainPos2);
	glm::mat4 global1fountain2 = local1fountain2;
	glUniformMatrix4fv(model_waterparticle, 1, GL_FALSE, &global1fountain2[0][0]);



	glUniform3f(CameraRight_worldspace_ID, view[0][0], view[1][0], view[2][0]);
	glUniform3f(CameraUp_worldspace_ID, view[0][1], view[1][1], view[2][1]);
	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &projectionfountain[0][0]);

	//printf("view 00 %f ", view[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	// Draw the particules !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount2);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);



	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(void) {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Physically Based Animation", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//detect key inputs
	//glfwSetKeyCallback(window, keycallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	init();

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		display();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &footballbVAO);
	glDeleteVertexArrays(1, &footballwVAO);
	glDeleteVertexArrays(1, &treeVAO);
	glDeleteVertexArrays(1, &flagpostVAO);
	glDeleteVertexArrays(1, &groundVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteProgram(programID);
	//glDeleteProgram(flagID);
	glDeleteProgram(lightingID);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &flagVBO);
	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &flagIBO);

	delete[] g_particule_position_size_data;

	// Cleanup VBO and shader
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteProgram(waterParticleID);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &waterParticleVAO);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		rotate_angle += 1.0f;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	myyaw += xoffset;
	mypitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (mypitch > 89.0f)
		mypitch = 89.0f;
	if (mypitch < -89.0f)
		mypitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(myyaw)) * cos(glm::radians(mypitch));
	front.y = sin(glm::radians(mypitch));
	front.z = sin(glm::radians(myyaw)) * cos(glm::radians(mypitch));
	cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
