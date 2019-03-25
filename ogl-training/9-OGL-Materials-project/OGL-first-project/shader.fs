#version 330 core
out vec4 FragColor;


struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Material material;
uniform Light light;  
uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;

void main()
{
	vec3 ambient = light.ambient * material.ambient;

	vec3 norm=normalize(Normal);
	vec3 lightDir=normalize(light.position-FragPos);
	vec3 diffuse = max(dot(lightDir,norm),0) * light.diffuse * material.diffuse;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular =  spec * light.specular *material.specular ;  

	vec3 result = ambient + diffuse+specular;
	FragColor = vec4(result, 1.0);
}
