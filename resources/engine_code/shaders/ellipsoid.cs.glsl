#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform vec3 center;          //xyz of center
uniform vec3 radii;           //allows for 3 distinct radii
uniform vec3 rotation;        //rotation on x,y,z - going to need to think about what this looks like

uniform vec4 color;           //what color should it be drawn with?

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
  //subtract center.xyz from gl_GlobalInvocationID.xyz
  vec3 local = gl_GlobalInvocationID.xyz - center.xyz;

  //rotate the result, using the rotation vector - this is inverted
  local *= rotationMatrix(vec3(1,0,0), -rotation.x);
  local *= rotationMatrix(vec3(0,1,0), -rotation.y);
  local *= rotationMatrix(vec3(0,0,1), -rotation.z);

  //test the result of the rotation, against the forumula for an ellipsoid:
  //   x^2 / a^2  +  y^2 / b^2  +  z^2 / c^2  =  1    (<= to one for our purposes)
  //where x, y and z are the components of the result of the rotation
  //and a, b and c are the x,y and z radii components
  float result = pow(local.x, 2) / pow(radii.x,2) + pow(local.y, 2) / pow(radii.y,2) + pow(local.z, 2) / pow(radii.z,2);

  if(result <= 1)
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
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), draw ? mix(color, pcol, float(pmask.r)/255.) : pcol);
      //mask is set to the greater of the two mask values, between previous and the current operation
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > mask) ? pmask.r : mask, 0,0,0));
  }
  else
  {
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(pmask.r));
  }
}
