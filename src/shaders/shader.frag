#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}

    
    
    
    
//outColor = vec4(0.96078f, 0.67058f, 0.72549f, 1.0f);