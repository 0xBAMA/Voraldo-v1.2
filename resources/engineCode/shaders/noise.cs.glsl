#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform sampler3D tex;        //noise texture
uniform float low_thresh;  //lowest value in perlin texture to accept
uniform float high_thresh; //highest value in perlin texture to accept
uniform bool usmooth;   //color scaling by perlin value 

uniform vec4 ucolor;           //what color should it be drawn with?
vec4 color;

uniform bool draw;      //should this shape be drawn?
uniform int mask;      //this this shape be masked?

bool in_shape()
{
  //code to see if gl_GlobalInvocationID.xyz is inside the shape
  vec4 texread = texture(tex, vec3(gl_GlobalInvocationID.xyz)/256.0);   
  if(usmooth)
  {
      color = ucolor;
      color.rgb *= texread.r;
  }
  else
  {
      color = ucolor;
  }

  if(texread.r < high_thresh && texread.r > low_thresh)
    return true;
  else
    return false;
}

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous_mask = 0?)
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));                 //existing color value (what is the previous color?)

  if(in_shape())
  {
      // color takes on mix of previous color and draw op's color, based on existing mask value
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), draw ? mix(color, pcol, float(pmask.r)/255.) : pcol);
      //mask is set to the greater of the two mask values, between previous and the current operation
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > mask) ? pmask.r : mask));
  }
  else
  {
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(pmask.r));
  }
}
