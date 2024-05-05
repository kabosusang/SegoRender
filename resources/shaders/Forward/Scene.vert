#version 450

layout (location = 0) out vec2 outUV;

void main()
{
    // 由于我们假设这是一个四边形网格，所以gl_VertexIndex的范围是[0, 3]
    // 我们使用mod函数来确保索引在[0, 3]范围内
    ivec2 vertexIndex = ivec2(mod(gl_VertexIndex, 4));

    // 映射到整个屏幕的四个角点
    vec2 screenVertices[4] = vec2[](
        vec2(1.0, 1.0), // 右下角
        vec2(1.0, -1.0), // 右上角
        vec2(-1.0, -1.0), // 左上角
        vec2(-1.0, 1.0)  // 左下角
    );

    // 选择当前顶点的UV坐标
    outUV = screenVertices[vertexIndex.x];

    // 映射gl_VertexIndex的值到[-1, 1]的范围以用于屏幕坐标
    gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
}