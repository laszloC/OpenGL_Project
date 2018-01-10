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
};

struct PointLight{
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material{
    sampler2D ambient;
    sampler2D diffuse;
    sampler2D specular;
};

float shininess = 64.0f;

uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[2];

uniform	mat3 normalMatrix;
//uniform mat3 lightDirMatrix;

uniform sampler2D shadowMap;

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir);

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir);

float computeShadow()
{
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 1.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	
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
    //properties
    vec3 cameraPosEye = vec3(0.0f);

    vec3 normalEye = normalize(normalMatrix * normal);

    vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);

    vec3 color = calculateDirLight(dirLight, normalEye, viewDirN);
    
    vec4 diffTex = texture(material.diffuse, fTexCoords);
    if (diffTex.a < 0.1){
        discard;    
    }

    fColor = vec4(color, 1.0f);
    //fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) *  fogFactor;
}

vec3 calculateDirLight(DirLight light, vec3 normalN, vec3 viewDir){

    //vec3 lightDir = normalize(lightDirMatrix * light.direction);
    vec3 lightDir = normalize(light.direction);

    //diffuse shading
    float diff = max(dot(normalN, light.direction), 0.0f);
    
    //specular shading
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normalN), 0.0f), shininess);

    vec3 ambient = light.ambient * vec3(texture(material.ambient, fTexCoords)) * light.color;
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fTexCoords)) * light.color;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fTexCoords)) * light.color;
    
    //float shadow = computeShadow();
    float shadow = 0.0f;
    return min((ambient + (1.0f - shadow) * diffuse + (1.0f - shadow) * specular), 1.0f);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);

    //specular shading
    vec3 halfVector = normalize(light.position + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0f), shininess);

    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fTexCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}