#version 330 core
in vec3 flagPosition;
in vec3 flagNormal;
in vec3 flagColor;

out vec3 Normal;
out vec3 FragPos;

out vec3 color;	

uniform mat4 modelFlag;
uniform mat4 viewFlag;
uniform mat4 projectionFlag;

void main()
{
    gl_Position = projectionFlag * viewFlag *  modelFlag * vec4(flagPosition, 1.0f);
    FragPos = vec3(modelFlag * vec4(flagPosition, 1.0f));
    Normal = mat3(transpose(inverse(modelFlag))) * flagNormal;
	color = flagColor;
}