#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable



layout (location = 0) in vec3 position;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
} ubo;

struct pushConsts {
    uint64_t outBufferAdress;
};

layout(buffer_reference, scalar) buffer OutBuffer{vec4 outs[];};

layout(push_constant) uniform _pushConsts { pushConsts consts;};

void main() {
    OutBuffer outbuf = OutBuffer(consts.outBufferAdress);

	gl_PointSize = 10.0f;
	gl_Position = ubo.projectionViewMatrix * (vec4(outbuf.outs[gl_VertexIndex].xyz,1));
         debugPrintfEXT("My float is %d", gl_VertexIndex);   

    if(gl_VertexIndex % 2 ==0) {
//    gl_Position = ubo.projectionViewMatrix * vec4(0,0,0,1);
    fragColor = vec3(1,0,1);        
    } else {
//    gl_Position = ubo.projectionViewMatrix *  vec4(1,0,1,1);
    fragColor = vec3(0,1,0);
    }

}