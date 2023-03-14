#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vTan;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;
uniform vec3 _LightPos;
uniform vec3 _ViewPos;

out vec3 WorldPos;
out vec3 WorldNormal;
out mat3 tangent;

out vec3 normal;
out vec2 uv;

void main(){    
    WorldPos = vec3(_Model * vec4(vPos, 1));
    WorldNormal = mat3(transpose(inverse(_Model))) * vNormal;
    uv = vUV; 
    tangent = mat3(vTan, cross(WorldNormal, vTan), WorldNormal);
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}

