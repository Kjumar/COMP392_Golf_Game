#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;


struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor;
	vec4 directionalLight;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix; 
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);

	vec4 outlineNormal = vec4(fragNormalWorld.xyz, 0.0) * 0.003;
	mat4 view_transform = ubo.projection * ubo.view;
	float outline_thickness = clamp((view_transform * positionWorld).w / 2, 1.0, 2.0);
	gl_Position = view_transform * (positionWorld + (outlineNormal * outline_thickness));
	fragPosWorld = positionWorld.xyz;
	fragColor = vec3(0.0);
}