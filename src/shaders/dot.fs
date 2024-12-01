#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

uniform vec3 viewPos;
uniform DirLight dirLight;
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
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main() {    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 ambient, diffuse, specular;
    // combine results
    if (useTextures) {
        // Use textures for components
        ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    } else {
        // Use uniform colors for components
        ambient = light.ambient * ambientColor;
        diffuse = light.diffuse * diff * diffuseColor;
        specular = light.specular * spec * specularColor;
    }
    
    return (ambient + diffuse + specular);
}
