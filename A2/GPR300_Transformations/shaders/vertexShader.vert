#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;

out vec3 Normal;
out float height;
uniform float time;
uniform float yScale;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main(){ 
    Normal = vNormal;
    gl_Position = projection * view * model * vec4(vPos,1);
    height = yScale;
}
