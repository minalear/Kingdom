#version 400
in vec2 UV;

out vec4 fragColor;

uniform vec3 tint;
uniform sampler2D image;

void main() {
    vec4 sampleColor = texture(image, UV);
    fragColor = vec4(sampleColor.rgb * tint, sampleColor.a);
}