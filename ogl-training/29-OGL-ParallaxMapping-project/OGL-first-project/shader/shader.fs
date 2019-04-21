#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightscale;

vec2 ParallaxMapping(vec2 texCoords,vec3 viewDir)
{
    const float minLayers = 64.0;
	const float maxLayers = 256.0;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  

	float minheightscale = heightscale;
	float maxheightscale = 3*heightscale;
	float fixedheightscale = mix(maxheightscale, minheightscale, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  


	float layerDepth = 1.0 / numLayers;
	float currentDepth = 0.0;

	vec2 P = viewDir.xy/viewDir.z * fixedheightscale;
	vec2 currentTexCoords = texCoords;
	vec2 deltaTexCoords = P / numLayers;

	float currentDepthMapValue = texture(depthMap , currentTexCoords).r;

	while(currentDepth<currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepth +=layerDepth;
		currentDepthMapValue = texture(depthMap , currentTexCoords).r;
	}

	vec2 prevTexCoords = currentTexCoords+deltaTexCoords;
	float afterdepth = currentDepth - currentDepthMapValue;
	float beforedepth = texture(depthMap,prevTexCoords).r - currentDepth+layerDepth;

	// interpolation of texture coordinates
	float weight = afterdepth / (afterdepth + beforedepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords; 
}

void main()
{           
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

	vec2 texCoords = ParallaxMapping(fs_in.TexCoords,viewDir);
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		discard;
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap,texCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}