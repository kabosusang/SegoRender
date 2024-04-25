#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"
#include "../../../SegEngine/resource/asset/base/Light.h"

struct VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 uv;
};

#define MAX_POINT_LIGHTS 8 //TOOD: Dynamic Poit light


layout(location = 0) in VS_OUT fs_in;

layout(set = 0,binding = 0) uniform sampler2D texSampler;
layout(location = 0) out vec4 outColor;


// Uniforms (push_constant)
layout(push_constant) uniform Material{ 
   layout(offset = 64) Material_toshader material;
} mt;

layout(set = 0,binding = 1) uniform Light_st{
    LightObj LightData_st;
};


vec3 CalcDirLight(DirLight light,vec3 normal, vec3 viewDir);

void main(){
    
    if (LightData_st.lightSetting.UseLight != 0)
    {
        vec3 normal = normalize(fs_in.Normal);
        vec3 viewDir = normalize(LightData_st.dirLight.viewPos - fs_in.FragPos);
        vec3 result = CalcDirLight(LightData_st.dirLight,normal, viewDir);
        outColor = vec4(result, 1.0);
    }else{
        if (mt.material.UseSampler != 0){
            outColor = texture(texSampler, fs_in.uv);
        }else{
            outColor = mt.material.baseColorFactor;
        }
    }

}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 color;
    if (mt.material.UseSampler != 0){
        color = texture(texSampler, fs_in.uv).rgb ;
    }else{
        color = mt.material.baseColorFactor.rgb;
    }

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
    
    return  (ambient + diffuse + specular);
}
