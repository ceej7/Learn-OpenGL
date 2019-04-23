#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseTexture;

uniform vec3 samples[64];

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0); 

uniform mat4 projection;

void main()
{             
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(noiseTexture,TexCoords*noiseScale).xyz);

	vec3 tangent = normalize(randomVec - Normal *dot(randomVec,Normal));
	vec3 bitangent = cross(Normal,tangent);
	mat3 TBN = mat3 (tangent, bitangent,Normal);

	float occlusion = 0.0;
	for(int i=0; i<kernelSize; i++)
	{
		vec3 sample = TBN*samples[i];
		sample = FragPos + sample *radius;

		vec4 offset = vec4(sample,1.0);
		offset = projection * offset;
		offset.xyz/=offset.w;
		offset.xyz = offset.xyz*0.5+0.5;

		float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(FragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion =1.0 - (occlusion/kernelSize);
   
    FragColor = vec4(vec3(occlusion), 1.0);
}