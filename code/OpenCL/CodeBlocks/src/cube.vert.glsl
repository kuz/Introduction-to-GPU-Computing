#version 330

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec3 vertexNormal;

void main(void) {
    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));
    vertexNormal = normalize(normalMatrix * normal);
    gl_Position = modelViewProjectionMatrix*vec4(position, 1);
}
