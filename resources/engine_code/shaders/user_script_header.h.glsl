#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; //3d workgroup

// what the header should provide:
//   primitives
//   operators - folding
//   easing functions
//   color space conversions
//   palette functions
//   noise
//   hashing
//   ... ?

// the shader environment
// need current and previous color buffers
uniform layout(rgba8) image3D current;
uniform layout(rgba8) image3D previous;

// need current and previous mask buffers
uniform layout(r8ui) uimage3D current_mask;
uniform layout(r8ui) uimage3D previous_mask;

// lighting? don't think having access to this has much utility
uniform layout(rgba8) image3D lighting;

uniform sampler2D dither; // blue noise texture
uniform int ditherdim;

vec4 blue(ivec2 samploc) {
	samploc.x = samploc.x % ditherdim;
	samploc.y = samploc.y % ditherdim;
	return texture(dither, vec2(samploc)/vec2(ditherdim, ditherdim));
}



// position in the cube spanning -1 to 1 on each axis, I guess can use lighting here
vec3 myloc = (vec3(gl_GlobalInvocationID.xyz) + vec3(0.5) - vec3(imageSize(lighting).x)/2.) / (vec3(imageSize(lighting))/2.);
float off  = 2./imageSize(lighting).x;

// the intersection record
struct irec{
    bool draw;
    vec4 color;
    int mask;
};


// point rotation about an arbitrary axis, ax - from gaziya5
vec3 erot(vec3 p, vec3 ax, float ro) {
    return mix(dot(p,ax)*ax,p,cos(ro))+sin(ro)*cross(ax,p);
}

// smooth minimum
float smin(float a, float b, float k) {
    float h = max(0.,k-abs(b-a))/k;
    return min(a,b)-h*h*h*k/6.;
}

// from michael0884's marble marcher community edition
void planeFold(inout vec3 z, vec3 n, float d) {
    z.xyz -= 2.0 * min(0.0, dot(z.xyz, n) - d) * n;
}

void sierpinskiFold(inout vec3 z) {
    z.xy -= min(z.x + z.y, 0.0);
    z.xz -= min(z.x + z.z, 0.0);
    z.yz -= min(z.y + z.z, 0.0);
}

void mengerFold(inout vec3 z)
{
    z.xy += min(z.x - z.y, 0.0)*vec2(-1.,1.);
    z.xz += min(z.x - z.z, 0.0)*vec2(-1.,1.);
    z.yz += min(z.y - z.z, 0.0)*vec2(-1.,1.);
}

void boxFold(inout vec3 z, vec3 r) {
    z.xyz = clamp(z.xyz, -r, r) * 2.0 - z.xyz;
}



// from a distance estimated fractal by discord user Nameless#1608
// array repetition
#define pmod(p,a) mod(p - 0.5*a,a) - 0.5*a

// another fold
void sphereFold(inout vec3 z) {
    float minRadius2 = 0.25;
    float fixedRadius2 = 2.;
    float r2 = dot(z,z);
    if (r2 < minRadius2) {
        float temp = (fixedRadius2/minRadius2);
        z*= temp;
    } else if (r2 < fixedRadius2) {
        float temp =(fixedRadius2/r2);
        z*=temp;
    }
}




// -- begin user code --
