#version 400 core

in vec3 textureCoordinates;
out vec4 fColor;

uniform samplerCube skybox;

uniform bool fogEnabled;
uniform vec3 fogColor;
uniform vec3 lightColor;

void main()
{
    fColor = texture(skybox, textureCoordinates);
	fColor *= vec4(lightColor, 1.0f);
    if  (fogEnabled){
        fColor = vec4(fogColor, 1.0f);
    }
}
