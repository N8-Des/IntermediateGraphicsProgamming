#version 450                          
out vec4 FragColor;

in vec3 WorldPos;
in vec3 WorldNormal;
in vec3 normal;
in vec2 uv;
in vec4 LightSpaceFragPosition;

out vec4 color;

uniform vec3 _ViewPos;
uniform sampler2D _Texture1;
uniform sampler2D _ShadowMap;

uniform float _Time;

uniform float _MinBias;
uniform float _MaxBias;
uniform vec3 _LightPosition;


struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    float attenuation;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};

struct SpotLight
{
    vec3 color;
    vec3 position;
    vec3 direction;
    float intensity;
    float attenuation;
    float minAngle;
    float maxAngle;
};

struct Material
{
	vec3 color;
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};


float calculateShadow(float lightNormal)
{
    vec3 pos = LightSpaceFragPosition.xyz * 0.5f + 0.5f;
    //clamp positive z position
    if(pos.z > 1)
    {
        pos.z = 1;
    }

    float shadowBias = max(_MaxBias * (1.0 - lightNormal), _MinBias);

    //blur dem shadows
    //taken from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    float shadow = 0.0 ,depth;
    vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            depth = texture(_ShadowMap, pos.xy + vec2(x, y) * texelSize).r;
            shadow += (depth + shadowBias) < pos.z ? 0.0 : 1.0;
        }
    }

    return shadow / 9.0;
}

const int MAX_LIGHTS = 2;
uniform PointLight _PointLights[MAX_LIGHTS];
uniform DirectionalLight _DirectionalLight;
uniform SpotLight _SpotLight;
uniform Material _Material;

vec3 calculateDirectionalLight(DirectionalLight light)
{
    vec3 result = vec3(0);
    vec3 normal = normalize(WorldNormal);
    vec3 lightDir = -normalize(light.direction);
    vec3 viewDir = normalize(_ViewPos - WorldPos);
    vec3 halfway = normalize(lightDir + viewDir);

    float diff = _Material.diffuseK * max(dot(normal, lightDir), 0.0);
    float spec = _Material.specularK * pow(max(dot(normal, halfway), 0.0), _Material.shininess);
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 specular = spec * light.color * light.intensity;
    vec3 ambient = _Material.ambientK * light.intensity * light.color;
    result = (ambient + diffuse + specular);    
    return result;
};

vec3 calculateSpotLight(SpotLight light)
{
    vec3 result = vec3(0);
    vec3 normal = normalize(WorldNormal);
    vec3 lightDir = normalize(WorldPos - light.position);
    float dotTheta = dot(lightDir, light.direction);

    float i = (dotTheta - light.maxAngle) /  (light.minAngle - light.maxAngle);
    float intensity = light.intensity * i;
    vec3 viewDir = normalize(_ViewPos - WorldPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float attenuation = pow((light.attenuation / max(light.attenuation, distance(WorldPos, light.position))), 2);
    float diff = _Material.diffuseK * max(dot(normal, lightDir), 0.0);
    float spec = _Material.specularK * pow(max(dot(normal, halfway), 0.0), _Material.shininess);
    vec3 diffuse = diff * light.color * intensity * attenuation;
    vec3 specular = spec * light.color * intensity * attenuation;
    vec3 ambient = _Material.ambientK * intensity * light.color;
    result = (ambient + diffuse + specular) * _Material.color;      
    return result;
}

vec3 calculatePointLight(PointLight light)
{
    vec3 result = vec3(0);
    vec3 normal = normalize(WorldNormal);
    vec3 lightDir = normalize(light.position - WorldPos);
    vec3 viewDir = normalize(_ViewPos - WorldPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float attenuation = pow((light.attenuation / max(light.attenuation, distance(WorldPos, light.position))), 2);
    float diff = _Material.diffuseK * max(dot(normal, lightDir), 0.0);
    float spec = _Material.specularK * pow(max(dot(normal, halfway), 0.0), _Material.shininess);
    vec3 diffuse = diff * light.color * light.intensity * attenuation;
    vec3 specular = spec * light.color * light.intensity * attenuation;
    vec3 ambient = _Material.ambientK * light.intensity * light.color;
    result = (ambient + diffuse + specular) * _Material.color;      
    return result;
};

void main()
{
    vec3 result = vec3(0);
    vec3 lightDirection = normalize(_LightPosition - WorldPos);
    result += calculateDirectionalLight(_DirectionalLight);
    float shadow = calculateShadow(dot(lightDirection, normal));
    result *= shadow;
    vec4 lerpedTex = texture(_Texture1, uv);
    color = vec4(result, 1.0) * lerpedTex;
}
