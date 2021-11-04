#version 330

// ins
in vec3 varyingColor;
in vec2 varyingUV;

// uniforms
uniform sampler2D sampler;

// outs
out vec4 fragColor;

void main(void) {
    fragColor = vec4(varyingColor, 1);// * texture2D(sampler, varyingUV);
}