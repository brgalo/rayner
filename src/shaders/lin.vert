#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : enable


layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

struct pushConsts {
    uint64_t vertsBufferAddress;
    uint64_t idxBufferAddress;
    uint64_t outBufferAddress;
    uint64_t oriBufferAddress;
    uint64_t dirBufferAddress;
    uint64_t currentTri;
};

layout(buffer_reference, scalar) buffer OutBuffer{vec4 outs[];};

layout(push_constant) uniform _pushConsts { pushConsts consts;};

const float AMBIENT = 0.02;

void main() {

    OutBuffer outbuf = OutBuffer(consts.oriBufferAddress);
    if (gl_VertexIndex % 2 ==0) {
        outbuf = OutBuffer(consts.dirBufferAddress);
    }


    gl_Position = ubo.projectionViewMatrix*outbuf.outs[gl_VertexIndex/2];

    float lightIntensity = 0.8 ;
    fragColor = vec3(0.8,0.3,0.2);
}