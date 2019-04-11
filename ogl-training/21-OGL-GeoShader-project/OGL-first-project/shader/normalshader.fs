#version 330 core
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
    float shininess;
}; 

uniform Material material;
in vec2 TexCoords; 
out vec4 FragColor;

void main()
{
	FragColor =  vec4( vec3(texture(material.texture_diffuse1,TexCoords)), 1.0);
}
