#version 330 core
out vec4 FragColor;
in vec3 LocalPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1/(2π) 和 1/π

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    // 将局部坐标转换为球面UV
    vec2 uv = SampleSphericalMap(normalize(LocalPos));
    
    // 从HDR贴图采样
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
