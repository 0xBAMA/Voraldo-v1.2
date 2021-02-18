
// -- end user code --

void main()
{
    vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));          //existing color value (what is the previous color?)
    uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous mask)

    // call is_inside()
    irec i = is_inside();

    if(i.draw)
    {
        // color takes on mix of previous color and draw op's color, based on mask
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), mix(i.color, pcol, pmask/255.));
    }
    else
    {
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
    }

    //mask is set to the greater of the two mask values, between previous and the current operation
    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > i.mask) ? pmask.r : i.mask));
}
