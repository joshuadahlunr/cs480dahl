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
    // Normalize the input lighting vectors

   	vec3 N = normalize(fN);
    vec3 E = normalize(fE);
    vec3 L = normalize(fL);

    vec3 H = normalize( L + E );
    vec4 ambient = ambientProduct;
	float Kd = max(dot(L, N), 0.0);
    vec4 diffuse = Kd*diffuseProduct;

    float Ks = pow(max(dot(N, H), 0.0), shininess);
    vec4 specular = Ks*specularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0 )
	specular = vec4(0.0, 0.0, 0.0, 1.0);

    gl_FragColor = ambient + diffuse + specular;
    gl_FragColor.a = 1.0;
}
