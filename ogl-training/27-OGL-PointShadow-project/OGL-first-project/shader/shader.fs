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
} fs_in;

uniform Material material;
uniform Light light;  
uniform vec3 viewPos;
uniform float far_plane;
uniform samplerCube depthMap;
out vec4 FragColor;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - light.position;
	float currentDepth = length(fragToLight);
//	float closestDepth = texture(depthMap, fragToLight).r;
//	closestDepth *= far_plane;
//	float bias = 0.05;
//	float shadow = currentDepth-bias >closestDepth ?1.0:0.0;
//	return shadow;


//	float shadow =0;
//	float bias = 0.05;
//	float samples = 4.0;
//	float offset = 0.1;
//
//	for(float x = -offset; x<offset;x+=offset/(samples*0.5))
//	{
//		for(float y = -offset; y<offset;y+=offset/(samples*0.5))
//		{
//			for(float z = -offset; z<offset;z+=offset/(samples*0.5))
//			{
//				float cloestDepth = texture(depthMap ,fragToLight+vec3(x,y,z)).r;
//				cloestDepth*=far_plane;
//				shadow += currentDepth-bias > cloestDepth?1.0:0.0;
//			}
//		}
//	}
//	shadow/=(samples*samples*samples);
//	return shadow;
	float shadow = 0.0;
	float bias   = 0.15;
	int samples  = 20;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0; 
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;   // Undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);  
	return shadow;
}  

void main()
{
//	//ambient
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
	float shadow = ShadowCalculation(fs_in.FragPos);

	vec3 result = ambient +  (1.0 - shadow) * (diffuse + specular);
	FragColor = vec4(result, 1.0);
}
