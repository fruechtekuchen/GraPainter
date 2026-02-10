#version 430
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
layout(location = 0) out vec4 out_col;
layout(location = 1) out vec3 out_pos;
layout(location = 2) out vec3 out_norm;

uniform vec3 modulate_by;
uniform sampler2D diffuse;

void main() {
    vec3 diff = texture(diffuse, tc).rgb * modulate_by;
    out_col = vec4(diff, 1);
    out_pos = pos_wc.xyz;
    out_norm = norm_wc;
}
