#version 450

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 fragCoord;
out vec3 fragPos;
out vec3 fragNormal;

uniform mat4 mvpMatrix;
uniform mat4 viewSpaceMatrix;
uniform mat3 normalViewSpaceMatrix;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
	fragPos = vec3(viewSpaceMatrix * vec4(position, 1.0));
	fragNormal = normalize(normalViewSpaceMatrix * normal);
	fragCoord = texCoord;
}