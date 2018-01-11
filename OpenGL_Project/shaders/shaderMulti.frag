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

struct Phong{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

float shininess = 64.0f;

uniform Material material;

uniform DirLight dirLight;
uniform PointLight pointLights[2];

uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;

uniform mat4 view;

uniform sampler2D shadowMap;

Phong calculateDirLight(DirLight lightD, vec3 normal, vec3 viewDir);

Phong calculatePointLight(PointLight lightP, vec3 normal, vec3 fragPos, vec3 viewDir);

float computeShadow()
{
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias> closestDepth ? 1.0f : 0.0f;
	
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
    vec4 diffTex = texture(material.diffuse, fTexCoords);
    if (diffTex.a < 0.1){
        discard;    
    }

    vec3 cameraPosEye = vec3(0.0f);// in eye coordinates the camera is at the origin

    vec3 normalEye = normalize(normalMatrix * normal);

    vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);

    Phong directional = calculateDirLight(dirLight, normalEye, viewDirN);

    float shadow = computeShadow();

    vec3 directionalC = directional.ambient + (1.0f - shadow) * directional.diffuse + (1.0f - shadow) * directional.specular;

    Phong positional[2];
    
    positional[0] = calculatePointLight(pointLights[0], normalEye, fragPosEye.xyz, viewDirN);
    positional[1] = calculatePointLight(pointLights[1], normalEye, fragPosEye.xyz, viewDirN);

    vec3 positional0 = positional[0].ambient + positional[0].diffuse + positional[0].specular;
    vec3 positional1 = positional[1].ambient + positional[1].diffuse + positional[1].specular;

    //vec3 color = directionalC;
    //vec3 color = positional0;
    // vec3 color = positional1;
    // vec3 color = directionalC + positional0;
    // vec3 color = directionalC + positional1;
    vec3 color = directionalC + positional0 + positional1;

    fColor = vec4(color, 1.0f);
    //fColor = vec4(normalEye, 1.0f);
    //fColor = vec4(normalize(lightDirMatrix * dirLight.direction), 1.0f);
    //fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) *  fogFactor;
}

Phong calculateDirLight(DirLight lightD, vec3 normalN, vec3 viewDir){

    Phong phong;

    vec3 lightDir = normalize(lightDirMatrix * lightD.direction);
    //vec3 lightDir = normalize((view * vec4(lightD.direction, 1.0f)).xyz);

    //diffuse shading
    float diff = max(dot(normalN, lightDir), 0.0f);
    
    //specular shading
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normalN), 0.0f), shininess);

    phong.ambient = vec3(texture(material.diffuse, fTexCoords));
    phong.diffuse = vec3(texture(material.diffuse, fTexCoords));
    phong.specular = vec3(texture(material.specular, fTexCoords));

    phong.ambient *= lightD.ambient * lightD.color;
    phong.diffuse *= lightD.diffuse * diff * lightD.color;
    phong.specular *= lightD.specular * spec * lightD.color;
    
    return phong;
}

Phong calculatePointLight(PointLight lightP, vec3 normalN, vec3 fragPos, vec3 viewDir)
{
    Phong phong;

    vec3 lightPosEye = (view * vec4(lightP.position, 1.0f)).xyz;
    vec3 lightDirN = normalize(lightPosEye - fragPos);

    //diffuse shading
    float diff = max(dot(normalN, lightDirN), 0.0f);

    //specular shading
    vec3 halfVector = normalize(lightDirN + viewDir);
    float spec = pow(max(dot(halfVector, normalN), 0.0f), shininess);

    float dist = length(lightP.position - fragPos);

    float att = 1.0/(lightP.constant + lightP.linear * dist + lightP.quadratic * (dist * dist));

    //float attenuation = 1.0f;
    
    phong.ambient = att * lightP.ambient * lightP.color;
    phong.diffuse = att * lightP.diffuse * diff * lightP.color;
    phong.specular = att * lightP.specular * spec * lightP.color;
    
    phong.ambient *= vec3(texture(material.diffuse, fTexCoords));
    phong.diffuse *= vec3(texture(material.diffuse, fTexCoords));
    phong.specular *= vec3(texture(material.specular, fTexCoords));

    return phong;
}