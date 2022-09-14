#version 410 core

out vec4 FragColor;
in vec2 uv;

uniform sampler2D image;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{
    const float gamma = 1.2f;
    vec3 hdrColor = texture(image, uv).rgb;
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    hdrColor += bloomColor; //additive blending
    //tone mapping
    vec3 reinhard = hdrColor/(hdrColor + vec3(1.0));
    reinhard = vec3(1.0) - exp(-hdrColor * exposure);
    reinhard = pow(reinhard, vec3(1.0 / gamma));
   
    //vec3 reinhard = hdrColor;
    FragColor = vec4(reinhard, 1.0);
}