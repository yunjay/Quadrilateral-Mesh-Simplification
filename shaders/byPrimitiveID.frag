#version 460 
out vec4 color;
uniform int numElements;
void main()
{   
    color = vec4(vec3(clamp(float(gl_PrimitiveID)/float(numElements),0.0,1.0)),1.0);
}