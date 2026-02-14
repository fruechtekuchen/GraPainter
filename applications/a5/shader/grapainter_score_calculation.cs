#version 460

layout (local_size_x = 1, local_size_y = 1) in;

layout(binding = 0, std430) buffer positions_buffer
{
    uint scores[];
};
layout (binding = 1, rgba32f) uniform image2D canvas_texture;
layout (binding = 2) uniform sampler2D image_texture;


void main() {
    uint index = gl_GlobalInvocationID.y;
    scores[index] = 0;

    vec2 canvas_size = imageSize(canvas_texture);
    for(int x = 0; x < canvas_size.x; x++) {
        vec2 image_tc = vec2(x, index) / canvas_size;
        vec4 image_val = texture(image_texture, image_tc);
        vec4 fixed_image_val = vec4(1);
        if(abs(image_val.r) < abs(image_val.r - 1)) fixed_image_val.r = 0;
        if(abs(image_val.g) < abs(image_val.g - 1)) fixed_image_val.g = 0;
        if(abs(image_val.b) < abs(image_val.b - 1)) fixed_image_val.b = 0;
        vec4 canvas_val = imageLoad(canvas_texture, ivec2(x, index));
        bool is_painted_right = fixed_image_val == canvas_val;
        if(is_painted_right){
            scores[index] += 1;
        } else {
            //imageStore(canvas_texture, ivec2(x, index), vec4(0.1, 0.1, 0.1,1));
        }
        //imageStore(canvas_texture, ivec2(x, index), (0.3*image_val+0.7*canvas_val));

    }
    
}