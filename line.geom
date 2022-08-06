#version 330

layout (lines) in;
layout (triangle_strip, max_vertices = 8) out;

uniform mat4 mvp;
uniform vec2 viewportSize;
uniform float thickness;

out vec2 gs_texCoord;

void main() {
    vec4 from = gl_in[0].gl_Position;
    vec4 to = gl_in[1].gl_Position;

    vec4 fromProjected = mvp * from;
    vec4 toProjected = mvp * to;

    vec2 fromScreen = (fromProjected.xy / fromProjected.w) * viewportSize;
    vec2 toScreen = (toProjected.xy / toProjected.w) * viewportSize;

    vec2 dirScreen = thickness * normalize(toScreen - fromScreen);
    vec2 perpScreen = vec2(-dirScreen.y, dirScreen.x);

    // TODO: z is constant for the whole primitive, ideally should vary if we want to e.g.
    // overlay the wireframe on top of a solid mesh
    float z = fromProjected.z / fromProjected.w;

    vec2 p0 = fromScreen - dirScreen - perpScreen;
    gl_Position = vec4(p0 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.0, 0.0);
    EmitVertex();

    vec2 p1 = p0 + 2.0 * perpScreen;
    gl_Position = vec4(p1 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.0, 1.0);
    EmitVertex();

    vec2 p2 = p0 + dirScreen;
    gl_Position = vec4(p2 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.5, 0.0);
    EmitVertex();

    vec2 p3 = p2 + 2.0 * perpScreen;
    gl_Position = vec4(p3 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.5, 1.0);
    EmitVertex();

    vec2 p4 = toScreen - perpScreen;
    gl_Position = vec4(p4 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.5, 0.0);
    EmitVertex();

    vec2 p5 = p4 + 2.0 * perpScreen;
    gl_Position = vec4(p5 / viewportSize, z, 1.0);
    gs_texCoord = vec2(0.5, 1.0);
    EmitVertex();

    vec2 p6 = p4 + dirScreen;
    gl_Position = vec4(p6 / viewportSize, z, 1.0);
    gs_texCoord = vec2(1.0, 0.0);
    EmitVertex();

    vec2 p7 = p6 + 2.0 * perpScreen;
    gl_Position = vec4(p7 / viewportSize, z, 1.0);
    gs_texCoord = vec2(1.0, 1.0);
    EmitVertex();
    
    EndPrimitive();
}
