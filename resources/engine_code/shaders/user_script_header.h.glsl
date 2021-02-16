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
//  uniform layout(rgba8) image3D current;
uniform sampler3D current;
uniform layout(rgba8) image3D lighting;

// need current and previous color buffers
// need current and previous mask buffers
// lighting? don't think that has much utility
