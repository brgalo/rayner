#version 450

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
    gl_Position = ubo.projectionViewMatrix*position;

    float lightIntensity = 0.8 ;
    fragColor = vec3(lightIntensity * color.xyz);
}