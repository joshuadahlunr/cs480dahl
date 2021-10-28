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

void main()
{
    // Transform vertex  position into eye space coordinates
    vec3 pos = (ModelView * vPosition).xyz;

    vec3 L = normalize( LightPosition.xyz - pos );   // Light
    vec3 E = normalize( -pos );                      // Eye
    vec3 H = normalize( L + E );                     // Halfway

    // Transform vertex normal into eye space coordinates
    vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;

	// Compute terms in the illumination equation
    vec4 ambient = ambientProduct;

    float Kd = max( dot(L, N), 0.0 );		// if angle is > 90 dot is negative
    vec4  diffuse = Kd*diffuseProduct;
    float Ks = pow( max(dot(N, H), 0.0), shininess );
    vec4  specular = Ks * specularProduct;
    if( dot(L, N) < 0.0 )  specular = vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position = Projection * ModelView * vPosition;

    color = ambient + diffuse + specular;
    color.a = 1.0;
    uv = v_uv;
}
