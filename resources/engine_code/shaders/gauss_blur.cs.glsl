#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform int radius;
uniform bool respect_mask;
uniform bool touch_alpha;

void main()
{
    uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value (previous_mask = 0?)
    vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));                 //existing color value (what is the previous color?)

    float num = 0.0;
    float numT = 0.0;

    float weight;
    float max_dist = distance(vec3(0), vec3(radius));

    vec4 csum = vec4(0,0,0,0);

    for(int x = (-1 * radius); x <= radius; x++)
        for(int y = (-1 * radius); y <= radius; y++)
            for(int z = (-1 * radius); z <= radius; z++)
            {
                weight = 1-(distance(vec3(0), vec3(x, y, z)));
                csum += weight*imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz) + ivec3(x,y,z));
                num  += weight;
            }

    //divide csum and msum by num
    csum /= num;

    imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), pmask);

    if(touch_alpha)
    { //alpha will change to the average of the neighboring cells
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), respect_mask ? mix(csum, pcol, float(pmask.r)/255.) : csum);
    }
    else
    { //don't touch alpha, get the value from pcol
        imageStore(current, ivec3(gl_GlobalInvocationID.xyz), vec4(respect_mask ? mix(csum.xyz, pcol.xyz, float(pmask.r)/255.) : csum.xyz, pcol.a));
    }

}
