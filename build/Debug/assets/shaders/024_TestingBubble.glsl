#version 450 core

layout (local_size_x = 256) in;

uniform float closest_allowed_dist = 50.0;
uniform float radius = 250;
uniform float maxradius = 500;
uniform vec3 center = vec3(0.0);
uniform float timestep = 0.4;
uniform float weight=0.15;

struct flock_member
{
    vec3 position;
    vec3 velocity;
};

layout (std430, binding = 0) readonly buffer members_in
{
    flock_member member[];
} input_data;

layout (std430, binding = 1) buffer members_out
{
    flock_member member[];
} output_data;

shared flock_member shared_member[gl_WorkGroupSize.x];

float rule1(vec3 myposition,vec3 center ,float minradius,float maxradius)
{
     float test= length(center - myposition);
    
    if(test < minradius){
        return -1.0;
    }
    if( test > maxradius){
        return -1.0;
    }
    return 1.0;
}


void main(void)
{
    uint i, j;
    int global_id = int(gl_GlobalInvocationID.x);
    int local_id  = int(gl_LocalInvocationID.x);

    flock_member me = input_data.member[global_id];
    flock_member new_me;
    vec3 accelleration = vec3(0.0);
    vec3 flock_center = vec3(0.0);


    flock_center /= float(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    new_me.position = me.position + me.velocity * timestep;
    
    float change =  rule1(me.position,center,radius,maxradius);
    
    accelleration += normalize( me.position - center)* weight;
    accelleration = accelleration * change;
    new_me.velocity = me.velocity + accelleration * timestep;

    output_data.member[global_id] = new_me;
}
