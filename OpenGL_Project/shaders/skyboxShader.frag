#version 410 core

in vec3 textureCoordinates;
out vec4 fColor;

uniform samplerCube skybox;

uniform bool fogEnabled;
uniform vec3 fogColor;
void main()
{
    fColor = texture(skybox, textureCoordinates);
    if  (fogEnabled){
        fColor = vec4(fogColor, 1.0f);
    }
}
