#version 430
in vec4 pos_wc;
in vec2 tc;
out vec4 out_col;

uniform mat4 view;
uniform mat4 proj;
uniform sampler2D gbuf_depth;
uniform sampler2D gbuf_pos;
uniform float time;
uniform vec2 near_far;

float linear_depth(in float depth) {
    return (2.0 * near_far.x) / (near_far.y + near_far.x - depth * (near_far.y - near_far.x));
}

//--- BEGIN MAGIC
float random(in vec2 _st) { return fract(sin(dot(_st.xy, vec2(12.9898, 78.233)))* 43758.5453123); }
// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 _st) {
    vec2 i = floor(_st);
    vec2 f = fract(_st);
    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}
#define NUM_OCTAVES 7
float fbm(in vec2 _st) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}
float fog(in vec2 st, in float time) {
    vec2 q = vec2(fbm(st + 0.00*time),
            fbm(st + vec2(1.0)));
    vec2 r = vec2(fbm(st + 1.0*q + vec2(1.7,9.2)+ 0.15*time),
            fbm(st + 1.0*q + vec2(8.3,2.8)+ 0.126*time));
    float f = fbm(st+r);
    return sqrt(f*f*f + .6*f*f + .5*f);
}
//--- END MAGIC
#define FOG_NOISE_SCALE 5
#define FOG_BASE_INTENSITY 0.5
#define FOG_SPEED 5

void main() {
    // Get screen coordinates of the current fragment for the wpos lookup
    vec4 sc = proj * view * pos_wc;
    sc /= sc.w;
    sc = 0.5f*sc + vec4(.5f,.5f,0,0);

    // calculate fog distance (called l in the slides)
    float obj_depth = texture(gbuf_depth, sc.xy).x;
    float l = 0;
    if (obj_depth == 1.0f) { // in case there this no object in the buffer
        l = 10;
    } else {
        vec4 obj_pos_wc = texture(gbuf_pos, sc.xy);
        l = distance(pos_wc, obj_pos_wc);
    }

    // calculate the noise (called n)
	vec2 offset = FOG_SPEED/1000. * vec2(0,1)*time/1000.f;
    float n = fog(30*vec2(tc + offset), time/1000);//, time/2500.);
    // map n to value between -FOG_NOISE_SCALE and FOG_NOISE_SCALE
    n/=1.4;
    n -= 0.5;
    n *= 2;
    n *= FOG_NOISE_SCALE;
    l *= FOG_BASE_INTENSITY;

    float fog_thickness = 1-exp(-0.075*(l+n));

    float ntcx = 2 * tc.x-1;
    float ntcy = 2 * tc.y-1; 

    if (ntcx < 0.5 && ntcx > -0.5 && ntcy < 0.5 && ntcy > -0.5) {
        // out_col = vec4(1,1,1,fog_thickness);
    } else {
        float distx = 0;
        float disty = 0;

        if (!(ntcx < 0.5 && ntcx > -0.5)) {
            distx = abs(abs(ntcx) - 0.5);
        }
        if (!(ntcy < 0.5 && ntcy > -0.5)) {
            disty = abs(abs(ntcy) - 0.5);
        }
        float falloff_dist = 0.4f;
        float dist_comb = sqrt( distx *distx + disty *disty);
        float wert = min(falloff_dist, dist_comb);
        float quadratic_falloff = pow(1-wert/falloff_dist,2.0f);
        fog_thickness *= quadratic_falloff;
    }

    float frag_depth = linear_depth(gl_FragCoord.z);
    float geom_depth = linear_depth(texture(gbuf_depth, sc.xy).x);
    float fade_depth = clamp(abs(frag_depth - geom_depth) / 0.01f, 0.0, 1.0);
    fog_thickness *= fade_depth;
    out_col = vec4(1,1,1, fog_thickness);
}
