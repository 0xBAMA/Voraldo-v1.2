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

// position in the cube spanning -1 to 1 on each axis, I guess can use lighting here
vec3 myloc = (vec3(gl_GlobalInvocationID.xyz) + vec3(0.5) - vec3(imageSize(lighting).x)/2.) / (vec3(imageSize(lighting))/2.);

// the intersection record
struct irec{
    bool draw;
    vec4 color;
    int mask;
};



// point rotation about an arbitrary axis, ax - from gaz
vec3 erot(vec3 p, vec3 ax, float ro) {
    return mix(dot(p,ax)*ax,p,cos(ro))+sin(ro)*cross(ax,p);
}


// -- begin user code --
