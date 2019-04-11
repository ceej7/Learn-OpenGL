#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aCoord;
layout(location = 3) in mat4 aModel;

out VS_OUT
{
	vec3 Normal;
	vec2 TexCoords;
}vs_out;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection*view*aModel*vec4(aPos, 1.0);
	vs_out.Normal =  mat3(transpose(inverse(aModel)))* aNormal;
	vs_out.TexCoords=aCoord;
}