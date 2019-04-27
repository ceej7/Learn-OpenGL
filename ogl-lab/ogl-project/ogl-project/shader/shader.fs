#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
	sampler2D normal;
    float shininess;
}; 
struct Light {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	mat3 TBN;
} fs_in;

uniform vec3 viewPos;

uniform Material material;

uniform Light directLight;  
uniform sampler2D depthMap;

uniform Light pointLight;  
uniform samplerCube depthCubeMap;
uniform float far_plane;

out vec4 FragColor;

float ShadowCalculationForDirect(vec4 fragPosLightSpace,float bias)
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
		int cnt=4;
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

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float ShadowCalculationForPoint(vec3 fragPos,float bias,Light light)
{
	bias *= 5;
	vec3 fragToLight = fragPos - light.position;
	float currentDepth = length(fragToLight);
	if(currentDepth/far_plane>=1.0)
	{
		return 0;
	}
    float shadow = 0.0;
	int samples  = 20;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0; 
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(depthCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;   // Undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);  
	return shadow;
}


vec3 calculateAmbient(Light light)
{
	return light.ambient*vec3(texture(material.diffuse,fs_in.TexCoords));
}

vec3 calculateDiffandSpecularWithShadow(Light light ,bool isDirect) // blinn-phong model
{
	//vec3 norm=normalize(fs_in.Normal);
	// tranform the norm from TBN space(from mat.normal texture) to world space
	vec3 norm = texture(material.normal,fs_in.TexCoords).rgb;
	norm=normalize(norm*2.0 - 1.0);
	norm = normalize(fs_in.TBN*norm);
	// use light.direction if the light is direct light
	vec3 lightDir=isDirect?normalize(light.direction):normalize(light.position-fs_in.FragPos); 
	float diff = max(dot(lightDir,norm),0);
	vec3 diffuse = diff * light.diffuse*vec3(texture(material.diffuse,fs_in.TexCoords));

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	vec3 halfwayDir = normalize(lightDir+viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
	vec3 specular =  spec * light.specular *vec3(texture(material.specular,fs_in.TexCoords));

	float bias = max(0.07* (1.0 - dot(norm, lightDir)), 0.01);  
	// calcuate shadow with direct/point shadow calculation if isDrect or not
	float shadow = isDirect?ShadowCalculationForDirect(fs_in.FragPosLightSpace,bias):ShadowCalculationForPoint(fs_in.FragPos,bias,light);
	vec3 result = (1.0 - shadow) * (diffuse + specular);
	return result;

}

void main()
{
	vec3 result;
	vec3 ambient=vec3(0.0);
	vec3 diff_spec = vec3(0.0);
	//	ambient & diffuse + specular -- direct light
	ambient += calculateAmbient(directLight);
	diff_spec += calculateDiffandSpecularWithShadow(directLight,true);

	//ambient & diffuse + specular -- point light
	ambient += calculateAmbient(pointLight);
	diff_spec+= calculateDiffandSpecularWithShadow(pointLight,false);

	FragColor = vec4(ambient + diff_spec, 1.0);
}
