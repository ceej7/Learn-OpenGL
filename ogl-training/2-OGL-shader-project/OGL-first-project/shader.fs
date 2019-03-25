#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec3 posColor;
void main()
{
	FragColor = vec4(posColor.x/2+0.5,posColor.y/2+0.5,posColor.z/2+0.5,1.0);
}
