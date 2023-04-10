#version 460

//in vec3 position;
in GS_OUT{
    vec3 position;
    vec3 normal;
}fs_in;

out vec4 color;

uniform vec3 viewPosition;
uniform vec3 lightPosition;
void main(){
    vec3 fragPosition = fs_in.position;
    vec3 lightDir = normalize(lightPosition - fragPosition);
    float diffuse = max(dot(fs_in.normal, lightDir), 0.0);

    float ambient=0.1;

    float specular=0.0;

    color = vec4( (diffuse + ambient + specular) * vec3(1.0), 1.0);
}