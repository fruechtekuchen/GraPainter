#version 430
in vec4 particle_pos;
out vec4 out_col;
uniform vec2 near_far;
uniform vec2 screenres;
uniform float particle_size;
uniform sampler2D gbuf_depth;
uniform sampler2D gbuf_diff;
uniform sampler2D gbuf_pos;
uniform sampler2D gbuf_norm;
flat in int life_time;

float linear_depth(in float depth, in float near, in float far) { return (2.0 * near) / (far + near - depth * (far - near)); }

void main() {
    vec2 screen_space_pos = gl_FragCoord.xy/screenres;
    vec4 obj_pos_WS = texture(gbuf_pos, screen_space_pos.xy);
    float obj_depth =  texture(gbuf_depth, screen_space_pos.xy).x;
    float obj_depth_lin = linear_depth(obj_depth, near_far.x, near_far.y);

    float obj_col = texture(gbuf_diff, screen_space_pos.xy).x;
    vec3 obj_norm = texture(gbuf_norm, screen_space_pos.xy).xyz;

    if (obj_depth_lin < linear_depth(gl_FragCoord.z, near_far.x, near_far.y)) { // Is probably done by OpenGl already, but we want to make sure
        discard;
    }
    if (obj_depth == 1.0) {
        discard;
    }

    vec3 light_vec_WS = particle_pos.xyz - obj_pos_WS.xyz;
    float dist = length(light_vec_WS);
    if (dist > particle_size*2.5f) {
        discard;
    }

    float fade_time = clamp(life_time /  500.f, 0, 1);

    vec3 light_dir_WS = normalize(light_vec_WS);

    float lambert_factor = max(0.0, dot(obj_norm, light_dir_WS));
    float quadratic_falloff =  1- (pow(dist/particle_size/2.5,2.0f));

    float frag_depth = linear_depth(gl_FragCoord.z, near_far.x, near_far.y);
    float geom_depth = obj_depth_lin;
    float fade_depth = clamp(abs(frag_depth - geom_depth) / 0.05f, 0.0, 1.0);

    vec4 light_col = pow(fade_time, 2) * vec4(1, 115 / 255.f, 65 / 255.f, 1) * lambert_factor *  (fade_depth * fade_depth) * quadratic_falloff;
    out_col = light_col * obj_col;
}
