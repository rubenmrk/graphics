#version 450

in vec2 fragCoord;

out vec4 outColor;

uniform vec3 textColor;

uniform sampler2D textureColor;

void main()
{
    float alpha = texture(textureColor, fragCoord).r;
    if (alpha == 0)
        outColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    else
        outColor = vec4(textColor, alpha);
}