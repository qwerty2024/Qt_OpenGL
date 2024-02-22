struct materialProperty
{
    vec3 diffuseColor;
    vec3 ambienceColor;
    vec3 specularColor;
    float shinnes;
};

uniform sampler2D u_texture;
uniform highp vec4 u_lightPosition;
uniform highp float u_lightPower;
uniform highp materialProperty u_materialProperty;
uniform bool u_isUsingDiffuseMap;
varying highp vec4 v_position;
varying highp vec2 v_texcoord;
varying highp vec3 v_normal;

void main(void)
{
    vec4 resultColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 eyePosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 diffMatColor = texture2D(u_texture, v_texcoord);
    vec3 eyeVect = normalize(v_position.xyz - eyePosition.xyz);
    vec3 lightVect = normalize(v_position.xyz - u_lightPosition.xyz);
    vec3 reflectLight = normalize(reflect(lightVect, v_normal));
    float len = length(v_position.xyz - eyePosition.xyz);
    float specularFactor = u_materialProperty.shinnes;
    float ambientFactor = 0.1f;

    if (u_isUsingDiffuseMap == false) diffMatColor = vec4(u_materialProperty.diffuseColor, 1.0f);

    vec4 diffColor = diffMatColor * u_lightPower * max(0.0f, dot(v_normal, -lightVect));// / (1.0 + 0.25 * pow(len, 2.0));
    resultColor += diffColor;

    vec4 ambientColor = ambientFactor * diffMatColor;
    resultColor += ambientColor * vec4(u_materialProperty.ambienceColor, 1.0f);

    vec4 specularColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * u_lightPower * pow(max(0.0f, dot(reflectLight, -eyeVect)), specularFactor);// / (1.0 + 0.25 * pow(len, 2.0));
    resultColor += specularColor * vec4(u_materialProperty.specularColor, 1.0f);

    gl_FragColor = resultColor;
}
