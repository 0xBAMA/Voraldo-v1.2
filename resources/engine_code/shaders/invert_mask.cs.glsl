#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous_mask = 0?)
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));                 //existing color value (what is the previous color?)

  imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(255-pmask.r));
  imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
}
