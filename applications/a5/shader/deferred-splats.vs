#version 430
layout(location = 0) in vec3 in_pos;
layout(location = 1) in int in_lifetime;

uniform mat4 view;
uniform mat4 proj;

out vec3 vert_particle_pos;
flat out int time;

void main() {
    time = in_lifetime;
    vec4 pos_c = view * vec4(in_pos, 1);
    gl_Position = proj * pos_c;
    vert_particle_pos = in_pos;
}
