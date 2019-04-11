#version 330 core

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
    float shininess;
}; 

in VS_OUT
{
	vec3 Normal;
	vec2 TexCoords;
}gs_in;

out vec4 FragColor;

uniform Material material;

void main()
{
	FragColor = vec4( vec3(texture(material.texture_diffuse1,gs_in.TexCoords)), 1.0);
}
