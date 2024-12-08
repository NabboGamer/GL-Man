#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

uniform float exposure;
uniform bool useHDR;
uniform float gamma;

void main() {
    // bloom effect
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    hdrColor += bloomColor; // additive blending

    if(useHDR) {
        // tone mapping
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma correction
        result = pow(result, vec3(1.0 / gamma));

        FragColor = vec4(result, 1.0);
    }
    else{
        // gamma correction    
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));

        FragColor = vec4(result, 1.0);
    }
}