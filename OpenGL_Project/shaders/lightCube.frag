#version 400 core
in vec4 fragPosEye;

out vec4 fColor;

uniform vec3 color;

uniform bool fogEnabled;
uniform vec3 fogColor;
uniform float fogDensity;

//computes the fog factor
float computeFog(){
    float fragmentDistance = length(fragPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 1));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{   
    
    fColor = vec4(color, 1.0f);
    if (fogEnabled){      
        float fogFactor = computeFog();
        fColor = vec4(fogColor, 1.0f) * (1 - fogFactor) + vec4(color, 1.0f) *  fogFactor;
    }
}
