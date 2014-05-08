#version 330

out vec4 fragColor;
in vec3 vertexNormal;

void main(void) {
    
    // --------------- Color --------------- //
    fragColor = vec4(0.5, 0.5, 0.5, 0);
    
    // -------------- Lighting ------------- //
    
    // Phong model
    // I = light_ambient * material_ambient +
    //     light_diffuse * material_diffuse * cos(light, normal) +
    //     light_specular * material_specular * cos(reflection, viewer)^shineniness
    
    // ... TODO ... //
}
