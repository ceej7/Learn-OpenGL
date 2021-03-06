#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;


uniform Material material;
uniform Light light;  
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
	vec3 ambient = light.ambient * vec3(texture(material.diffuse,TexCoords));

	vec3 norm=normalize(Normal);
	vec3 lightDir=normalize(light.position-FragPos);
	float diff = max(dot(lightDir,norm),0);
	vec3 diffuse = diff * light.diffuse *  vec3(texture(material.diffuse,TexCoords));

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  

	//bling
	vec3 halfwayDir = normalize(lightDir+viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
	vec3 specular =  spec * light.specular *vec3(texture(material.specular,TexCoords)); 

	vec3 result = ambient + diffuse+specular;
	FragColor = vec4(result, 1.0);
}
