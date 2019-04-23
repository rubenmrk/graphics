#version 450

in vec2 fragCoord;
in vec3 fragPos;
in vec3 fragNormal;

out vec4 outColor;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 ambient;
uniform Light light;
uniform Material material;

void main()
{
    // Ambient
    vec3 ambientColor = ambient * texture(material.diffuse, fragCoord).rgb;

    // Diffuse
    vec3 lightDir = normalize(light.position - fragPos);
	float diffscalar = max(dot(fragNormal, lightDir), 0.0);
    
    vec3 diffuseColor = light.diffuse * (diffscalar * texture(material.diffuse, fragCoord).rgb);

    // Specular
    vec3 viewDir = normalize(-fragPos);
	vec3 reflectDir = reflect(-lightDir, fragNormal);
	float specscalar = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 specularColor = light.specular * (specscalar * texture(material.specular, fragCoord).rgb);

	// Resulting Phong shading
    outColor = vec4(ambientColor + diffuseColor + specularColor, 1.0);
}