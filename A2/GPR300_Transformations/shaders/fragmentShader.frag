#version 450                          
out vec4 FragColor;

in vec3 Normal;
in float height;
void main(){         
    FragColor = vec4(1 - height * 0.1, height * 0.15, 0, 1);
}
