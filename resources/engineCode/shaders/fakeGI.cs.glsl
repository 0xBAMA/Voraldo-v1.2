#version 430
layout(local_size_x = 8, local_size_y = 1, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(rgba16f) image3D lighting;        //values held in the lighting buffer

uniform int y_index;   // what plane are we in
uniform float scale_factor; // how much should you scale the hit cell's lighting by
uniform float alpha_thresh; // what is the minimum threshold considered a 'hit' when tracing the GI rays
uniform vec4 sky_intensity; // if the ray escapes, how much light should it get?

uniform int type; // 0 is Werness, 1 is stochastic

uint seed;
uint wang_hash(inout uint s)
{
    s = uint(s ^ uint(61)) ^ uint(s >> uint(16));
    s *= uint(9);
    s = s ^ (s >> 4);
    s *= uint(0x27d4eb2d);
    s = s ^ (s >> 15);
    return s;
}

float RandomFloat01(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * 6.283;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}



  // This has a sequential dependence, handled by how the shader is invoked - we don't need to worry about it in the shader
  // Context, from the same guy who did the Voxel Automata Terrain, Brent Werness:
  //   "Totally faked the GI!  It just casts out 9 rays in upwards facing the lattice directions.
  //    If it escapes it gets light from the sky, otherwise it gets some fraction of the light
  //    from whatever cell it hits.  Run from top to bottom and you are set!"

void main()
{

  if(type == 0){
    ivec3 my_loc = ivec3(gl_GlobalInvocationID.x, y_index, gl_GlobalInvocationID.z);
    ivec3 check_loc = ivec3(0);

    vec4 prev_light_val = imageLoad(lighting, my_loc);    //existing light value (how much light, stored in r)
    vec4 prev_color_val = imageLoad(current, my_loc);    //existing color value (what is the color?)

    vec4 new_light_val = prev_light_val;

    ivec3 imsize = imageSize(lighting)-ivec3(1);

    if(prev_color_val.a >= alpha_thresh) // this cell is opaque enough to participate
    {
        for(int dx = -1; dx <= 1; dx++)
        {
            for(int dz = -1; dz <= 1; dz++)
            {
                check_loc = ivec3(my_loc.x + dx, my_loc.y + 1, my_loc.z + dz);
                bool hit = false;

                while(check_loc.x >= 0 && check_loc.x < imsize.x && check_loc.z >= 0 && check_loc.z < imsize.z && check_loc.y < imsize.y)
                {
                    if(imageLoad(current, check_loc).a >= alpha_thresh)
                    {
                        // take some portion (determined by scale_factor) of the light from that location
                        new_light_val = new_light_val + imageLoad(lighting, check_loc) * scale_factor;

                        hit = true;
                        break;
                    }
                    check_loc.x += dx;
                    check_loc.y += 1;
                    check_loc.z += dz;
                }

                if(!hit)  // ray escaped the volume, use sky_intensity instead
                {
                    // take the light from the sky
                    new_light_val = prev_light_val + sky_intensity * sky_intensity.w;
                }
            }
        }

        imageStore(lighting, my_loc, new_light_val);
    }
  }else{
    ivec3 my_loc = ivec3(gl_GlobalInvocationID.x, y_index, gl_GlobalInvocationID.z);
    vec3 check_loc = vec3(0);

    vec4 prev_light_val = imageLoad(lighting, my_loc);    //existing light value (how much light, stored in r)
    vec4 prev_color_val = imageLoad(current, my_loc);    //existing color value (what is the color?)

    vec4 new_light_val = prev_light_val;

    ivec3 imsize = imageSize(lighting)-ivec3(1);

    if(prev_color_val.a >= alpha_thresh) // this cell is opaque enough to participate
    {
      for(int num=0; num <= type; num++) // still 9 rays
      {
        seed = seed+num;
        vec3 update_vector = RandomUnitVector(seed);

        check_loc = vec3(my_loc)+update_vector;
        bool hit = false;

        while(check_loc.x >= 0 && check_loc.x < imsize.x && check_loc.z >= 0 && check_loc.z < imsize.z && check_loc.y < imsize.y)
        {
          if(imageLoad(current, ivec3(check_loc)).a >= alpha_thresh)
          {
            // take some portion (determined by scale_factor) of the light from that location
            new_light_val = new_light_val + imageLoad(lighting, ivec3(check_loc)) * scale_factor;
            hit = true;
            break;
          }
          check_loc += ivec3(check_loc+update_vector);
        }

        if(!hit)  // ray escaped the volume, use sky_intensity instead
        {
          // take the light from the sky
          new_light_val = prev_light_val + sky_intensity * sky_intensity.w;
        }
      }
      imageStore(lighting, my_loc, new_light_val);
      // imageStore(lighting, my_loc, vec4(1., 0,0,0));
    }
  }
}
