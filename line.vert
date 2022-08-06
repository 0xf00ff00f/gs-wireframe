#version 330

layout(location = 0) in vec3 from;
layout(location = 1) in vec3 to;
layout(location = 2) in vec2 shift;
layout(location = 3) in vec2 texCoord;

uniform mat4 mvp;
uniform vec2 viewportSize;
uniform float thickness;

out vec2 vs_texCoord;

void main(void)
{
    vec4 fromProjected = mvp * vec4(from, 1.0);
    vec4 toProjected = mvp * vec4(to, 1.0);

    vec2 fromScreen = (fromProjected.xy / fromProjected.w) * viewportSize;
    vec2 toScreen = (toProjected.xy / toProjected.w) * viewportSize;
    vec2 dirScreen = normalize(toScreen - fromScreen);
    vec2 perpScreen = vec2(-dirScreen.y, dirScreen.x);
    vec2 posScreen = fromScreen + thickness * shift.x * dirScreen + thickness * shift.y * perpScreen;

    gl_Position = vec4(posScreen / viewportSize, fromProjected.z / fromProjected.w, 1.0);
    vs_texCoord = texCoord;
}
