#version 440

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;

out vec4 fs_color;

uniform sampler2D ilufanTexture;
uniform sampler2D boxTexture;

void main() {
    fs_color = (texture(boxTexture, vs_texcoord) +
            texture(ilufanTexture, vs_texcoord));
}
