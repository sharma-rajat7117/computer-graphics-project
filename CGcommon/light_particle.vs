#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 squareVertices;
layout(location = 1) in vec4 xyzs; // Position of the center of the particule and size of the square
layout(location = 2) in vec4 color; // Position of the center of the particule and size of the square


layout (location = 3) in vec2 aTexCoords;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main()
{
	 TexCoords = aTexCoords;
	float particleSize = xyzs.w; // because we encoded it this way.
	vec3 particleCenter_wordspace = xyzs.xyz;

	vec3 cr = vec3(view[0][0],view[1][0],view[2][0]);
    	vec3  cu = vec3(view[0][1],view[1][1],view[2][1]);
    	vec3 finalPos = particleCenter_wordspace +
	  cr *squareVertices.x * particleSize +
	 cu * squareVertices.y * particleSize;
	
	//vec3 vertexPosition_worldspace = 
	//	particleCenter_wordspace
	//	+  CameraRight_worldspace *squareVertices.x * particleSize
	//	+  CameraUp_worldspace * squareVertices.y * particleSize;

	// Output position of the vertex
	gl_Position = projection * view * model  * vec4(finalPos, 1.0f);

	// UV of the vertex. No special space for this one.
	UV = squareVertices.xy + vec2(0.5, 0.5);
	particlecolor = color;
}