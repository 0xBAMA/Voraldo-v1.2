#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform sampler2D map;          //heightmap texture
uniform float vscale;            //vertically scaling the texture

uniform vec4 color;           //what color should it be drawn with?
uniform bool height_color;      //should the coloring be scaled by the height

vec4 drawcolor; // set in in_shape(), use in main()

uniform bool draw;      //should this shape be drawn?
uniform int mask;      //this this shape be masked?

bool in_shape()
{
  //code to see if gl_GlobalInvocationID.xyz is inside the shape
  vec4 mapread = texture(map,vec2(gl_GlobalInvocationID.xz/256.0f));

  if(height_color)
      drawcolor = color*texture(map, vec2(gl_GlobalInvocationID.xz/256.0f));
  else
      drawcolor = color;
  
  if(gl_GlobalInvocationID.y < (mapread.r * 256.0f * vscale))
    return true;
  else
    return false;
}

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));  //existing color value (what is the previous color?)

  if(in_shape())
  {
      // color takes on mix of previous color and draw op's color, based on existing mask value
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), draw ? mix(drawcolor, pcol, float(pmask.r)/255.) : pcol);
      //mask is set to the greater of the two mask values, between previous and the current operation
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > mask) ? pmask.r : mask));
  }
  else
  {
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(pmask.r));
  }
}
