#version 450
#extension GL_GOOGLE_include_directive : enable


layout(set = 0, binding = 3) uniform sampler2D base_color_texture_sampler;
layout(location = 0) in vec2 g_tex_coord;
#define MIN_SHADOW_ALPHA 0.001

void main() 
{	
	float alpha = texture(base_color_texture_sampler, g_tex_coord).a;
	if (alpha < MIN_SHADOW_ALPHA)
	{
		discard;
	}
}