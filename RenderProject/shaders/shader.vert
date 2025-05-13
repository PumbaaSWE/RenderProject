#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 not_in_use;
    mat4 view;
    mat4 proj;
} ubo;


layout(push_constant, std430) uniform PushConstant {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 normal;


void main() {
    //gl_Position = vec4(inPosition, 1.0);
	gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 1.0);
    //vec3 temp = ubo.model[0].xyz;
mat4 nm = inverse(ubo.view * pc.model);
nm = transpose(nm);
	vec4 n = nm * vec4(inNormal, 1.0);
    normal = n.xyz;
}