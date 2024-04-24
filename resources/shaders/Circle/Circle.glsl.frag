#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../device.h"


layout (location = 0) in CircleOutput Input;

layout(location = 0) out vec4 outColor;

void main(){
   //Parameters
   //vec3 circleColor = vec3(0.85,0.35,0.2);

   //Calculate distance and fill circle with while
   float distance = 1.0 - length(Input.LocalPosition);
   float circle = smoothstep(0.0,Input.Fade,distance);
   circle *= smoothstep(Input.Thickness + Input.Fade,Input.Thickness,distance);
   
   //Set output color 
   outColor = Input.color;
   outColor.a *= circle;

   // if (circle == 0.0) 
   // dircard;
   // EntityID = InID;
}