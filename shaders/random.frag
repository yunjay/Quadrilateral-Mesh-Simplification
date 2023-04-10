#version 460 
out vec4 color;
float random(vec2 seed){
    return fract( sin(dot(seed.xy ,vec2(12.9898,78.233))* 437.453) * 0.8);
    //0.8 so eyes don't ow
};
void main()
{   
    float seed = float(gl_PrimitiveID);

    color = vec4(
                random(vec2(seed,1.0)),
                random(vec2(seed,2.0)),
                random(vec2(seed,3.0)),
                1.0);
}