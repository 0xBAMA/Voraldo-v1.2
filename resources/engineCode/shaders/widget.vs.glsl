#version 430

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 normal;
out vec3 color;
out vec3 vpos;

uniform float theta;
uniform float phi;
uniform float ratio;

uniform vec3 basis_x;
uniform vec3 basis_y;
uniform vec3 basis_z;

uniform vec3 offset;


void main()
{
	normal = basis_x * vNormal.x + basis_y * vNormal.y + basis_z * vNormal.z;
	color = vColor;
	mat3 rot = -mat3(basis_x.x, basis_x.y, basis_x.z,
					basis_y.x, basis_y.y, basis_y.z,
					-basis_z.x, -basis_z.y, -basis_z.z);

	// vpos = 0.2 * (basis_x * vPosition.x + basis_y * vPosition.y + basis_z * vPosition.z);
	vpos = 0.2 * rot * vPosition;

	vpos.x /= ratio;
	vpos += offset;

	gl_Position = vec4(vpos, 1.0);
}
