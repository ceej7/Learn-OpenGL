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

in VS_OUT{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform Material material;
uniform Light light;  
uniform vec3 viewPos;
uniform sampler2D depthMap;
out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace,float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	if(currentDepth>1.0)
	{
		return 0.0;
	}
	else{
		// check whether current frag pos is in shadow
		float shadow = 0.0;
		vec2 texelSize =1.0/ textureSize(depthMap,0);
		int cnt=3;
		for(int x=-cnt;x<=cnt;x++)
		{
			for(int y=-cnt;y<=cnt;y++)
			{
				float pcfDepth = texture(depthMap,projCoords.xy+vec2(x,y)*texelSize).r;
				shadow +=currentDepth-bias>pcfDepth?1.0:0.0;
			}
		}
		shadow /=(2*cnt+1)*(2*cnt+1)+0.0;
		return shadow;
	
	}
}  

void main()
{
	//ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse,fs_in.TexCoords));

	//diffuse
	vec3 norm=normalize(fs_in.Normal);
	vec3 lightDir=normalize(light.position-fs_in.FragPos);
	float diff = max(dot(lightDir,norm),0);
	vec3 diffuse = diff * light.diffuse *  vec3(texture(material.diffuse,fs_in.TexCoords));

	//bling specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	vec3 halfwayDir = normalize(lightDir+viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
	vec3 specular =  spec * light.specular *vec3(texture(material.specular,fs_in.TexCoords)); 

	// shadow
	float bias = max(0.04 * (1.0 - dot(norm, lightDir)), 0.004);  
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace,bias);


	vec3 result = ambient +  (1.0 - shadow) * (diffuse + specular);
	FragColor = vec4(result, 1.0);
}
