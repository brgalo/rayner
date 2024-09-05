#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec2 uv;
    int hitIdx;
    float energy;
};