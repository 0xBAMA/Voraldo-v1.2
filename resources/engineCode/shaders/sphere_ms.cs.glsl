#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform vec3 location;  //where is this sphere centered?
uniform float radius;   //what is the radius of this sphere?
uniform vec4 color;     //what color should it be drawn with?

uniform bool draw;      //should this shape be drawn?
uniform int mask;      //this this shape be masked?

// uniform int global_multisample; // how many samples to consider

int global_multisample = 8;
vec3 multisample_offset = vec3(0); // global scope

bool in_shape()
{
  float d = distance(gl_GlobalInvocationID.xyz+multisample_offset, location);

  if(d < radius)  //sphere defined as all points within 'radius' of the center point
    return true;
  else
    return false;
}

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));  //existing color value (what is the previous color?)

  // initially no samples have evaluated true
  float multisample_true_result_count = 0.;
  float multisample_total_weight = 0.;

  // multisampling - offsets sample locations before considering in_shape(), blends results
  for(int x = 0; x < global_multisample; x++)
  for(int y = 0; y < global_multisample; y++)
  for(int z = 0; z < global_multisample; z++)
  {
    // update multisample_offset with current values of x,y,z
    multisample_offset = vec3(float(x),float(y),float(z)) / vec3(float(global_multisample));
    if(in_shape())
    {
        multisample_true_result_count += 5./distance(multisample_offset, vec3(0.5)); // weight by distance?
    }
    multisample_total_weight += 5./distance(multisample_offset, vec3(0.5));
  }

  if(multisample_true_result_count > 0)
  {
    // ratio of hits vs misses, using global_multisample, cubed
    float ratio = float(multisample_true_result_count) / float(multisample_total_weight);

    // compute final color, mask values
    uint mask_scaled = uint(ratio * mask);
    vec4 color_scaled = vec4(ratio) * color;

    // assign as before
      // color takes on mix of previous color and draw op's color, based on existing mask value
    imageStore(current, ivec3(gl_GlobalInvocationID.xyz), draw ? mix(color_scaled, pcol, float(pmask.r)/255.) : pcol);
      //mask is set to the greater of the two mask values, between previous and the current operation
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > mask_scaled) ? pmask.r : mask_scaled));
  }
  else
  {
    // write back previous values, same as before
    imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(pmask.r));
  }
}
