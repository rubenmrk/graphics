#version 450

in vec3 position;
in vec2 texCoord;

out vec2 fragCoord;

uniform mat4 mvpMatrix;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
	fragCoord = texCoord;   
}