#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec3 color;
  
uniform vec3 lightPosFlag; 
uniform vec3 viewPosFlag; 
uniform vec3 lightColorFlag;
uniform vec3 objectColorFlag;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColorFlag;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosFlag - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColorFlag;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPosFlag - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColorFlag;  
        
    vec3 result = (ambient + diffuse + specular) * objectColorFlag;
    FragColor = vec4(result, 1.0);
} 