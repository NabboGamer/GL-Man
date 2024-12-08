#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 viewPos;
in float shininess;

in DIR_LIGHT {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
};

//out vec4 FragColor;

uniform Material material;

// Flag to indicate whether to use textures
uniform bool useTextures;

// Colors for material when textures are not available
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

// Textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// function prototypes
vec3 CalcDirLight(vec3 normal, vec3 viewDir);

void main() {  
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // phase 1: directional lighting
    vec3 result = CalcDirLight(norm, viewDir);    
    
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        BrightColor = vec4(result, 1.0);
    }
    else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 ambient, diffuse, specular;
    // combine results
    // Use uniform colors for components
    ambient = light.ambient * ambientColor;
    diffuse = light.diffuse * diff * diffuseColor;
    specular = light.specular * spec * specularColor;
    
    return (ambient + diffuse + specular);
}
