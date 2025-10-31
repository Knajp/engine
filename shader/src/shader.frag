#version 450

layout(push_constant) uniform puc
{
	int useTexture;
} pc;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	if(pc.useTexture == 1)
		//outColor = vec4(1.0, 0.0, 0.0, 1.0);
		outColor = texture(texSampler, fragUV);
	else
		outColor = vec4(fragColor, 1.0);
}