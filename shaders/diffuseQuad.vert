#version 460 
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out VS_OUT
{
	vec3	vertexPosition;
	vec3	Normal;		// per vertex normal vector
	//vec4	color;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(inPosition, 1.0); 
	vs_out.vertexPosition = vec3(model * vec4(inPosition, 1.0));
	vs_out.Normal = normalize(transpose(inverse(mat3(model)))*inNormal);

}
