#version 430

//note that this only effects what the parameters to glDispatchCompute are - by using gl_GlobalInvocationID, you don't need to worry what any of those numbers are
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform layout(rgba8) image3D lighting; //lighting values

//these variables express whether or not each channel is being used
uniform bool use_r;
uniform bool use_g;
uniform bool use_b;
uniform bool use_a;
uniform bool use_l;

//this is the "center color"
uniform vec4 color;
uniform float l_val;

//this is how much above or below the value can be from 'color'
uniform float r_var;
uniform float g_var;
uniform float b_var;
uniform float a_var;
uniform float l_var;

// how much to mask
uniform int mask;

void main()
{
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));                 //existing color value (what is the previous color?)
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));          // this is the value of the mask before this function was called
  vec4 light = imageLoad(lighting, ivec3(gl_GlobalInvocationID.xyz));  

  //the logic is relatively simple - if the color matches the criteria, mask it
  bool do_we_mask = false;

  // rgba
  if(use_r && abs(color.r - pcol.r) < r_var)
      do_we_mask = true;

  if(use_g && abs(color.g - pcol.g) < g_var)
      do_we_mask = true;

  if(use_b && abs(color.b - pcol.b) < b_var)
      do_we_mask = true;

  if(use_a && abs(color.a - pcol.a) < a_var)
      do_we_mask = true;

  // light channel
  if(use_l && abs(l_val - light.r) < l_var)
      do_we_mask = true;

  imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol); //color can't change as a result of this operation

  if(do_we_mask)
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(mask));
  else
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), pmask);
}
