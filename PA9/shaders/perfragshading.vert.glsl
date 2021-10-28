#version 330

// Load the position, color, and texture coordinates from the CPU
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_uv;
// Load the MVP matricies from the CPU
uniform vec4 ambientProduct, diffuseProduct, specularProduct;
uniform float shininess;
uniform vec4 lightPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// We output color and textures coordinates
smooth out vec3 color;
smooth out vec2 uv;
// output values that will be interpolatated per-fragment
out vec3 fN;
out vec3 fE;
out vec3 fL;


void main()
{
    fN = vNormal;
    fE = vPosition.xyz;
    fL = lightPosition.xyz;

    if( LightPosition.w != 0.0 ) {
	    fL = lightPosition.xyz - vPosition.xyz;
    }

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vPosition;
    color = v_color;
    uv = v_uv;
}
