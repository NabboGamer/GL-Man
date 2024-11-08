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
uniform bool hasAmbientTexture;
uniform bool hasDiffuseTexture;
uniform bool hasSpecularTexture;

// Colors for material when textures are not available
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

// Textures
uniform sampler2D texture_ambient1;
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
    vec3 reflectDir = reflect(-lightDir, normal);

    float roughness;
    float specularStrength;
    if(hasSpecularTexture){
        roughness = texture(texture_specular1, TexCoords).r;
        // Specularity is the inverse of Roughness
        specularStrength = roughness;
    } else {
        specularStrength = specularColor.x;
    }

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * specularStrength;
    
    vec3 ambient, diffuse, specular;
    // combine results

    if(hasAmbientTexture){
        ambient = light.ambient * ambientColor * vec3(texture(texture_ambient1, TexCoords));
    } else {
        ambient = light.ambient * ambientColor;
    }
    
    if(hasDiffuseTexture){
        diffuse = light.diffuse * diff * diffuseColor * vec3(texture(texture_diffuse1, TexCoords));
    } else {
        diffuse = light.diffuse * diff * diffuseColor;
    }
    
    if(hasSpecularTexture){
        specular = light.specular * spec * specularColor * vec3(texture(texture_specular1, TexCoords));
    } else {
        specular = light.specular * spec * specularColor;
    }
    
    return (ambient + diffuse + specular);
}
