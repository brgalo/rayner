#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct RayPayload {
    vec2 uv;
    int hitIdx;
    float energy;
};

const float ORIGIN = 1.0/32.0;
const float FLOAT_SCALE = 1.0/65536.0;
const float INT_SCALE = 256.0;

// function to offset ray from triangle surface!
vec3 offsetRay(const vec3 ori, const vec3 normal) {
    ivec3 of_i = ivec3(INT_SCALE*normal.x, INT_SCALE*normal.y, INT_SCALE*normal.z);
    // modify components based on sign
    vec3 p_i =  vec3(intBitsToFloat(floatBitsToInt(ori.x) + ((ori.x < 0.0) ? -of_i.x : of_i.x)),
                     intBitsToFloat(floatBitsToInt(ori.y) + ((ori.y < 0.0) ? -of_i.y : of_i.y)),
                     intBitsToFloat(floatBitsToInt(ori.z) + ((ori.z < 0.0) ? -of_i.z : of_i.z)));

return vec3(abs(ori.x) < ORIGIN ? ori.x + FLOAT_SCALE*normal.x : p_i.x,
            abs(ori.y) < ORIGIN ? ori.y + FLOAT_SCALE*normal.y : p_i.y,
            abs(ori.z) < ORIGIN ? ori.z + FLOAT_SCALE*normal.z : p_i.z);
}