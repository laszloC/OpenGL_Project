#version 410 core

out vec4 fColor;

uniform vec3 color;

void main() 
{    
    fColor = vec4(color, 1.0f);
    //fColor = vec4(1.0f);
}
