#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D previous;       //now-current values of the block
uniform layout(r8ui) uimage3D previous_mask;  //now-current values of the mask

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8ui) uimage3D current_mask;   //values of the mask after the update

uniform vec3 tvec;            //location of center of top
uniform vec3 bvec;            //location of center of bottom
uniform float radius;         //how thick is this cylinder?

uniform vec4 color;           //what color should it be drawn with?

uniform bool draw;      //should this shape be drawn?
uniform int mask;      //this this shape be masked?


bool planetest(vec3 plane_point, vec3 plane_normal, vec3 test_point)
{
	// Determines whether a point is above or below a plane

  //		return false if the point is above the plane
	//		return true if the point is below the plane

	float result = 0.0;
	//equation of plane
	// a (x-x1) + b (y-y1) + c (z-z1) = 0

	float a = plane_normal.x;
	float b = plane_normal.y;
	float c = plane_normal.z;

	float x1 = plane_point.x;
	float y1 = plane_point.y;
	float z1 = plane_point.z;

	float x = test_point.x;
	float y = test_point.y;
	float z = test_point.z;

	result = a * (x - x1) + b * (y - y1) + c * (z - z1);

	return (result <= 0) ? true : false;

}

bool in_shape()
{
  //code to see if gl_GlobalInvocationID.xyz is inside the shape

  vec3 cylinder_center = ( bvec + tvec ) / 2.0f;

  vec3 cylinder_tvec_normal = bvec - tvec;
  cylinder_tvec_normal = planetest( tvec, cylinder_tvec_normal, cylinder_center) ? cylinder_tvec_normal : (cylinder_tvec_normal * -1.0f);

  vec3 cylinder_bvec_normal = bvec - tvec;
  cylinder_bvec_normal = planetest( bvec, cylinder_bvec_normal, cylinder_center) ? cylinder_bvec_normal : (cylinder_bvec_normal * -1.0f);


  if( planetest(bvec, cylinder_bvec_normal, gl_GlobalInvocationID.xyz) && planetest(tvec, cylinder_tvec_normal, gl_GlobalInvocationID.xyz) )
  {
    if((length( cross( tvec - bvec, bvec - gl_GlobalInvocationID.xyz ) ) / length( tvec - bvec )) < radius)
    {
      //distance from point to line from http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
      return true;
    }
  }

  return false;
}

void main()
{
  uvec4 pmask = imageLoad(previous_mask, ivec3(gl_GlobalInvocationID.xyz));  //existing mask value
  vec4 pcol = imageLoad(previous, ivec3(gl_GlobalInvocationID.xyz));  //existing color value (what is the previous color?)

  if(in_shape())
  {
      // color takes on mix of previous color and draw op's color, based on existing mask value
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), draw ? mix(color, pcol, float(pmask.r)/255.) : pcol);
      //mask is set to the greater of the two mask values, between previous and the current operation
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4((pmask.r > mask) ? pmask.r : mask, 0,0,0));
  }
  else
  {
      imageStore(current, ivec3(gl_GlobalInvocationID.xyz), pcol);
      imageStore(current_mask, ivec3(gl_GlobalInvocationID.xyz), uvec4(pmask.r));
  }
}
