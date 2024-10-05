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

void main() {
    // always uses outbuf var -> change push consts in render call to point to right buffer!
    OutBuffer outbuf = OutBuffer(consts.outBufferAddress);

	gl_PointSize = 10.0f;
	gl_Position = (ubo.projectionViewMatrix * outbuf.outs[gl_VertexIndex]);

    if(gl_VertexIndex%8 ==0) {
//    gl_Position = ubo.projectionViewMatrix * vec4(0,0,0,1);
    fragColor = vec3(1,0,0);        
    } else if (gl_VertexIndex%8 ==1) {
//    gl_Position = ubo.projectionViewMatrix *  vec4(1,0,1,1);
    fragColor = vec3(0,1,0);
    } else if (gl_VertexIndex%8 ==2) {
    fragColor = vec3(0,0,1);
    } else if (gl_VertexIndex%8 ==3) {
    fragColor = vec3(1,1,0);
    } else if (gl_VertexIndex%8 ==4) {
    fragColor = vec3(0,1,1);
    } else if (gl_VertexIndex%8 ==5) {
    fragColor = vec3(1,0,1);
    } else if (gl_VertexIndex%8 ==6) {
    fragColor = vec3(1,1,1);
    } else if (gl_VertexIndex%8 ==7) {
    fragColor = vec3(0.5,0.5,0.5);
    }
//    gl_Position = ubo.projectionViewMatrix * temp;
}
