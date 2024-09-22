#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : enable



layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

struct pushConsts {
    uint64_t vertsBufferAdress;
    uint64_t idxBufferAdress;
    uint64_t outBufferAdress;
};

layout(buffer_reference, scalar) buffer OutBuffer{vec4 outs[];};

layout(push_constant) uniform _pushConsts { pushConsts consts;};

void main() {
    OutBuffer outbuf = OutBuffer(consts.outBufferAdress);

	gl_PointSize = 10.0f;
	gl_Position = (ubo.projectionViewMatrix * outbuf.outs[gl_VertexIndex]);

    if(gl_VertexIndex ==0) {
//    gl_Position = ubo.projectionViewMatrix * vec4(0,0,0,1);
    fragColor = vec3(1,0,0);        
    } else if (gl_VertexIndex ==1) {
//    gl_Position = ubo.projectionViewMatrix *  vec4(1,0,1,1);
    fragColor = vec3(0,1,0);
    } else if (gl_VertexIndex ==2) {
    fragColor = vec3(0,0,1);
    } else if (gl_VertexIndex ==3) {
    fragColor = vec3(1,1,0);
    } else if (gl_VertexIndex ==4) {
    fragColor = vec3(0,1,1);
    } else if (gl_VertexIndex ==5) {
    fragColor = vec3(1,0,1);
    } else if (gl_VertexIndex ==6) {
    fragColor = vec3(1,1,1);
    } else if (gl_VertexIndex ==7) {
    fragColor = vec3(0.5,0.5,0.5);
    }
//    gl_Position = ubo.projectionViewMatrix * temp;
}
