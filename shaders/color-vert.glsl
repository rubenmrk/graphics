#version 450

in vec3 position;
in vec3 color;

out vec3 fragColor;

uniform mat4 mvpMatrix;

void main()
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	fragColor = color;
}