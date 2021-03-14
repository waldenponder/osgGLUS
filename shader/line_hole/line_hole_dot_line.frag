#version 430  compatibility

flat in uint g_id;
flat in vec4 v_linePt;
in vec4 g_color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint idTexture;
layout(location = 2) out vec4 depthTexture;
layout(location = 3) out vec4 linePtTexture;
uniform bool u_is_hidden_line;

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
 // if(u_is_hidden_line)
		
   //else 
	//	FragColor = g_color;
   FragColor = vec4(0.2, 0.4, 0.6, 0.8);
   idTexture = g_id;
   depthTexture =   packFloatToRgba(gl_FragCoord.z);
   linePtTexture = v_linePt;
}
