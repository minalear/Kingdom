#version 400
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

out vec2 UV;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main() {
    UV = aUV;
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}