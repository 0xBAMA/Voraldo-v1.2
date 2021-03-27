#version 430 core

in vec2 v_pos;

// sampler for main display texture
uniform sampler2DRect main_display_texture;

// supersampling factor, defined in includes.h
uniform float ssfactor;

out vec4 fragment_output;

// apply dithering?
// uniform bool dither;

// bayer matrix
/*const float bayer_pattern[] = { 0./255., 32./255.,  8./255., 40./255.,  2./255., 34./255., 10./255., 42./255.,
                               48./255., 16./255., 56./255., 24./255., 50./255., 18./255., 58./255., 26./255.,
                               12./255., 44./255.,  4./255., 36./255., 14./255., 46./255.,  6./255., 38./255.,
                               60./255., 28./255., 52./255., 20./255., 62./255., 30./255., 54./255., 22./255.,
                                3./255., 35./255., 11./255., 43./255.,  1./255., 33./255.,  9./255., 41./255.,
                               51./255., 19./255., 59./255., 27./255., 49./255., 17./255., 57./255., 25./255.,
                               15./255., 47./255.,  7./255., 39./255., 13./255., 45./255.,  5./255., 37./255.,
                               63./255., 31./255., 55./255., 23./255., 61./255., 29./255., 53./255., 21./255.};*/


uniform sampler2D dither;
uniform int ditherdim; // can't use imagesize to get dimension

// COLOR TEMP ADJUSTMENT
uniform vec3 temp_adjustment;

// Gamma Correction
uniform float gamma;



mat3 temp_adjust(vec3 c)
{
	mat3 t;
	t[0] = vec3(c.r,   0,   0);
	t[1] = vec3(  0, c.g,   0);
	t[2] = vec3(  0,   0, c.b);
	return t;
}

/*float get_dither_val()
{
	const int x = int(gl_FragCoord.x)%8;
	const int y = int(gl_FragCoord.y)%8;
	return bayer_pattern[(x + y * 8)];
}*/

float get_dither_val()
{
	return 1.0;
}

void main()
{
	vec4 texread_color = texture(main_display_texture, ssfactor*(gl_FragCoord.xy + gl_SamplePosition.xy));
	vec4 running_color = texread_color;

	// temperatury adjust
	running_color.xyz = temp_adjust(temp_adjustment) * running_color.xyz;

	// luminancy preservation
	running_color.xyz *= dot(texread_color.xyz, vec3(0.2126, 0.7152, 0.0722)) / max(dot(running_color.xyz, vec3(0.2126, 0.7152, 0.0722)), 1e-5);

	// gamma correction
	running_color = pow(running_color, vec4(1./gamma));

	// fragment output
	fragment_output = running_color;

	// dither
	if(ditherdim > 0)
		fragment_output.rgb += vec3(get_dither_val());
}
