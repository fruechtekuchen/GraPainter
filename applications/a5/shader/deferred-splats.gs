#version 430

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float particle_size;
uniform mat4 view;
uniform mat4 proj;


in vec3 vert_particle_pos[1];
out vec4 particle_pos;
flat in int time[1];
flat out int life_time;

void main() {
    life_time = time[0];
    vec2 directions[4] = vec2[4](vec2(-1,1), vec2(-1,-1), vec2(1,1), vec2(1,-1));

    for (int i = 0; i < 4; i++) {
            vec4 pos_viewSpace =  view * vec4(vert_particle_pos[0], 1);
            vec4 pos_vertex_viewSpace =   pos_viewSpace + vec4(2.5f * vec3(directions[i].x*particle_size, directions[i].y * particle_size,0),0);
            vec4 pos_vertex_viewSpace_moved = pos_vertex_viewSpace + vec4(0,0,2.5*particle_size,0);
            gl_Position = proj * pos_vertex_viewSpace_moved;

            particle_pos = vec4(vert_particle_pos[0],1);

            EmitVertex();
    }
    EndPrimitive() ;
}