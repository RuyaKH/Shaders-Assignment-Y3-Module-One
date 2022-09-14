#version 330 core

float LinearizeDepth(float depth) ;

out vec4 FragColor;

uniform sampler2D image;

in vec2 uv;

const float near_plane = 1;
const float far_plane = 100;

void main()
{             
 
    float depth = texture(image,uv).r ;
    //light perspective - orthographic
    FragColor = vec4(vec3(depth), 1.0);
    //camera's perespective 
    //FragColor = vec4(vec3(LinearizeDepth(depth) / far_plane), 1.0); // perspective
    
}

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}