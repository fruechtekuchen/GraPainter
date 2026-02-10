#version 430
in vec2 tc;
out vec4 out_col;
uniform sampler2D gbuf_depth;
uniform sampler2D gbuf_diff;
uniform sampler2D gbuf_pos;
uniform sampler2D gbuf_norm;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec2 near_far;
uniform vec2 screenres;
uniform int comic;

#define DISCRET_COUNTS 4.

#define EDGE_THICKNESS 2 // Bei uns sah es im Fullscreen besser aus, wenn wir einen 5x5 Pixelbereich miteinbeziehen und nicht nur die direkten Nachbarn(also 3x3 Pixelbereich)



float linear_depth(in float depth) {
    return (2.0 * near_far.x) / (near_far.y + near_far.x - depth * (near_far.y - near_far.x));
}

float disc_nl(float nl) {
    return float(floor(nl*DISCRET_COUNTS-0.000000001+1f))/DISCRET_COUNTS;
}

vec4 ambient_lighting() {
    return vec4(ambient_col * texture(gbuf_diff, tc).rgb, 1);
}

vec4 diffuse_lighting(vec3 normal, vec3 light_dir, vec3 light_col, vec3 diff_color){
    float dp = dot(normal, -light_dir);
    dp = max(dp, 0);
    return vec4(light_col * diff_color * dp, 1);
}

vec4 diffuse_lighting_comic(vec3 normal, vec3 light_dir, vec3 light_col, vec3 diff_color){
    float dp = dot(normal, -light_dir);
    dp = max(disc_nl(dp), 0);
    return vec4(light_col * diff_color * dp, 1);
}

vec4 specular_lighting(vec3 normal, vec3 diff_color, vec3 light_dir) {
    float n_shiny = 5;
    vec3 reflection = 2.0f* dot(normal, light_dir) * normal - light_dir;
    float modifier = pow(clamp(dot(vec3(0,0,-1), reflection), 0.f, 1.f), n_shiny);
    modifier = max(0.0f, modifier);
    return vec4(light_col * diff_color * modifier, 1);
}




vec4 specular_lighting_comic(vec3 normal, vec3 diff_color, vec3 light_dir) {
    float n_shiny = 5;
    float nl = disc_nl(dot(normal, light_dir));
    vec3 reflection = 2.0f* nl  * normal - light_dir;
    float modifier = pow(clamp(dot(vec3(0,0,-1), reflection), 0.f, 1.f), n_shiny);
    modifier = max(0.0f, modifier);
    return vec4(light_col * diff_color * modifier, 1);
}


bool evaluate_neighbour_depth() {
    float epsilon = 0.00001;//0.01;
    vec2 delta_pixel = vec2(1) / screenres;
    //float own_depth_value = linear_depth(texture(gbuf_depth, tc).x);
    float own_depth_value = texture(gbuf_depth, tc).x;
    for(int x = -EDGE_THICKNESS; x <= EDGE_THICKNESS; x+=2) {
        for(int y = -EDGE_THICKNESS; y <= EDGE_THICKNESS; y+=2) {
            vec2 neighbour_position = tc + vec2(x,y) * delta_pixel;
            neighbour_position = clamp(neighbour_position, 0, 1);
            //float neighbour_depth_value = linear_depth(texture(gbuf_depth, neighbour_position).x);
            float neighbour_depth_value = texture(gbuf_depth, neighbour_position).x;
            float distance = abs(own_depth_value - neighbour_depth_value);
            if (distance > epsilon) {
                return true;
            }
        }
    }
    return false;
}

bool evaluate_neighbour_normal() {
    float max_angle_difference = 3.1415926535897932384626433832795/3.;
    vec2 delta_pixel = vec2(1) / screenres;
    vec3 own_normal = texture(gbuf_norm, tc).xyz;
    for(int x = -EDGE_THICKNESS; x <= EDGE_THICKNESS; x+=2) {
        for(int y = -EDGE_THICKNESS; y <= EDGE_THICKNESS; y+=2) {
            vec2 neighbour_position = tc + vec2(x,y) * delta_pixel;
            neighbour_position = clamp(neighbour_position, 0, 1);

            vec3 neighbour_normal = texture(gbuf_norm, neighbour_position).xyz;
            float dot_product = dot(own_normal, neighbour_normal);
            float angle = acos(dot_product);

            if (angle > max_angle_difference) {
                return true;
            }
        }
    }
    return false;
}


bool is_corner() {

    if (evaluate_neighbour_depth()) return true;
    if (evaluate_neighbour_normal()) return true;
    return false;
}

void main() {
    float depth = texture(gbuf_depth, tc).r;
    if (depth == 1.0) {
        if (evaluate_neighbour_depth()) {
            out_col = vec4(0, 0, 0, 1);
            return;
        }
        else {
            discard;
        }
    }

    // Lighting
    if (comic == 0) {
        gl_FragDepth = depth;
        out_col = ambient_lighting();
        out_col += diffuse_lighting(texture(gbuf_norm, tc).xyz, light_dir, light_col, texture(gbuf_diff, tc).xyz);
        out_col += specular_lighting(texture(gbuf_norm, tc).xyz, texture(gbuf_diff, tc).xyz, light_dir);
    } else {
        gl_FragDepth = depth;
        out_col  = ambient_lighting();
        out_col += diffuse_lighting_comic(texture(gbuf_norm, tc).xyz, light_dir, light_col, texture(gbuf_diff, tc).xyz);
        out_col += specular_lighting(texture(gbuf_norm, tc).xyz, texture(gbuf_diff, tc).xyz, light_dir);
    }

    // Corner Stzle
    if (comic != 0) {
        if (is_corner()) {
            gl_FragDepth = 0.0000001; // Set the depth close to the camera, so that the particles and fog don't affect the outlines
            //out_col = vec4(0,1,1,1);
            out_col = vec4(0,0,0,1);
        }
    }
}
