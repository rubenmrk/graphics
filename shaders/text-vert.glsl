#version 450

in vec3 position;
in vec2 texCoord;

out vec2 fragCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    fragCoord = texCoord;
}