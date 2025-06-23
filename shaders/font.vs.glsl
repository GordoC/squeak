#version 330
in vec3 in_position;
in vec2 in_texcoord;
out vec2 TexCoords;

uniform mat3 projection;
uniform mat3 transform;

void main()
{
    // For example, use only the xy components of position.
    vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos, 1.0);
    TexCoords = in_texcoord;
}