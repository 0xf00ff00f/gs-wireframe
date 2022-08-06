#version 330

in vec2 gs_texCoord;

uniform float thickness;

out vec4 fragColor;

void main(void)
{
    const float radius = 0.5;
    float feather = 2.0 / thickness;

    float d = length(gs_texCoord - vec2(0.5, 0.5));
    float c = smoothstep(radius, radius - feather, d);

    fragColor = vec4(1.0, 1.0, 1.0, c);
}
