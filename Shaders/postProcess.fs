#version 410 core

out vec4 FragColor;
uniform sampler2D image;
uniform sampler2D hdrBuffer;
in vec2 uv;

uniform bool toneMap;
uniform bool invert;
uniform bool grey;
uniform bool gamma;
uniform float exposure;

void main()
{
    vec3 result = texture(image, uv).rgb;

    //tone mapping
    if (toneMap == true) 
    {
        result = result / (result + vec3(1.0f));
        FragColor = vec4(result, 1.0);
    }
    else if (toneMap == false)
    {
        FragColor = vec4(result, 1.0);
    }

    //inverting textures and scene
    if (invert == true)
    {
        result = 1 - texture(image, uv).rgb;
        FragColor = vec4(result, 1.0);
    }
    else if (invert == false)
    {
        FragColor = vec4(result, 1.0);
    }

    //gamma tone mapping
    if (gamma == true)
    {
        const float gamma = 2.2;
        vec3 hdrColor = texture(hdrBuffer, uv).rgb;
  
        // reinhard tone mapping
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
        //exposure tone mapping
        mapped = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma correction 
        mapped = pow(mapped, vec3(1.0 / gamma));
        result = mapped;
  
        FragColor = vec4(result, 1.0);
    }
    else if (gamma == false)
    {
        FragColor = vec4(result, 1.0);
    }

    //grey scaling textures and scene
    if (grey == true)
    {
        float avg = (result.x + result.y + result.z)/3;
        FragColor = vec4(vec3(avg), 1.0);
    }
    else if (grey == false)
    {
        FragColor = vec4(result, 1.0);
    }


}