#version 460 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light; // some direction light
uniform sampler2D tex0;
out vec4 fragColor;

void main()
{
    vec4 diffuseColor = texture(tex0, uv);
    vec3 n = normalize(norm);
    vec3 l = normalize(light);

    float diffuseIntensity = clamp(dot(n, l), 0, 1);
    fragColor = diffuseColor * diffuseIntensity;
}