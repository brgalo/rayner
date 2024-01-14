#version 450

layout (location = 0) in vec3 position;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
	gl_PointSize = 10.0f;
	gl_Position = ubo.projectionViewMatrix * (1.2*vec4(position,1));

    fragColor = vec3(1,0,1);
}