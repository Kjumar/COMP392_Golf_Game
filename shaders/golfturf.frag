#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;


layout(location = 0) out vec4 outColor;

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
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);

	diffuseLight += ubo.directionalLight.w * max(dot(surfaceNormal, ubo.directionalLight.xyz), 0);

    vec3 fragColor = vec3(0.0, 0.6, 0.0) + vec3(0.0, 0.1, 0.0) * mod(trunc((fragPosWorld.x + fragPosWorld.z) * 0.8), 2);
	float bladeFactor = clamp(sin(sin(fragPosWorld.z) + fragPosWorld.x - push.normalMatrix[3].x - fragPosWorld.z) - 0.999, 0, 1) * 1000;
	float highlightIntensity = clamp(sin(push.normalMatrix[3].x / 2), 0, 1);
	vec3 highlightColor = vec3(0.3, 0.3, 0.3) * bladeFactor * clamp(cos((fragPosWorld.x / 2) - (push.normalMatrix[3].x / 4)), 0, 1) * highlightIntensity;

	outColor = vec4(diffuseLight * fragColor + highlightColor, 1.0);
}