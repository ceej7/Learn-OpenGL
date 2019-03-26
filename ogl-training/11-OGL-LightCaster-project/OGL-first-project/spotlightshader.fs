#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
struct Light {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
    float linear;
    float quadratic;
	float innercutOff;
	float outercutOff;
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
	float distance    = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
    				light.quadratic * (distance * distance)); 
	
	vec3 ambient = light.ambient * vec3(texture(material.diffuse,TexCoords));

	vec3 norm=normalize(Normal);
	vec3 lightDir=normalize(light.position-FragPos);
	
	float theta = dot(lightDir,normalize(-light.direction));
	if(theta>light.innercutOff)
	{
		float diff = max(dot(lightDir,norm),0);
		vec3 diffuse = diff * light.diffuse *  vec3(texture(material.diffuse,TexCoords));

		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular =  spec * light.specular *vec3(texture(material.specular,TexCoords)); 

		vec3 result = attenuation * (ambient + diffuse+specular);
		FragColor = vec4(result, 1.0);
	}
	else if(theta>light.outercutOff)
	{
		float intensity = (theta - light.outercutOff)/(light.innercutOff-light.outercutOff);
		float diff = max(dot(lightDir,norm),0);
		vec3 diffuse = diff * light.diffuse *  vec3(texture(material.diffuse,TexCoords));

		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular =  spec * light.specular *vec3(texture(material.specular,TexCoords)); 

		vec3 result = attenuation * (ambient + intensity*diffuse+intensity*specular);
		FragColor = vec4(result, 1.0);
	}
	else{
		FragColor = vec4(ambient, 1.0);
	}
	
}
