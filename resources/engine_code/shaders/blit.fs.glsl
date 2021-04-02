#version 430 core

in vec2 v_pos;

// sampler for main display texture
uniform sampler2DRect main_display_texture;

// supersampling factor, defined in includes.h
uniform float ssfactor;

out vec4 fragment_output;

// apply dithering?
// uniform bool dither;


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

float get_dither_val()
{
	return texture2D(dither, gl_FragCoord.xy/float(ditherdim)).r / 128.0 - (1.0 / 128.0);
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
