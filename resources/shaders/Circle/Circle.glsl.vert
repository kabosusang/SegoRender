#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"

//For Lighting Circle

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inLocalPos;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inThickness;
layout(location = 4) in float inFade;

layout(location = 0) out CircleOutput output;

layout(Push_constant) uniform PushConstant{
    mat4 mvp;
} pc;

void main(){
    output.LocalPosition= LocalPos;
    output.color = inColor;
    output.Thickness = inThickness;
    output.Fade = inFade;

    gl_Position = pc.mvp * vec4(inPosition, 1.0);

}
