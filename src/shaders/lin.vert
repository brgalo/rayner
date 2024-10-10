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
    uint64_t hitBufferAddress;
    uint64_t energyBufferAddress;
    uint64_t currentTri;
};

layout(buffer_reference, scalar) buffer OriBuffer{vec4 oris[];};
layout(buffer_reference, scalar) buffer DirBuffer{vec4 dirs[];};

layout(push_constant) uniform _pushConsts { pushConsts consts;};

const float AMBIENT = 0.02;

void main() {

    OriBuffer oribuf = OriBuffer(consts.oriBufferAddress);
    DirBuffer dirbuf = DirBuffer(consts.dirBufferAddress);

    if (gl_VertexIndex % 2 ==0) {
    gl_Position = ubo.projectionViewMatrix*vec4(oribuf.oris[gl_VertexIndex/2].xyz,1);
    } else {
    gl_Position = ubo.projectionViewMatrix*vec4(dirbuf.dirs[gl_VertexIndex/2].xyz,1);
    }

    float lightIntensity = 0.8 ;
    if(dirbuf.dirs[gl_VertexIndex/2].w > 7) {
    fragColor = vec3(1,0.2,0.2);
    } else {
        fragColor = vec3(0.2,1,0.2);
    }
}