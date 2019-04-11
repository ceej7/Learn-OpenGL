#version 330 core

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_height1;
    float shininess;
}; 

in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;

float near = 0.1; 
float far  = 50.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
	FragColor = vec4( vec3(texture(material.texture_diffuse1,TexCoords)), 1.0);
	//float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}
