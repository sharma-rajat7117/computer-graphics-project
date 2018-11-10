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
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "CGobject.h"
#include "..\Dependencies\OBJ_Loader.h"

using namespace std;

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

// Shader attribute locations
GLuint loc1;
GLuint loc2;
GLuint loc3;

// Uniform locations
int model_mat_location;
int view_mat_location;
int proj_mat_location;

// Buffers
GLuint VBO;
GLuint IBO;
GLuint groundVAO;
GLuint footballwVAO;
GLuint footballbVAO;
GLuint lightVAO;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// objects
CGCommon::CGObject ground;
CGCommon::CGObject footballw, footballb;

bool firstMouse = true;
float myyaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float mypitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//lighting position
glm::vec3 lightPos(1.0f, 1.0f, 3.0f);

bool rotateCubes = false;

enum class MeshType
{
	ground,
	football,
};

void bindVertexAttribute(int location, int locationSize, int startVBO, int offsetVBO)
{
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(location, locationSize, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void*)(startVBO * 8 * sizeof(float) + BUFFER_OFFSET(offsetVBO * sizeof(GLfloat))));
}

void linkCurrentBuffertoShader(CGCommon::CGObject cg_object)
{
	if (cg_object.Mesh.MeshName == "footballw")
	{
		glBindVertexArray(footballwVAO);
	}

	if (cg_object.Mesh.MeshName == "footballb")
	{
		glBindVertexArray(footballbVAO);
	}

	if (cg_object.Mesh.MeshName == "ground")
	{
		glBindVertexArray(groundVAO);
	}
	
	bindVertexAttribute(loc1, 3, cg_object.startVBO, 0);
	bindVertexAttribute(loc2, 3, cg_object.startVBO, 3);
	bindVertexAttribute(loc3, 2, cg_object.startVBO, 6);

	//IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);


}

void addToObjectBuffer(CGCommon::CGObject cg_object)  //MeshType meshType, int startVBO, int n_vertices, float *vertices)
{
	glBufferSubData(GL_ARRAY_BUFFER, cg_object.startVBO * 8 * sizeof(GLfloat), cg_object.Mesh.Vertices.size() * 8 * sizeof(GLfloat), &cg_object.Mesh.Vertices[0].Position.X);

	// Vertex Attribute array	
	if (cg_object.Mesh.MeshName == "footballw")
	{
		glGenVertexArrays(1, &footballwVAO);
	}

	if (cg_object.Mesh.MeshName == "footballb")
	{
		glGenVertexArrays(1, &footballbVAO);
	}

	if (cg_object.Mesh.MeshName == "ground")
	{
		glGenVertexArrays(1, &groundVAO);
	}

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
}

void setupUniformVariables()
{
	//Declare your uniform variables that will be used in your shader
	model_mat_location = glGetUniformLocation(programID, "model");
	view_mat_location = glGetUniformLocation(programID, "view");
	proj_mat_location = glGetUniformLocation(programID, "projection");
}

objl::Mesh groundMesh()
{
	objl::Vertex point1, point2, point3, point4, point5, point6, point7, point8 = objl::Vertex();

	point1.Position = objl::Vector3(-1.0f, -0.5f, -0.5f);
	point1.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point2.Position = objl::Vector3(-1.0f, 0.5f, -0.5f);
	point2.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point3.Position = objl::Vector3(-0.5f, -0.5f, 0.5f);
	point3.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point4.Position = objl::Vector3(-0.5f, 0.5f, 0.5f);
	point4.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point5.Position = objl::Vector3(0.5f, -0.5f, 0.5f);
	point5.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point6.Position = objl::Vector3(0.5f, 0.5f, 0.5f);
	point6.Normal = objl::Vector3(0.0f, -1.0f, 0.0f);
	point7.Position = objl::Vector3(0.5f, -0.5f, -0.5f);
	point7.Normal = objl::Vector3(0.0f, 1.0f, 0.0f);
	point8.Position = objl::Vector3(0.5f, 0.5f, -0.5f);
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
	mesh.MeshName = "ground";	
	mesh.MeshMaterial.Ns = 94.0;
	mesh.MeshMaterial.Ni = 1.0;
	mesh.MeshMaterial.d = 1.0;
	mesh.MeshMaterial.illum = 2;
	mesh.MeshMaterial.Kd.X = 0.64;
	mesh.MeshMaterial.Kd.Y = 0.64;
	mesh.MeshMaterial.Kd.Z = 0.64;
	mesh.MeshMaterial.Ks.X = 0.5;
	mesh.MeshMaterial.Ks.Y = 0.5;
	mesh.MeshMaterial.Ks.Z = 0.5;

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
	mesh.MeshName = "ground";
	return mesh;
}

std::vector<objl::Mesh> LoadMeshes(const char* objFileLocation)
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

void createObjects()
{
	int n_vbovertices = 0;
	int n_ibovertices = 0;

	CGCommon::CGObject ground = CGCommon::CGObject();
	ground.Mesh = groundMesh();
	ground.initialTranslateVector = vec3(0.0f, 0.0f, 0.0f);
	ground.initialScaleVector = vec3(0.5f, 0.5f, 0.5f);
	ground.color = vec3(0.0f, 1.0f, 0.0f);   // Quick solution for color as we are not using texture
	ground.startVBO = n_vbovertices;
	ground.startIBO = n_ibovertices;
	n_vbovertices += ground.Mesh.Vertices.size();
	n_ibovertices += ground.Mesh.Indices.size();

	// load meshes with OBJ Loader	
	const char* footballFileName = "../CGCommon/meshes/Football/football3.obj";

	// football
	vector<objl::Mesh> meshes = LoadMeshes(footballFileName);   // returns 2
	
	footballw = CGCommon::CGObject();
	footballb = CGCommon::CGObject();	
			
	// create 2 objects - one for White and one for Black	
	footballw.Mesh = meshes[0];
	footballw.Mesh.MeshName = "footballw";
	footballb.Mesh = meshes[1];
	footballb.Mesh.MeshName = "footballb";

	footballw.initialTranslateVector = footballb.initialTranslateVector = vec3(0.0f, 0.0f, 0.0f);
	footballw.initialScaleVector = footballb.initialScaleVector = vec3(0.1f, 0.1f, 0.1f);
	//footballw.color = vec3(0.0f, 0.0f, 1.0f);   // Quick solution for color as we are not using texture
	
	footballw.startVBO = n_vbovertices;
	footballw.startIBO = n_ibovertices;
	n_vbovertices += footballw.Mesh.Vertices.size();
	n_ibovertices += footballw.Mesh.Indices.size();

	footballb.startVBO = n_vbovertices;
	footballb.startIBO = n_ibovertices;
	n_vbovertices += footballb.Mesh.Vertices.size();
	n_ibovertices += footballb.Mesh.Indices.size();
	
	// Shader Attribute locations
	loc1 = glGetAttribLocation(programID, "position");
	loc2 = glGetAttribLocation(programID, "normal");
	loc3 = glGetAttribLocation(programID, "texture");

	// Create VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, n_vbovertices * 8 * sizeof(float), NULL, GL_STATIC_DRAW);  // Vertex contains 8 floats: position (vec3), normal (vec3), texture (vec2)

	// Start addition objects to containerVAO	
	addToObjectBuffer(ground); 
	addToObjectBuffer(footballw);
	addToObjectBuffer(footballb);

		// Create IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_ibovertices * sizeof(unsigned int), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, ground.startIBO * sizeof(unsigned int), sizeof(unsigned int) * ground.Mesh.Indices.size(), &ground.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, footballw.startIBO * sizeof(unsigned int), sizeof(unsigned int) * footballw.Mesh.Indices.size(), &footballw.Mesh.Indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, footballb.startIBO * sizeof(unsigned int), sizeof(unsigned int) * footballb.Mesh.Indices.size(), &footballb.Mesh.Indices[0]);

	//glBindVertexArray(containerVAO);
	//// Position attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	//glEnableVertexAttribArray(0);

	//// Normal attribute
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(1);
	//glBindVertexArray(0);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))	
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0); // Note that we skip over the normal vectors
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	createShaders();

	setupUniformVariables();

	createObjects();
}

void display()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	// render
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glPushMatrix();
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
	glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	// DRAW GROUND
	mat4 globalGroundTransform = ground.createTransform();// Root of the Hierarchy				
	updateUniformVariables(globalGroundTransform);
	ground.globalTransform = globalGroundTransform; // keep current state
	//glUniform3f(quickObjectColor_location, football.color.v[0], football.color.v[1], football.color.v[2]);

	glBindVertexArray(groundVAO);
	linkCurrentBuffertoShader(ground);
	glUniform3f(objectColorLoc, 1.0f, 0.0f, 0.0f);
	ground.Draw();

	// DRAW FOOTBALL - white
	mat4 globalFootballwTransform = footballw.createTransform();// Root of the Hierarchy				
	updateUniformVariables(globalFootballwTransform);
	footballw.globalTransform = globalFootballwTransform; // keep current state
	//glUniform3f(quickObjectColor_location, football.color.v[0], football.color.v[1], football.color.v[2]);

	glBindVertexArray(footballwVAO);
	linkCurrentBuffertoShader(footballw);
	glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
	footballw.Draw();

	// DRAW FOOTBALL - black
	mat4 globalFootballbTransform = footballb.createTransform();// Root of the Hierarchy				
	updateUniformVariables(globalFootballbTransform);
	footballb.globalTransform = globalFootballbTransform; // keep current state
														//glUniform3f(quickObjectColor_location, football.color.v[0], football.color.v[1], football.color.v[2]);

	glBindVertexArray(footballbVAO);
	linkCurrentBuffertoShader(footballb);
	glUniform3f(objectColorLoc, 0.0f, 0.0f, 0.0f);
	footballb.Draw();

	//glBindVertexArray(containerVAO);
	////draw cube
	//glm::mat4 model(1.0f);

	//glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, &model[0][0]);
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	//glBindVertexArray(0);

	//// Lamp
	//glUseProgram(lightingID);
	//// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
	//GLint modelLoc = glGetUniformLocation(lightingID, "model");
	//GLint viewLoc = glGetUniformLocation(lightingID, "view");
	//GLint projLoc = glGetUniformLocation(lightingID, "projection");
	//// Set matrices
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//model = glm::mat4();
	//model = glm::translate(model, lightPos);
	//model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//// Draw the light object (using light's vertex attributes)
	//glBindVertexArray(lightVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//glBindVertexArray(0);

	//glPopMatrix();

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
	glDeleteVertexArrays(1, &footballwVAO);
	glDeleteVertexArrays(1, &groundVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);

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
