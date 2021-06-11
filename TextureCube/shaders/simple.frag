#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D specTexSampler;
layout(binding = 2) uniform sampler2D diffTexSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    //Since we are in view space
    vec3 v = vec3(0.0);
    // This is a directional light
    vec3 l = normalize(vec3(0.0, 0.75, 1.0));
    vec3 n = normalize(fragNormal);
    vec3 r = normalize(reflect(-l, n));
    vec3 h = normalize(l + v);
    //Material from texture
    vec3 Ka = 0.1 * texture(diffTexSampler, fragTexCoord).rgb;
    vec3 Kd = 0.9 * texture(diffTexSampler, fragTexCoord).rgb;
    vec3 Ks = texture(specTexSampler, fragTexCoord).rgb;
    float alpha = 4.0;
    //Light's color (all components are white)
    vec3 La = vec3(1.0);
    vec3 Ls = vec3(1.0);
    vec3 Ld = vec3(1.0);
    //Phong's shading
    vec3 ambient = Ka * La;
    vec3 diffuse = Kd * Ld * max(0.0, dot(n, l));
    //Well, technically it is Blin - Phong
    vec3 specular = Ks * Ls * pow(max(0.0, dot(n, h)), alpha);
    //Final color for this fragment
    outColor = vec4(ambient + specular + diffuse, 1.0);
}
