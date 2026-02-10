#version 430
in vec4 pos_wc;
in vec3 norm_wc;
layout(location = 0) out vec4 out_col;
layout(location = 1) out vec3 out_pos;
layout(location = 2) out vec3 out_norm;
uniform vec3 color;
void main() {
    out_col = vec4(color, 1);
    out_pos = pos_wc.xyz;
    out_norm = norm_wc;
}
