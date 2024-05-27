#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 6) in vec4 inColor0;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

#define MAX_NUM_JOINTS 128

layout(Push_constant) uniform PushConstant{
    mat4 model;
} pc;


layout (set = 0, binding = 1) uniform UBONode {
	mat4 matrix;
	mat4 jointMatrix[MAX_NUM_JOINTS];
	float jointCount;
} node;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV0;
layout (location = 3) out vec2 outUV1;
layout (location = 4) out vec4 outColor0;

void main() 
{
	outColor0 = inColor0;

	vec4 locPos;
	
	locPos = pc.model * node.matrix * vec4(inPos, 1.0);
	outNormal = normalize(transpose(inverse(mat3(pc.model * node.matrix))) * inNormal);
	outWorldPos = locPos.xyz / locPos.w;
	outUV0 = inUV0;
	outUV1 = inUV1;
	gl_Position =  ubo.projection * ubo.view * vec4(outWorldPos, 1.0);
}