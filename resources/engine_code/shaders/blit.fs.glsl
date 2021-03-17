#version 430 core

in vec2 v_pos;

// sampler for main display texture
uniform sampler2DRect main_display_texture;

// supersampling factor, defined in includes.h
uniform float ssfactor;

out vec4 fragment_output;



// COLOR TEMP ADJUSTMENT
uniform vec3 temp_adjustment;

mat3 temp_adjust(vec3 c)
{
	mat3 t;
	t[0] = vec3(c.r,   0,   0);
	t[1] = vec3(  0, c.g,   0);
	t[2] = vec3(  0,   0, c.b);
	return t;
}

void main()
{
	vec4 texread_color = texture(main_display_texture, ssfactor*(gl_FragCoord.xy + gl_SamplePosition.xy));
	vec4 running_color = texread_color;

	// temperature correction
	running_color.xyz = temp_adjust(temp_adjustment) * running_color.xyz;

	// luminance preservation
	running_color.xyz *= dot(texread_color.xyz, vec3(0.2126, 0.7152, 0.0722)) / max(dot(running_color.xyz, vec3(0.2126, 0.7152, 0.0722)), 1e-5);


	// fragment output
	fragment_output = running_color;
}
