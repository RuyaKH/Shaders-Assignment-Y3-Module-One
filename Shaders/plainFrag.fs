#version 410 core
#define numPointLights 3

vec3 getDirectionalLight(vec3 norm, vec3 viewDir, vec2 texCoords, float shadow);
vec3 getPointLight(vec3 norm, vec3 viewDir, vec3 fragPos, int i, vec2 texCoords, float shadow);
vec3 getSpotLight(vec3 norm, vec3 viewDir, vec3 fragPos, vec2 texCoords);
vec3 getRimLighting(vec3 norm, vec3 viewDir);
vec2 ParallaxMapping(vec2 texCorods, vec3 viewDir);
vec2 SteepParallaxMapping(vec2 texCoords, vec3 viewDir);
float calcShadow(vec4 fragPosLightSpace);

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 brightColor;

in vec3 normal ;
in vec3 posWS;
in vec2 uv;
in mat3 TBN;

struct pointLight{
    vec3 position;
    vec3 color;
    float Kc;
    float Kl;
    float Ke;
};

struct spotLight{
    vec3 position;
    vec3 direction;
    vec3 color;
    float Kc;
    float Kl;
    float Ke;

    float innerRad;
    float outerRad;
};

uniform vec3 lightCol;
uniform vec3 lightDir;
uniform vec3 objectCol;
uniform vec3 viewPos;
uniform vec3 rimColor;
uniform float rimPower;
uniform pointLight pLight[numPointLights];
uniform spotLight sLight;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalMap;
uniform sampler2D disMap;
uniform sampler2D bumpMap;
uniform sampler2D depthMap;

uniform mat4 lightSpaceMatrix;

uniform bool DL;
uniform bool PL;
uniform bool SL;
uniform bool RL;

uniform float PXscale;
uniform float bloomBrightness;

float ambientFactor = 0.5;
float shine = 128;
float specularStrength = 0.4;

void main()
{
    vec4 posLS = lightSpaceMatrix * vec4(posWS,1.0);
    float shadow = calcShadow(posLS);
    //shadow = shadow*0.5;
    vec2 texCoords = uv;
    vec3 norm = texture(normalMap, texCoords).xyz;
    norm = norm*2.0 - 1.0;
    norm = normalize(TBN*norm);
    vec3 viewDir = normalize(viewPos - posWS);
    vec3 result = vec3(0.0);

    texCoords = ParallaxMapping(texCoords, viewDir);

    if(RL)
        result = result + getRimLighting(norm, viewDir);
    if(DL)
        result = result + getDirectionalLight(norm, viewDir, texCoords, shadow);
    if(PL)
        for(int i=0; i<numPointLights; i++) 
        { 
            result = result + getPointLight(norm, viewDir, posWS, i, texCoords, shadow); 
        }
        result = result;
    if(SL)
        result = result + getSpotLight(norm, viewDir, posWS, texCoords);

    //layout 1
    FragColor = vec4(result, 1.0f); 
    //layout 2
    //float brightness = (result.x - result.y + result.z)/3;
    float brightness = max(max(result.x, result.y), result.z);
    if (brightness > bloomBrightness)
        brightColor = FragColor;
    else
        brightColor = vec4(vec3(0.0), 1.0);
}

//rim lighting
vec3 getRimLighting(vec3 norm, vec3 viewDir)
{
    float f = 1.0 - dot(norm, viewDir);
    f = smoothstep(0.0, 1.0, f);
    f = pow(f, rimPower);
    vec3 result = f * rimColor;
    return result;
}

////directional light////
vec3 getDirectionalLight(vec3 norm, vec3 viewDir, vec2 texCoords, float shadow) {
    //ambient lighting
    vec3 diffMapColor = texture(diffuseTexture, texCoords).xyz;
    vec3 ambientColor = lightCol*diffMapColor*ambientFactor;

    //diffuse lighting
    float diffuseFactor = dot(norm, -lightDir);
    diffuseFactor = max(diffuseFactor, 0.0);
    vec3 diffuseColor = lightCol*diffMapColor*diffuseFactor;

    //specular
    float specMapColor = texture(specularTexture, texCoords).x;
    vec3 reflectDir = reflect(lightDir, norm);
    float specularFactor = dot(viewDir, reflectDir);
    specularFactor = max(specularFactor, 0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColor = lightCol * specularFactor * specMapColor;

    vec3 result = ambientColor + (1.0 - shadow) * (diffuseColor + specularColor);
    return result;
}

vec3 getPointLight(vec3 norm, vec3 viewDir, vec3 fragPos, int i, vec2 texCoords, float shadow) {
    ////point light////
    float dist = length(pLight[i].position - fragPos);
    float attn = 1.0/(pLight[i].Kc + (pLight[i].Kl*dist) + (pLight[i].Ke*(dist*dist)));
    vec3 pLightDir = normalize(pLight[i].position - fragPos);

    //ambient
    vec3 diffMapColor = texture(diffuseTexture, texCoords).xyz;
    vec3 ambientColor = pLight[i].color * diffMapColor * ambientFactor;
    ambientColor = ambientColor * attn;

    //diffuse
    float diffuseFactor = dot(norm, pLightDir);
    diffuseFactor = max(diffuseFactor,0.0);
    vec3 diffuseColor = pLight[i].color * diffMapColor * diffuseFactor;
    diffuseColor = diffuseColor * attn;

    //specular
    float specMapColor = texture(specularTexture, texCoords).x;
    vec3 reflectDir = reflect(pLightDir, norm);
    float specularFactor = dot(viewDir, reflectDir);
    specularFactor = max(specularFactor, 0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColor = pLight[i].color * specularFactor * specMapColor;
    specularColor = specularColor * attn;
    //vec3 pointLightResult = ambientColor + (1.0 - shadow) * (diffuseColor + specularColor);
    vec3 pointLightResult = ambientColor + diffuseColor + specularColor;

    return pointLightResult;
} 


vec3 getSpotLight(vec3 norm, vec3 viewDir, vec3 fragPos, vec2 texCoords) {
    ////spot light////
    float dist = length(sLight.position - fragPos);
    float attn = 1.0/(sLight.Kc + (sLight.Kl*dist) + (sLight.Ke*(dist*dist)));
    vec3 sLightDir = normalize(sLight.position - fragPos);

    vec3 diffMapColor = texture(diffuseTexture, texCoords).xyz;

    //diffuse
    float diffuseFactor = dot(norm, sLightDir);
    diffuseFactor = max(diffuseFactor,0.0);
    vec3 diffuseColor = sLight.color * diffMapColor * diffuseFactor;
    diffuseColor = diffuseColor * attn;

    //specular
    float specMapColor = texture(specularTexture, texCoords).x;
    vec3 reflectDir = reflect(sLightDir, norm);
    float specularFactor = dot(viewDir, reflectDir);
    specularFactor = max(specularFactor, 0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColor = lightCol * specularFactor * specMapColor;
    specularColor = specularColor * attn;

    float theta = dot(-sLightDir, normalize(sLight.direction));
    float denom = (sLight.innerRad - sLight.outerRad);
    float illum = (theta - sLight.outerRad) / denom;
    illum = clamp(illum, 0.0, 1.0);
    diffuseColor = diffuseColor * illum;
    specularColor = specularColor * illum;

    vec3 spotLightResult = diffuseColor + specularColor;
    return spotLightResult;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    float height = texture(disMap, texCoords).r;
    return texCoords - (viewDir.xy) * (height * PXscale);
    //return vec2(0.5);
}

vec2 SteepParallaxMapping(vec2 texCoords, vec3 viewDir) {
    float numLayers = 10;
    float layerDepth = 1.0/numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy * PXscale;
    vec2 deltaTexCoords = P / numLayers;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(disMap, currentTexCoords).r;
    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(bumpMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    return currentTexCoords;
}

float calcShadow(vec4 fragPosLightSpace)
{
    vec2 texelSize = 1.0/textureSize(depthMap,0);
    //perform perspective divide values in range [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    //sample from shadow map
    float closestDepth = texture(depthMap, projCoords.xy).r;
    //get depth of current fragment from lights perspective
    float currentDepth = projCoords.z;
    //check whether current frag pos is in shadow
    float shadow = 0.0;
    float bias = 0.015;
    for (int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            float pcf = texture(depthMap, projCoords.xy + vec2(i, j) * texelSize).r;
            if (currentDepth-bias > pcf)
                shadow += 1.0;
            if (projCoords.z > 1.0)
                shadow = 0.0;
        }
    }
    shadow = shadow/9;

    return shadow;
}