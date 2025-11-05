#version 450

layout(push_constant) uniform pConstants
{
	int useTexture;
} pc;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	float textureWidth = 0.25;

	
	if(pc.useTexture == -1)
		outColor = vec4(fragColor, 1.0);
	else
	{
		vec2 localUV = vec2(pc.useTexture * textureWidth + fragUV.x * textureWidth, fragUV.y);
		outColor = texture(texSampler, localUV);
	}
		
}