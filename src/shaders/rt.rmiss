#version 460

#include "commonrt.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    payload.uv = vec2(0.5,.5);
    payload.hitIdx = -1;
    payload.energy = 0;
}