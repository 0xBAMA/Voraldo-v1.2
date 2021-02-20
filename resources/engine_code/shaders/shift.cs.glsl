#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform layout(rgba8) image3D lighting;  //wanted to make the lighting buffer shift with the
// color buffer, but to do that I'm going to need a second lighting buffer

uniform ivec3 movement;     //how much are you moving this current cell by? 
uniform bool loop;          //does the data loop off the sides (toroid style)
uniform int mode;           //will you respect the current value of the mask? this could get ambiguous but we'll deal

void main()
{
    ivec3 regular_pos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 shifted_pos = regular_pos - movement;

    ivec3 image_size = imageSize(current);

    if(loop)
    {
        shifted_pos.x = shifted_pos.x % image_size.x;
        shifted_pos.y = shifted_pos.y % image_size.y;
        shifted_pos.z = shifted_pos.z % image_size.z;
    }

  uvec4 pmask = imageLoad(previous_mask, regular_pos);  //existing mask value (previous_mask = 0?)
  uvec4 psmask = imageLoad(previous_mask, shifted_pos);  //existing mask value (shifted)
  
  vec4 plight = imageLoad(lighting, regular_pos);
  vec4 pslight = imageLoad(lighting, shifted_pos);
  
  vec4 pcol = imageLoad(previous, regular_pos);    //existing color value (what is the previous color?)
  vec4 pscol = imageLoad(previous, shifted_pos);   //existing color value (shifted)
  
  

    if(mode == 1)       //ignore mask buffer, move color and light data only (current_mask takes value of previous_mask)
    {
        // do the color shift
        imageStore(current, regular_pos, pscol);
        
        // do the light shift
        //imageStore(lighting, regular_pos, pslight);
        
        //write the same value of mask back to current_mask
        imageStore(current_mask, regular_pos, pmask);
    }
    else if(mode == 2)  //respect mask buffer, if pmask is true, current takes value of previous, if false, do the shift
    {
        imageStore(current, regular_pos, mix(pscol, pcol, pmask));
        imageStore(current_mask, regular_pos, pmask);
    }
    else if(mode == 3)  //carry mask buffer, mask comes along for the ride with the color values
    {
        //do the color shift
        imageStore(current, regular_pos, pscol);
       
        //do the mask shift
        imageStore(current_mask, regular_pos, psmask);
    }
}
