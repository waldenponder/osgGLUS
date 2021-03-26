#version 430  compatibility

flat in int g_id;
flat in vec4 v_linePt;
in vec4 g_color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int  idTexture;
layout(location = 2) out vec4 depthTexture;
layout(location = 3) out vec4 linePtTexture;
uniform sampler2D depthTextureSampler;

//bgfx shaderlib.sh
vec4 packFloatToRgba(float _value)
{
	const vec4 shift = vec4(256 * 256 * 256, 256 * 256, 256, 1.0);
	const vec4 mask = vec4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
	vec4 comp = fract(_value * shift);
	comp -= comp.xxyz * mask;
	return comp;
}

float unpackRgbaToFloat(vec4 _rgba)
{
	const vec4 shift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(_rgba, shift);
}

void main()
{
	FragColor = g_color;
	
	//if(g_id == 1)  FragColor = vec4(1,0,0,1);
	//else if(g_id == 2)  FragColor = vec4(1,1,0,1);
	
	vec2 text_size = textureSize(depthTextureSampler, 0);
	vec4 p2 = texture(depthTextureSampler, gl_FragCoord.xy / text_size);
	float depth = unpackRgbaToFloat(p2);
	//if (depth == 0)
	//FragColor = vec4(1,1,0,1);
	if(depth > 0 && gl_FragCoord.z > depth)
		FragColor = vec4(0);
	  
	idTexture = g_id;
	depthTexture =   packFloatToRgba(gl_FragCoord.z);
	linePtTexture = v_linePt;
}