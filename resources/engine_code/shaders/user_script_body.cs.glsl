// the define for the sample count and scale happens above this line (NUM + SCALE)
// -- end user code --

void main()
{
    vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));          //existing color value (what is the previous color?)
    uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous mask)

    int hitcount  = 0; // number of successful hits
    int masktotal = 0; // accumulated mask total
    vec4 acccolor = vec4(0); // accumulated

    //for(int j = 0; j < NUM; j++){
      // jitter myloc, at the user specified scale

      // call is_inside()
      irec i = is_inside();

      // add to accumulator

      // sample averaging
   // }




    // color takes on mix of previous color and draw op's color, based on mask
    imageStore(current, ivec3(gl_GlobalInvocationID.xyz), i.draw ? mix(i.color, pcol, pmask.r/255.) : pcol);
    //mask is set to the greater of the two mask values, between previous and the current operation
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > i.mask) ? pmask.r : i.mask));
}
