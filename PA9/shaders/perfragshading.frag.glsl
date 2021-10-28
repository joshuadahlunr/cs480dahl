#version 330

// Load the MVP matricies from the CPU
vec4 ambientProduct = vec4(0, 0, .5, 1), diffuseProduct = vec4(0,.5,0,1), specularProduct = vec4(.5, 0, 0, 1);
float shininess = 200;

uniform vec4 lightPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// We input vertex color and textures coordinates
smooth in vec3 color;
smooth in vec2 uv;

// Input values that were interpolatated per-fragment
in vec3 fN;
in vec3 fE;
in vec3 fL;

out vec4 frag_color;

void main()
{
    // Normalize the input lighting vectors
   	vec3 N = normalize(fN);
    vec3 E = normalize(fE);
    vec3 L = normalize(fL);

    vec3 H = normalize( L + E );
    vec4 ambient = ambientProduct;
	float Kd = max(dot(L, N), 0.0);
    vec4 diffuse = Kd * diffuseProduct;

    float Ks = pow(max(dot(N, H), 0.0), shininess);
    vec4 specular = Ks * specularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0)
	    specular = vec4(0.0, 0.0, 0.0, 1.0);

    frag_color = ambient + diffuse + specular;
    frag_color.a = 1.0;
    frag_color = vec4(H, 1);
}
