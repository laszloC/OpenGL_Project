#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fTexCoords;

out vec4 fColor;

//lighting
struct DirLight{
    vec3 direction;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}

struct PointLight{
    vec3 position;
    vec3 color;

    float constant = 1.0f;
    float linear = 0.0045f;
    float quadratic = 0.0075f;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}

uniform Material material;

uniform int nrPointLights;

uniform DirLight dirLight;
uniform PointLight pointLights[nrPointLights];

uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform sampler2D shadowMap;

float ambientStrength = 0.0f;
float specularStrength = 1.5f;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir);

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute distance to light
	float dist = length(lightDir - fragPosEye.xyz);

	//compute attenuation
	//float att = 1.0/(constant + linear * dist + quadratic * (dist * dist));

	//compute ambient light
	ambient = ambientStrength * lightColor;

	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

}

float computeShadow()
{
    //preform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0)
        return 0.0f;
    //Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;

    //Get closest depth value from light's perspective (using [0,1] range fragPoslight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

    //Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;

    //Check wether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = 0.0f;

    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    //soft shadow
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <=1; ++y)
        {
            float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0;

    return shadow;
}

float computeFog(){
    float fogDensity = 0.1f;
    float fragmentDistance = length(fragPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 1));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    vec3 normal = 
	computeLightComponents();

	//float shadow = computeShadow();
    float shadow = 0.0f;

    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	vec3 baseColor = vec3(1.0f, 0.55f, 0.0f);//orange
	
	//ambient *= baseColor;
	//diffuse *= baseColor;
	//specular *= baseColor;

	ambient *= vec3(texture(diffuseTexture, fTexCoords));
    diffuse *= vec3(texture(diffuseTexture, fTexCoords));
	specular *= vec3(texture(specularTexture, fTexCoords));
	
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    
    fColor = vec4(color, 1.0f);
    //fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) *  fogFactor;
}

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir){

    vec3 lightDir = normalize(-light.direction);
    //diffuse shading
    float diff = max(dot(normal, lightDir));
    //specular shading
    vec3 halfVector = 
}