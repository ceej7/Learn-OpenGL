#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform sampler2D blurBuffer;
uniform bool blur=true;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
	vec3 blurColor = texture(blurBuffer, TexCoords).rgb;
    if(blur)
    {
        // reinhard
        vec3 result =hdrColor+blurColor;
        // exposure
        result = vec3(1.0) - exp(-result * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // reinhard
        vec3 result =blurColor;
        // exposure
        result = vec3(1.0) - exp(-result * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}