#version 330 core

out vec4 frag_color;
  
in vec2 UV;

uniform sampler2D depthMap;

void main() {             
    float depthValue = texture(depthMap, UV).r;
    frag_color = vec4(vec3(depthValue), 1.0);
}  