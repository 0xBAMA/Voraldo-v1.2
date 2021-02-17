
// -- end user code --

void main()
{
    vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));          //existing color value (what is the previous color?)
    uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous mask)

    // call is_inside()
    irec i = is_inside();

    if(i.draw)
    {
        vec4 temp = vec4(0);
        temp.r = mix(i.color.r, pcol.r, pmask.r/255.);
        temp.g = mix(i.color.g, pcol.g, pmask.g/255.);
        temp.b = mix(i.color.b, pcol.b, pmask.b/255.);
        temp.a = mix(i.color.a, pcol.a, pmask.a/255.);

        // color takes on mix of previous color and draw op's color, based on mask
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), temp);
        //mask is set to the greater of the two mask values, between previous and the current operation
        imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > i.mask) ? pmask.r : i.mask));
    }
    else
    {
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
        imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > i.mask) ? pmask.r : i.mask));
    }
}
