#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 2) in vec3 particle_position;
layout (location = 3) in vec3 particle_velocity;

out VS_OUT
{
    flat vec3 color;
} vs_out;

uniform mat4 mvp;

mat4 make_lookat(vec3 forward, vec3 up)
{
    vec3 side = cross(forward, up);
    vec3 u_frame = cross(side, forward);

    return mat4(vec4(side, 0.0),
                vec4(u_frame, 0.0),
                vec4(forward, 0.0),
                vec4(0.0, 0.0, 0.0, 1.0));
}

vec3 choose_color(float f)
{
    float R = sin(f * 6.2831853+0.666);
    float G = sin((f + 0.3333) * 6.2831853);
    float B = 0;

    return vec3(R, G, B) * 0.25 + vec3(0.75,0.25,0);
}

void main(void)
{
    mat4 lookat = make_lookat(normalize(particle_velocity), vec3(0.0, 1.0, 0.0));
    vec4 obj_coord = lookat * vec4(position.xyz, 1.0);
    gl_Position = mvp * (obj_coord + vec4(particle_position, 0.0));

    vec3 N = mat3(lookat) * normal;
    vec3 C = choose_color(fract(float(gl_InstanceID / float(1237.0))));

    vs_out.color = C;
}
