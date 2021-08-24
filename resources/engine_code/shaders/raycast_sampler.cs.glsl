#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;    //specifies the workgroup size

double tmin, tmax; //global scope, set in hit() to tell min and max parameters

#define MIN_DISTANCE 0.0
#define MAX_DISTANCE 5.0

// max number of steps along the ray
uniform int num_steps;


// the display texture
uniform layout(rgba16f) image2D current; // we can get the dimensions with imageSize
uniform sampler2D dither; // blue noise texture
uniform int ditherdim;
uniform int frame;


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


vec3 uncharted2(vec3 v)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    float ExposureBias = 2.0f;
    v *= ExposureBias;

    return (((v*(A*v+C*B)+D*E)/(v*(A*v+B)+D*F))-E/F)*(((W*(A*W+C*B)+D*E)/(W*(A*W+B)+D*F))-E/F);
}

vec3 rienhard(vec3 v)
{
    return v / (vec3(1.) + v);
}

vec3 rienhard2(vec3 v)
{
    const float L_white = 4.0;
    return (v * (vec3(1.) + v / (L_white * L_white))) / (1.0 + v);
}

vec3 tonemap_uchimura(vec3 v)
{
    const float P = 1.0;  // max display brightness
    const float a = 1.0;  // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4;  // linear section length
    const float c = 1.33; // black
    const float b = 0.0;  // pedestal

    // Uchimura 2017, "HDR theory and practice"
    // Math: https://www.desmos.com/calculator/gslcdxvipg
    // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    vec3 w0 = 1.0 - smoothstep(0.0, m, v);
    vec3 w2 = step(m + l0, v);
    vec3 w1 = 1.0 - w0 - w2;

    vec3 T = m * pow(v / m, vec3(c)) + vec3(b);
    vec3 S = P - (P - S1) * exp(CP * (v - S0));
    vec3 L = m + a * (v - vec3(m));

    return T * w0 + L * w1 + S * w2;
}

vec3 tonemap_uchimura2(vec3 v)
{
    const float P = 1.0;  // max display brightness
    const float a = 1.7;  // contrast
    const float m = 0.1; // linear section start
    const float l = 0.0;  // linear section length
    const float c = 1.33; // black
    const float b = 0.0;  // pedestal

    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    vec3 w0 = 1.0 - smoothstep(0.0, m, v);
    vec3 w2 = step(m + l0, v);
    vec3 w1 = 1.0 - w0 - w2;

    vec3 T = m * pow(v / m, vec3(c)) + vec3(b);
    vec3 S = P - (P - S1) * exp(CP * (v - S0));
    vec3 L = m + a * (v - vec3(m));

    return T * w0 + L * w1 + S * w2;
}

vec3 tonemap_unreal3(vec3 v)
{
    return v / (v + 0.155) * 1.019;
}


#define toLum(color) dot(color, vec3(.2125, .7154, .0721) )
#define lightAjust(a,b) ((1.-b)*(pow(1.-a,vec3(b+1.))-1.)+a)/b
#define reinhard(c,l) c * (l / (1. + l) / l)
vec3 jt_toneMap(vec3 x){
    float l = toLum(x);
    x = reinhard(x,l);
    float m = max(x.r,max(x.g,x.b));
    return min(lightAjust(x/m,m),x);
}
#undef toLum
#undef lightAjust
#undef reinhard


vec3 robobo1221sTonemap(vec3 x){
	return sqrt(x / (x + 1.0f / x)) - abs(x) + x;
}

vec3 roboTonemap(vec3 c){
    return c/sqrt(1.+c*c);
}

vec3 jodieRoboTonemap(vec3 c){
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc=c/sqrt(c*c+1.);
    return mix(c/sqrt(l*l+1.),tc,tc);
}

vec3 jodieRobo2ElectricBoogaloo(const vec3 color){
    float luma = dot(color, vec3(.2126, .7152, .0722));

    // tonemap curve goes on this line
    // (I used robo here)
    vec4 rgbl = vec4(color, luma) * inversesqrt(luma*luma + 1.);

    vec3 mappedColor = rgbl.rgb;
    float mappedLuma = rgbl.a;

    float channelMax = max(max(max(
    	mappedColor.r,
    	mappedColor.g),
    	mappedColor.b),
    	1.);

    // this is just the simplified/optimised math
    // of the more human readable version below
    return (
        (mappedLuma*mappedColor-mappedColor)-
        (channelMax*mappedLuma-mappedLuma)
    )/(mappedLuma-channelMax);

    const vec3 white = vec3(1);

    // prevent clipping
    vec3 clampedColor = mappedColor/channelMax;

    // x is how much white needs to be mixed with
    // clampedColor so that its luma equals the
    // mapped luma
    //
    // mix(mappedLuma/channelMax,1.,x) = mappedLuma;
    //
    // mix is defined as
    // x*(1-a)+y*a
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/mix.xhtml
    //
    // (mappedLuma/channelMax)*(1.-x)+1.*x = mappedLuma

    float x = (mappedLuma - mappedLuma*channelMax)
        /(mappedLuma - channelMax);
    return mix(clampedColor, white, x);
}

vec3 jodieReinhardTonemap(vec3 c){
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc=c/(c+1.);
    return mix(c/(l+1.),tc,tc);
}

vec3 jodieReinhard2ElectricBoogaloo(const vec3 color){
    float luma = dot(color, vec3(.2126, .7152, .0722));

    // tonemap curve goes on this line
    // (I used reinhard here)
    vec4 rgbl = vec4(color, luma) / (luma + 1.);

    vec3 mappedColor = rgbl.rgb;
    float mappedLuma = rgbl.a;

    float channelMax = max(max(max(
    	mappedColor.r,
    	mappedColor.g),
    	mappedColor.b),
    	1.);

    // this is just the simplified/optimised math
    // of the more human readable version below
    return (
        (mappedLuma*mappedColor-mappedColor)-
        (channelMax*mappedLuma-mappedLuma)
    )/(mappedLuma-channelMax);

    const vec3 white = vec3(1);

    // prevent clipping
    vec3 clampedColor = mappedColor/channelMax;

    // x is how much white needs to be mixed with
    // clampedColor so that its luma equals the
    // mapped luma
    //
    // mix(mappedLuma/channelMax,1.,x) = mappedLuma;
    //
    // mix is defined as
    // x*(1-a)+y*a
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/mix.xhtml
    //
    // (mappedLuma/channelMax)*(1.-x)+1.*x = mappedLuma

    float x = (mappedLuma - mappedLuma*channelMax)
        /(mappedLuma - channelMax);
    return mix(clampedColor, white, x);
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

vec4 blue(ivec2 samploc) {
	samploc.x = samploc.x % ditherdim;
	samploc.y = samploc.y % ditherdim;
	return texture(dither, vec2(samploc)/vec2(ditherdim, ditherdim));
}


vec4 get_color_for_pixel(vec3 org, vec3 dir)
{
    float current_t = float(tmax);
    vec4 t_color = clear_color;

    float step = float((tmax-tmin))/num_steps;
    // if(step < 1./textureSize(block, 0).x)
    //    step = 1./textureSize(block, 0).x;

	  // if(step < 0.001f)
        // step = 0.001f;

    vec3 samp = (org + current_t * dir + vec3(1.))/2.;
    vec4 new_read = texture(block, samp);
    vec4 new_light_read = texture(lighting, samp);

    float alpha_squared;
	 float offset = 0.;

    for(int i = 0; i < num_steps; i++)
    {   if(current_t>=tmin)
        {
            //apply the lighting scaling
            new_read.rgb *= new_light_read.rgb;

            // parameterizing the alpha power
            alpha_squared = pow(new_read.a, upow);

            // a over b, where a is the new sample and b is the current color, t_color
            t_color.rgb = new_read.rgb * alpha_squared + t_color.rgb * t_color.a * ( 1. - alpha_squared );
            t_color.a = alpha_squared + t_color.a * ( 1. - alpha_squared );

            // take a step
            current_t -= step;

            // new location
            samp = (org + (current_t + offset) * dir + vec3(1.))/2.;

            // take a sample
            new_read = texture(block, samp);
            new_light_read = texture(lighting, samp);

				// update the offset value, skipping on first iteration
				offset = (blue(ivec2(i, frame)).r-0.5) * step * 0.618033; // golden ratio factor idk?
        }
    }
    return t_color;
}

void main() {
    ivec2 Global_Loc = ivec2(gl_GlobalInvocationID.xy) + ivec2(x_offset+int(clickndragx), y_offset+int(clickndragy));
    ivec2 dimensions = ivec2(imageSize(current));

	 ivec2 samploc = (Global_Loc+ivec2(frame)).xy;
	 vec4 bluenoisesamp = blue(samploc);


	 float aspect_ratio = float(dimensions.y) / float(dimensions.x);

    float x_start = scale*(((float(Global_Loc.x)+bluenoisesamp.x)/float(dimensions.x)) - 0.5);
    float y_start = scale*(((float(Global_Loc.y)+bluenoisesamp.y)/float(dimensions.y)) - 0.5)*(aspect_ratio);

    mat3 rot = inverse(mat3(basis_x.x, basis_x.y, basis_x.z,
                            basis_y.x, basis_y.y, basis_y.z,
                            basis_z.x, basis_z.y, basis_z.z));


    //start with a vector pointing down the z axis (greater than half the corner to corner distance, i.e. > ~1.75)
    vec3 org = rot * vec3(-x_start, -y_start,  2.+0.1*bluenoisesamp.z); //add the offsets in x and y
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
            case 0: // None (Linear)
                break;
            case 1: // ACES (Narkowicz 2015)
                color.xyz = cheapo_aces_approx(color.xyz);
                break;
            case 2: // Unreal Engine 3
                color.xyz = pow(tonemap_unreal3(color.xyz), vec3(2.8));
                break;
            case 3: // Unreal Engine 4
                color.xyz = aces_fitted(color.xyz);
                break;
            case 4: // Uncharted 2
                color.xyz = uncharted2(color.xyz);
                break;
            case 5: // Gran Turismo
                color.xyz = tonemap_uchimura(color.xyz);
                break;
            case 6: // Modified Gran Turismo
                color.xyz = tonemap_uchimura2(color.xyz);
                break;
            case 7: // Rienhard
                color.xyz = rienhard(color.xyz);
                break;
            case 8: // Modified Rienhard
                color.xyz = rienhard2(color.xyz);
                break;
            case 9: // jt_tonemap
                color.xyz = jt_toneMap(color.xyz);
                break;
            case 10: // robobo1221s
                color.xyz = robobo1221sTonemap(color.xyz);
                break;
            case 11: // robo
                color.xyz = roboTonemap(color.xyz);
                break;
            case 12: // jodieRobo
                color.xyz = jodieRoboTonemap(color.xyz);
                break;
            case 13: // jodieRobo2
                color.xyz = jodieRobo2ElectricBoogaloo(color.xyz);
                break;
            case 14: // jodieReinhard
                color.xyz = jodieReinhardTonemap(color.xyz);
                break;
            case 15: // jodieReinhard2
                color.xyz = jodieReinhard2ElectricBoogaloo(color.xyz);
                break;
        }

		  // grab the result from the previous frame
		  color = mix(color, imageLoad(current, Global_Loc), 0.6+0.1*bluenoisesamp.a);

        // store the final result
        imageStore(current, Global_Loc, color);

    }  // else, this part of the tile falls outside of the image bounds, no operation should take place
}
