#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : enable


layout (location = 0) out vec4 fragColor;

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

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

struct hitInfo {
    uint64_t tri;
    float energy;
};


layout(push_constant) uniform _pushConsts { pushConsts consts;};

layout(buffer_reference, scalar) buffer VertBuffer{vec4 verts[];};
layout(buffer_reference, scalar) buffer IndexBuffer{uint idxs[];};
layout(buffer_reference, scalar) buffer EnergyBuffer{float e[];};

void main() {
    EnergyBuffer energybuf = EnergyBuffer(consts.energyBufferAddress);
    IndexBuffer idxbuf = IndexBuffer(consts.idxBufferAddress);
    VertBuffer vertbuf = VertBuffer(consts.vertsBufferAddress);

    // retrieve geom data
    uint idx = idxbuf.idxs[gl_VertexIndex];

    vec4 position = vertbuf.verts[idx];

    gl_Position = ubo.projectionViewMatrix*position;
    
    idx = gl_VertexIndex;

    idx = idx/3;

    vec4 colA = vec4(1,0,0,1);
    vec4 colB = vec4(0,1,0,1);
    
    float e = energybuf.e[idx];

    vec4 col = mix(colA,colB,e);

    fragColor = col;

    //fragColor = vec3(0.2,0.2,energybuf.e[gl_VertexIndex/3]);
}