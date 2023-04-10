#version 460 
out vec4 color;
float random(vec2 seed){
    return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))* 437.453)) * 0.91;
    //return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))* 4.153)) * 0.9;
    //0.8 so eyes don't ow
};
// max float 3E+38
void main()
{   
    //anything over 1000 breaks the random function for some reason. Overflow?
    float seed = float(gl_PrimitiveID)/71.759 + fract(sin(float(gl_PrimitiveID/852.0)*0.021));
    
    color = vec4(
                0.1+random(vec2(seed,1.0)),
                0.1+random(vec2(seed,2.0)),
                0.1+random(vec2(seed,3.0)),
                1.0);
    
}