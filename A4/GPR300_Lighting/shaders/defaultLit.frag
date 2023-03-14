#version 450                          
out vec4 FragColor;

in vec3 WorldPos;
in vec3 WorldNormal;
in vec2 uv;
in mat3 tangent;

out vec4 color;

uniform vec3 _ViewPos;

uniform sampler2D _Texture1;
uniform sampler2D _Texture2;
uniform sampler2D _Texture1Normal;
uniform sampler2D _Texture2Normal;

uniform float _Time;
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


const int MAX_LIGHTS = 2;
uniform PointLight _PointLights[MAX_LIGHTS];
uniform DirectionalLight _DirectionalLight;
uniform SpotLight _SpotLight;
uniform Material _Material;
uniform float _NormalStrength;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 norm)
{
    vec3 result = vec3(0);
    vec3 normal = norm;
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

vec3 calculateSpotLight(SpotLight light, vec3 norm)
{
    vec3 result = vec3(0);
    vec3 normal = normalize(norm);
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

vec3 calculatePointLight(PointLight light, vec3 norm)
{
    vec3 result = vec3(0);
    vec3 normal = normalize(norm);
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
    vec3 norm = (texture(_Texture1Normal, uv).rgb * 2) - 1;
    norm *= vec3(_NormalStrength, _NormalStrength, 1);
    norm *= tangent;
    result += calculateDirectionalLight(_DirectionalLight, norm);
    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        result += calculatePointLight(_PointLights[i], norm);
    }
    vec2 newUV = vec2(uv.x + _Time, uv.y);
    float t = clamp(sin(_Time), 0, 1);
    vec4 lerpedTex = texture(_Texture1, uv) * (1.0f - t) + texture(_Texture2, uv) * t;
    color = vec4(result, 1.0) * texture(_Texture1, uv);
}
