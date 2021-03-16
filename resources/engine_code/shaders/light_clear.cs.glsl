#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; //workgroup dimensions

uniform layout(rgba16f) image3D lighting;
uniform layout(rgba16f) image3D lighting_cache;

uniform vec4 intensity;
uniform bool use_cache;

void main()
{
    // use alpha to boost? how are we going to deal with the fact that incoming values are constrained to (0.-1.)?
	if(use_cache)
		imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), imageLoad(lighting_cache, ivec3(gl_GlobalInvocationID.xyz)));
	else
   	imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), vec4(intensity));
}

