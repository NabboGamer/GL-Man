#version 330 core

// Note: Tone Mapping and Gamma Correction have been moved to the Bloom Shader because:
//       - Bloom is applied before tonemapping because it works on data in high dynamic range spaces;
//       - Gamma correction is always the last step in the post-processing pipeline, 
//         because it is about transforming the image into a visually correct format for the monitor.

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
//uniform float exposure;
//uniform float gamma;
//uniform bool useHDR;

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    //if(useHDR) {
        // Exposure tone mapping (Reinhard modified)
        //vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

        // Gamma correction
        //result = pow(result, vec3(1.0 / gamma));

        //FragColor = vec4(result, 1.0);
    //} else {
        // Gamma correction
        //vec3 result = pow(hdrColor, vec3(1.0 / gamma);

        FragColor = vec4(hdrColor, 1.0);
    //} 
}