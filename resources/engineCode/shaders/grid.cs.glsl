#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform ivec3 spacing;  //distance between gridlines, xyz
uniform ivec3 offsets;  //distance between gridlines, xyz
uniform ivec3 width;    //width of grid lines, xyz

uniform vec3 rotation;  //rotate the point by the inverse amount

uniform vec4 color;     //what color should it be drawn with?

uniform bool draw;      //should this shape be drawn?
uniform int mask;      //this this shape be masked?

//thanks to Neil Mendoza via http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

bool in_shape()
{
    vec3 location = vec3(gl_GlobalInvocationID.xyz);

    location *= rotationMatrix(vec3(1,0,0), rotation.x);
    location *= rotationMatrix(vec3(0,1,0), rotation.y);
    location *= rotationMatrix(vec3(0,0,1), rotation.z);

  bool x = ((int(location.x) + offsets.x) % spacing.x) <= width.x;
  bool y = ((int(location.y) + offsets.y) % spacing.y) <= width.y;
  bool z = ((int(location.z) + offsets.z) % spacing.z) <= width.z;

  return ((x && y) || (x && z) || (y && z));
}

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));  //existing color value (what is the previous color?)

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
