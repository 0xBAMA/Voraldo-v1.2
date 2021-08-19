#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;    //specifies the workgroup size

double tmin, tmax; //global scope, set in hit() to tell min and max parameters

// #define NUM_STEPS 2000
//#define NUM_STEPS 165

// #define NUM_STEPS 500
#define NUM_STEPS 780
#define MIN_DISTANCE 0.0
#define MAX_DISTANCE 5.0


// the display texture
uniform layout(rgba16f) image2D current; // we can get the dimensions with imageSize

// samplers
uniform sampler3D block;
uniform sampler3D lighting;

// because this is going to have to be tile-based, we need this local offset
uniform int x_offset;
uniform int y_offset;

uniform float clickndragx;
uniform float clickndragy;

//gl_GlobalInvocationID will define the tile size, so doing anything to define it here would be redundant
// this shader is general up to tile sizes of 2048x2048, since those are the maximum dispatch values

uniform vec3 basis_x;
uniform vec3 basis_y;
uniform vec3 basis_z;

uniform vec4 clear_color;

uniform float scale;
uniform float perspfactor;

uniform float upow;

// TONEMAPPING MODE
uniform int ACES_behavior;

// APPROX
// --------------------------
vec3 cheapo_aces_approx(vec3 v)
{
	v *= 0.6f;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}


// OFFICIAL
// --------------------------
mat3 aces_input_matrix = mat3(
	0.59719f, 0.35458f, 0.04823f,
	0.07600f, 0.90834f, 0.01566f,
	0.02840f, 0.13383f, 0.83777f
);

mat3 aces_output_matrix = mat3(
	1.60475f, -0.53108f, -0.07367f,
	-0.10208f,  1.10813f, -0.00605f,
	-0.00327f, -0.07276f,  1.07602f
);

vec3 mul(mat3 m, vec3 v)
{
	float x = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
	float y = m[1][0] * v[1] + m[1][1] * v[1] + m[1][2] * v[2];
	float z = m[2][0] * v[1] + m[2][1] * v[1] + m[2][2] * v[2];
	return vec3(x, y, z);
}

vec3 rtt_and_odt_fit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 aces_fitted(vec3 v)
{
	v = mul(aces_input_matrix, v);
	v = rtt_and_odt_fit(v);
	return mul(aces_output_matrix, v);
}


bool hit(vec3 org, vec3 dir)
{
    // hit() code adapted from:
    //
    //    Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
    //    "An Efficient and Robust Ray-Box Intersection Algorithm"
    //    Journal of graphics tools, 10(1):49-54, 2005

    //the bounding box
    vec3 min = vec3(-1,-1,-1);
    vec3 max = vec3(1,1,1);

    int sign[3];

    vec3 inv_direction = vec3(1/dir.x, 1/dir.y, 1/dir.z);

    sign[0] = (inv_direction[0] < 0)?1:0;
    sign[1] = (inv_direction[1] < 0)?1:0;
    sign[2] = (inv_direction[2] < 0)?1:0;

    vec3 bbox[2] = {min,max};

    tmin = (bbox[sign[0]][0] - org[0]) * inv_direction[0];
    tmax = (bbox[1-sign[0]][0] - org[0]) * inv_direction[0];

    double tymin = (bbox[sign[1]][1] - org[1]) * inv_direction[1];
    double tymax = (bbox[1-sign[1]][1] - org[1]) * inv_direction[1];

    if ( (tmin > tymax) || (tymin > tmax) )
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    double tzmin = (bbox[sign[2]][2] - org[2]) * inv_direction[2];
    double tzmax = (bbox[1-sign[2]][2] - org[2]) * inv_direction[2];

    if ( (tmin > tzmax) || (tzmin > tmax) )
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;
    return ( (tmin < MAX_DISTANCE) && (tmax > MIN_DISTANCE) );

    return true;
}

vec4 get_color_for_pixel(vec3 org, vec3 dir)
{
    float current_t = float(tmax);
    vec4 t_color = clear_color;

    float step = float((tmax-tmin))/NUM_STEPS;
    if(step < 0.001f)
        step = 0.001f;

    vec3 samp = (org + current_t * dir + vec3(1.))/2.;
    vec4 new_read = texture(block, samp);
    // vec4 new_light_read = texture(lighting, samp);

    // ok I only want to use that alpha, the rgb comes from a depth-wise lerp between red and blue
    // vec3 col = mix(vec3(1.), vec3(0.), distance(org, org+current_t*dir) / float(4));
    // new_read.rgb = col;
    new_read.rgb = samp;


    float alpha_squared;

    for(int i = 0; i < NUM_STEPS; i++)
    {   if(current_t>=tmin)
        {
            //apply the lighting scaling
            // new_read.rgb *= new_light_read.rgb;

            // parameterizing the alpha power
            alpha_squared = pow(new_read.a, upow);

            // a over b, where a is the new sample and b is the current color, t_color
            t_color.rgb = new_read.rgb * alpha_squared + t_color.rgb * t_color.a * ( 1. - alpha_squared );
            t_color.a = alpha_squared + t_color.a * ( 1. - alpha_squared );

            // take a step
            current_t -= step;

            // new location
            samp = (org + current_t * dir + vec3(1.))/2.;

            // take a sample
            new_read = texture(block, samp);
            // col = mix(vec3(1.), vec3(0.), distance(org, org+current_t*dir) / float(4));
            // new_read.rgb = col;
            new_read.rgb = samp;

            // new_light_read = texture(lighting, samp);
        }
    }
    return t_color;
}

void main()
{
    ivec2 Global_Loc = ivec2(gl_GlobalInvocationID.xy) + ivec2(x_offset+int(clickndragx), y_offset+int(clickndragy));
    ivec2 dimensions = ivec2(imageSize(current));

    float aspect_ratio = float(dimensions.y) / float(dimensions.x);

    float x_start = scale*((Global_Loc.x/float(dimensions.x)) - 0.5);
    float y_start = scale*((Global_Loc.y/float(dimensions.y)) - 0.5)*(aspect_ratio);

    mat3 rot = inverse(mat3(basis_x.x, basis_x.y, basis_x.z,
                            basis_y.x, basis_y.y, basis_y.z,
                            basis_z.x, basis_z.y, basis_z.z));

    //start with a vector pointing down the z axis (greater than half the corner to corner distance, i.e. > ~1.75)
    vec3 org = rot * vec3(-x_start, -y_start,  2.); //add the offsets in x and y
    // vec3 dir = rot * vec3(       0,        0, -2.); //simply a vector pointing in the opposite direction, no xy offsets
    vec3 dir = rot * vec3( -perspfactor*x_start, -perspfactor*y_start, -2.);  // perspective projection


    Global_Loc -= ivec2(clickndragx, clickndragy);

    if(Global_Loc.x < dimensions.x && Global_Loc.y < dimensions.y)  // we are good to check the ray against the AABB
    {   vec4 color;
        if(hit(org,dir))
        {
            color = get_color_for_pixel(org, dir);
        }
        else
        {
            color = clear_color;
        }

        // tonemapping
        switch(ACES_behavior)
        {
            case 0: // no tonemapping
                break;
            case 1: // cheap version
                color.xyz = cheapo_aces_approx(color.xyz);
                break;
            case 2: // full version
                color.xyz = aces_fitted(color.xyz);
                break;
        }
        // store final result
        imageStore(current, Global_Loc, color);
    }  // else, this part of the tile falls outside of the image bounds, no operation should take place
}
