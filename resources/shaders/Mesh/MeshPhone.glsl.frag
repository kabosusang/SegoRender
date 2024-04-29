#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"
#include "../light.h"

struct VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 uv;
    vec4 FragPosLightSpace;
};

#define MAX_POINT_LIGHTS 8 //TOOD: Dynamic Poit light


layout(location = 0) in VS_OUT fs_in;


layout(location = 0) out vec4 outColor;

// Uniforms (push_constant)
layout(push_constant) uniform Material{ 
   layout(offset = 64) Material_toshader material;
} mt;


// Uniforms 
layout(set = 0,binding = 1) uniform Light_st{
    LightObj LightData_st;
};

layout(set = 0,binding = 2) uniform sampler2D texSampler;
layout(set = 0,binding = 3) uniform sampler2D shadowMap;


//Function
vec3 CalcDirLight(vec3 color,DirLight light,vec3 normal, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace);


void main(){
    
    if (LightData_st.lightSetting.UseLight != 0)
    {
        vec3 color;
        if (mt.material.UseSampler != 0){
            color = texture(texSampler, fs_in.uv).rgb ;
        }else{
            color = mt.material.baseColorFactor.rgb;
        }
        vec3 normal = normalize(fs_in.Normal);
        vec3 viewDir = normalize(LightData_st.dirLight.viewPos - fs_in.FragPos);
        vec3 result = CalcDirLight(color,LightData_st.dirLight,normal, viewDir);
        outColor = vec4(result, 1.0);
    }else{
        if (mt.material.UseSampler != 0){
            outColor = texture(texSampler, fs_in.uv);
        }else{
            outColor = mt.material.baseColorFactor;
        }
    }

}

vec3 CalcDirLight(vec3 color,DirLight light, vec3 normal, vec3 viewDir)
{

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
 
    vec3 reflectDir = reflect(-lightDir, normal);
    // combine results
    vec3 ambient = 0.05 * color;
    vec3 diffuse = diff * color;

    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec;
    
    // 计算阴影
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    //return (ambient + diffuse + specular);
    return lighting;
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片段是否在阴影中
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}