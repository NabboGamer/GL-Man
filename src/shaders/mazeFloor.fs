#version 330 core
// For the maze wall, being a hand-built object, it is very complex to implement Normal Mapping. 
// Since it requires manually calculating the tangent and bitangent vectors for each point that makes up the mesh.

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

// Wall color, multiplied by the white texture to apply the tint
const vec3 wallColor = vec3(0.4, 0.4, 0.4);

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main() {    
    // properties
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
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Read the roughness from texture_specular1 and invert to get the specularity
    // Load only the red roughness channel because, by convention, roughness and 
    // specularity textures are often grayscale images. This means that all channels 
    // (red, green, and blue) contain the same value, representing the intensity of 
    // roughness or specularity uniformly(optimization).
    float roughness = texture(material.specular, TexCoords).r;
    // Specularity is the inverse of Roughness
    float specularStrength = 1.0 - roughness;
    
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * specularStrength;
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess) * specularStrength;

    // combine results
    vec3 ambient = light.ambient * wallColor * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * wallColor * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec;
    
    return (ambient + diffuse + specular);
}
