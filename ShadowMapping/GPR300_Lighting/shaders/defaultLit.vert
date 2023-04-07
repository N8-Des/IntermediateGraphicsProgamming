#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;
uniform vec3 _LightPos;
uniform vec3 _ViewPos;
uniform mat4 _LightMatrix;

out vec3 WorldPos;
out vec3 WorldNormal;
out vec4 LightSpaceFragPosition;
out vec3 normal;
out vec2 uv;

void main(){    
    WorldPos = vec3(_Model * vec4(vPos, 1));
    WorldNormal = mat3(transpose(inverse(_Model))) * vNormal;
    uv = vUV; 
    normal = vNormal;
    LightSpaceFragPosition = _LightMatrix * vec4(WorldPos, 1);
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}

