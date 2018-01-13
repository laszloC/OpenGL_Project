#version 400 core

layout (location = 0) in vec3 vertexPosition;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec4 tempPos = projection * view * vec4(vertexPosition, 1.0);
    gl_Position = tempPos.xyww;
    textureCoordinates = vertexPosition;
}
