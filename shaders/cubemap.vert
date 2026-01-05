#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 LocalPos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    LocalPos = aPos;
    mat4 rotView = mat4(mat3(view)); // 移除位移分量
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww; // 深度设为最大值（天空盒技巧）
}
