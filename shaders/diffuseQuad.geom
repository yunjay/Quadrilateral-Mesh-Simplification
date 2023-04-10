#version 460
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;
in VS_OUT{
    vec3	vertexPosition;
	vec3	Normal;		// per vertex normal vector
	//vec4	color;
}gs_in[4];

//out vec3 position;
out GS_OUT{
    vec3 position;
    vec3 normal;

}gs_out;
void main(){
    //traingle fan needs ordering. 0-1-3-2 or 1-0-2-3 etc
    //If a geometry shader is present, gl_PrimitiveID needs to be defined, or will be undefined.
    //gl_PrimitiveIDIn is a built in input.
    gl_Position=gl_in[0].gl_Position;
    gs_out.normal = gs_in[0].Normal;
    gs_out.position = gs_in[0].vertexPosition;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    gl_Position=gl_in[1].gl_Position;
    gs_out.normal = gs_in[1].Normal;
    gs_out.position = gs_in[1].vertexPosition;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    gl_Position=gl_in[3].gl_Position;
    gs_out.normal = gs_in[3].Normal;
    gs_out.position = gs_in[3].vertexPosition;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    gl_Position=gl_in[2].gl_Position;
    gs_out.normal = gs_in[2].Normal;
    gs_out.position = gs_in[2].vertexPosition;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();
    
    EndPrimitive();
}