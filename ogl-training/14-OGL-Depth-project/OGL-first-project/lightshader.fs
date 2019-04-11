#version 330 core
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
    float shininess;
}; 

struct DirectLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
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
uniform DirectLight directlight;
#define NR_POINT_LIGHTS 4 
uniform PointLight pointlights[NR_POINT_LIGHTS];
uniform SpotLight spotlight;
uniform vec3 viewPos;

out vec4 FragColor;

vec3 CalcDirLight(DirectLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	 // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result=vec3(0,0,0);
    // phase 1: Directional lighting
	result += CalcDirLight(directlight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointlights[i], norm, FragPos, viewDir);    
    // phase 3: Spot light
    result += CalcSpotLight(spotlight, norm, FragPos, viewDir);    
    // result
    FragColor = vec4(result, 1.0);
	
}

vec3 CalcDirLight(DirectLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir=normalize(light.position-fragPos);	
	//// attenuation
	float distance    = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
    				light.quadratic * (distance * distance)); 
	// ambient
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,TexCoords));

	//Spot Cone
	float theta = dot(lightDir,normalize(-light.direction));
	if(theta>light.outercutOff)
	{
		//diffuse
		float diff = max(dot(lightDir,normal),0);
		vec3 diffuse = diff * light.diffuse *  vec3(texture(material.texture_diffuse1,TexCoords));
		//specular
		vec3 reflectDir = reflect(-lightDir, normal);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular =  spec * light.specular *vec3(texture(material.texture_specular1,TexCoords)); 
		//inner Cone
		if(theta>light.innercutOff)
		{
			return attenuation * (ambient + diffuse+specular);
		}
		//outer Cone
		else{
			float intensity = (theta - light.outercutOff)/(light.innercutOff-light.outercutOff);
			return attenuation * (ambient + intensity*diffuse+intensity*specular);
		}
	}
	else{
		return attenuation * ambient;
	}
}