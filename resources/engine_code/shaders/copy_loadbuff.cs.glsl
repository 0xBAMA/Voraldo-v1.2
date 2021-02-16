#version 430

//note that this only effects what the parameters to glDispatchCompute are - by using gl_GlobalInvocationID, you don't need to worry what any of those numbers are
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform layout(rgba8) image3D loadbuff;   // the loadbuffer, generally containg data from the CPU

uniform bool respect_mask;         //when clearing, should you touch the masked cells?
//true means you will not touch the masked cells, false means you will indeed clear all

void main()
{
	uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous_mask = 0?)
	vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));                 //existing color value (what is the previous color?)
	vec4 lbcontent = imageLoad(loadbuff, ivec3(gl_GlobalInvocationID.xyz));

	if(pmask.r > 0 && respect_mask) //the cell was masked
	{
		// this will be more complex, need to figure out how variable mask works
		vec4 temp = vec4(0);

		// temp.r = mix(lbcontent.r, pcol.r, pmask.r/255.);
		// temp.g = mix(lbcontent.g, pcol.g, pmask.g/255.);
		// temp.b = mix(lbcontent.b, pcol.b, pmask.b/255.);
		// temp.a = mix(lbcontent.a, pcol.a, pmask.a/255.);
		temp.r = mix(lbcontent.r, pcol.r, clamp(sin(tan(gl_GlobalInvocationID.y / 14.)), 0., 1.));
		temp.g = mix(lbcontent.g, pcol.g, clamp(tan(cos(gl_GlobalInvocationID.x / 16.)), 0., 1.));
		temp.b = mix(lbcontent.b, pcol.b, clamp(cos(sin(gl_GlobalInvocationID.z / 12.)), 0., 1.));
		temp.a = mix(lbcontent.a, pcol.a, clamp(tan(sin(gl_GlobalInvocationID.y / 15.)), 0., 1.));
		// temp.r = smoothstep(lbcontent.r, pcol.r, pmask.r/255.);
		// temp.g = smoothstep(lbcontent.g, pcol.g, pmask.g/255.);
		// temp.b = smoothstep(lbcontent.b, pcol.b, pmask.b/255.);
		// temp.a = smoothstep(lbcontent.a, pcol.a, pmask.a/255.);

		imageStore(current, ivec3(gl_GlobalInvocationID.xyz), temp);  //color takes on previous color
		// imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), pmask > opmask ? pmask : opmask);  //mask is set to the greater of the two
		imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), pmask);
	}
	else
	{
		imageStore(current, ivec3(gl_GlobalInvocationID.xyz), lbcontent);
		imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), pmask);
	}
}
