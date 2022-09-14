#version 420 core

out vec4 FragColor;

uniform sampler2D image;
uniform int horz;
in vec2 uv;


float weights[] = {
0.04558874821397744,
0.04469312448750337,
0.04323941779418976,
0.041283252881657614,
0.03889759757448154,
0.036168179845020715,
0.03318836107227639,
0.030053872747518973,
0.026857814141164747,
0.023686258756849855,
0.020614736353370078,
0.01770575674976994,
0.015007434722657838,
0.012553174649950505,
0.01036228951631275,
0.008441368488393772,
0.006786173670846112,
0.005383839343129145,
0.004215162373402302,
0.003256804954062264,
0.002483273675394342
};

int r = 21;
float bias = 1.1f;

void main()
{
    vec2 textureOffset = 1.0/textureSize(image, 0);
    vec3 color = texture(image, uv).rgb * (weights[0]*bias);
    if (horz == 1) 
    {
        for(int i = 1; i < r; i++) 
        {
            color = color + texture(image, uv + vec2(textureOffset.x * i, 0.0)).rgb * (weights[i]*bias);
            color = color + texture(image, uv - vec2(textureOffset.x * i, 0.0)).rgb * (weights[i]*bias);
        }
    }
    else 
    {
        for(int i = 1; i < r; i++) 
        {
            color = color + texture(image, uv + vec2(0.0, textureOffset.y * i)).rgb * (weights[i]*bias);
            color = color + texture(image, uv - vec2(0.0, textureOffset.y * i)).rgb * (weights[i]*bias);
        }
    }

    FragColor = vec4(color, 1.0);
}