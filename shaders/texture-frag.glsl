#version 450

in vec2 fragCoord;

out vec4 outColor;

uniform sampler2D textureColor;

void main()
{
    outColor = texture(textureColor, fragCoord);
}