#version 460

layout (binding = 0, rgba32f) uniform image2D canvas_texture;

layout (local_size_x = 1, local_size_y = 1) in;


uniform ivec2 start_corner;
uniform ivec2 brush_center;
uniform float brush_size;
uniform vec3 paint_color;
uniform float delta_time;


void main() {
    ivec2 position = start_corner + ivec2(gl_GlobalInvocationID.xy);
    float dist = distance(position, brush_center);
    if(dist < brush_size) {
        float dist = distance(position, brush_center);
        float alpha = (1 - dist/brush_size)*delta_time*20;
        vec3 old_color = imageLoad(canvas_texture, position).xyz;
        vec3 new_color = alpha*paint_color + (1-alpha)*old_color;
        imageStore(canvas_texture, position, vec4(new_color, 1));
    }
}