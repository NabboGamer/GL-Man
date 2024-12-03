#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix;

layout (std140) uniform Shared
{
    mat4 projection;
    mat4 view;
    vec3 lightDir;
    vec3 lightPos;
    vec3 lightSpecular;
    float materialShininess;
};

layout (std140) uniform Other
{
    vec3 lightAmbient;
    vec3 lightDiffuse;
};

out DIR_LIGHT {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 viewPos;
out float shininess;

void main() {
     FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    TexCoords = aTexCoords;
    viewPos = lightPos;

    light.direction = lightDir;
    light.specular = lightSpecular;
    light.ambient = lightAmbient;
    light.diffuse = lightDiffuse;
    shininess = materialShininess;
    
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f);
}