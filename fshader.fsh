struct materialProperty
{
    vec3 diffuseColor;
    vec3 ambienceColor;
    vec3 specularColor;
    float shinnes;
};

uniform sampler2D u_diffuseMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_shadowMap;
uniform highp vec4 u_lightPosition;
uniform highp float u_lightPower;
uniform highp materialProperty u_materialProperty;
uniform bool u_isUsingDiffuseMap;
uniform bool u_isUsingNormalMap;
varying highp vec4 v_position;
varying highp vec2 v_texcoord;
varying highp vec3 v_normal;
varying highp mat3 v_tbnMatrix;
varying highp vec4 v_lightDirection;
varying highp vec4 v_positionLightMatrix;

float SampleShadowMap(sampler2D map, vec2 coords, float compare)
{
    vec4 v = texture2D(map, coords);
    float value = v.x * 255.0f + (v.y * 255.0f + (v.z * 255.0f + v.w) / 255.0f) / 255.0f;
    return step(compare, value);
}

float SampleShadowMapLinear(sampler2D map, vec2 coords, float compare, vec2 texelSize)
{
    vec2 pixelPos = coords / texelSize + 0.5f;
    vec2 fractPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fractPart) * texelSize;

    float blTexel = SampleShadowMap(map, startTexel, compare);
    float brTexel = SampleShadowMap(map, startTexel + vec2(texelSize.x, 0.0f), compare);
    float tlTexel = SampleShadowMap(map, startTexel + vec2(0.0f, texelSize.y), compare);
    float trTexel = SampleShadowMap(map, startTexel + texelSize, compare);

    float mixA = mix(blTexel, tlTexel, fractPart.y);
    float mixB = mix(brTexel, trTexel, fractPart.y);

    return mix(mixA, mixB, fractPart.x);
}

float SampleShadowMapPCF(sampler2D map, vec2 coords, float compare, vec2 texelSize)
{
    float result = 0.0f;
    for (float y = -1.0f; y < 1.0f; y+= 1.0f)
    {
        for (float x = -1.0f; x < 1.0f; x+= 1.0f)
        {
            vec2 offset = vec2(x, y) * texelSize;
            result += SampleShadowMapLinear(map, coords + offset, compare, texelSize);
        }
    }

    return result / 9.0f;
}

float CalcShadowAmount(sampler2D map, vec4 initialShadowCoords)
{
    vec3 tmp = v_positionLightMatrix.xyz / v_positionLightMatrix.w;
    tmp = tmp * vec3(0.5f) + vec3(0.5f);
    float offset = 2.0;
    offset *= dot(v_normal, v_lightDirection.xyz);

    return SampleShadowMapPCF(u_shadowMap, tmp.xy, tmp.z * 255.0f + offset, vec2(1.0f / 1024.0f));
}

void main(void)
{
    highp float shadowCoef = CalcShadowAmount(u_shadowMap, v_positionLightMatrix);
    vec4 resultColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 eyePosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 diffMatColor = texture2D(u_diffuseMap, v_texcoord);

    vec3 usingNormal = v_normal;
    if (u_isUsingNormalMap == true) usingNormal = normalize(texture2D(u_normalMap, v_texcoord).rgb * 2.0f - 1.0f);

    vec3 eyeVect = normalize(v_position.xyz - eyePosition.xyz);
    if (u_isUsingNormalMap == true) eyeVect = normalize(v_tbnMatrix * eyeVect);

    vec3 lightVect = normalize(v_lightDirection.xyz);
    if (u_isUsingNormalMap == true) lightVect = normalize(v_tbnMatrix * lightVect);

    vec3 reflectLight = normalize(reflect(lightVect, usingNormal));
    float len = length(v_position.xyz - eyePosition.xyz);
    float specularFactor = u_materialProperty.shinnes;
    float ambientFactor = 0.1f;

    if (u_isUsingDiffuseMap == false) diffMatColor = vec4(u_materialProperty.diffuseColor, 1.0f);

    vec4 diffColor = diffMatColor * u_lightPower * max(0.0f, dot(usingNormal, -lightVect));// / (1.0 + 0.25 * pow(len, 2.0));
    resultColor += diffColor;

    vec4 ambientColor = ambientFactor * diffMatColor;
    resultColor += ambientColor * vec4(u_materialProperty.ambienceColor, 1.0f);

    vec4 specularColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * u_lightPower * pow(max(0.0f, dot(reflectLight, -eyeVect)), specularFactor);// / (1.0 + 0.25 * pow(len, 2.0));
    resultColor += specularColor * vec4(u_materialProperty.specularColor, 1.0f);

    shadowCoef += 0.2f;
    if (shadowCoef > 1.0f) shadowCoef = 1.0f;
    gl_FragColor = resultColor * shadowCoef;
}






























