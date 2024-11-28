#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
uniform float gamma;

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // Exposure tone mapping (Reinhard modified)
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma correction
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}
