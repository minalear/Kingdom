#version 400
in vec2 UV;

out vec4 fragColor;

uniform sampler2D image;

void main() {
    vec4 sampleColor = texture(image, UV);
    fragColor = vec4(sampleColor.rgb, sampleColor.a);
}