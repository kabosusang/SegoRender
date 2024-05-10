#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"
#include "../light.h"

struct VS_OUT{
    vec3 FragPos;
    vec3 FragPosWS;
    vec3 Normal;
    vec2 uv;
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
float ComputePCF(vec4 sc /*shadow croodinate*/, int r /*filtering range*/);
vec4 ComputeShadowCoord(vec3 WorldLightSpace);
float ShadowDepthProject(vec4 ShadowCoord, vec2 Offset);

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
        vec3 viewDir = normalize(LightData_st.dirLight.viewPos - fs_in.FragPosWS);
        vec3 result = CalcDirLight(color,LightData_st.dirLight,normal, viewDir);
 
        //计算阴影
        float ShadowFactor = 1.0;
        vec4 ShadowCoord  = ComputeShadowCoord(fs_in.FragPosWS);
        ShadowFactor = ComputePCF(ShadowCoord / ShadowCoord.w, 2);
        //ShadowFactor = ShadowDepthProject(ShadowCoord / ShadowCoord.w,  vec2(0.0, 0.0));
        outColor = vec4(result * ShadowFactor, 1.0);
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
    vec3 lightColor = vec3(1.0f);
    // Ambient
    vec3 ambient = 0.3 * color;
    // Diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    return (ambient + diffuse + specular) * color ;
}


float ShadowDepthProject(vec4 ShadowCoord, vec2 Offset)
{
	float ShadowFactor = 1.0;
	if ( ShadowCoord.z > -1.0 && ShadowCoord.z < 1.0 ) 
	{
		float Dist = texture(shadowMap, ShadowCoord.st + Offset ).r;
		if ( ShadowCoord.w > 0.0 && Dist < ShadowCoord.z ) 
		{
			ShadowFactor = 0.1;
		}
	}
	return ShadowFactor;
}


// Percentage Closer Filtering (PCF)
float ComputePCF(vec4 sc /*shadow croodinate*/, int r /*filtering range*/)
{
	ivec2 TexDim = textureSize(shadowMap, 0);
	float Scale = 1.5;
	float dx = Scale * 1.0 / float(TexDim.x);
	float dy = Scale * 1.0 / float(TexDim.y);

	float ShadowFactor = 0.0;
	int Count = 0;
	
	for (int x = -r; x <= r; x++)
	{
		for (int y = -r; y <= r; y++)
		{
			ShadowFactor += ShadowDepthProject(sc, vec2(dx*x, dy*y));
			Count++;
		}
	}
	return ShadowFactor / Count;
}


const mat4 BiasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0);

vec4 ComputeShadowCoord(vec3 WorldLightSpace)
{
	return (BiasMat * LightData_st.dirLight.lightSpaceMatrix * vec4(WorldLightSpace,1.0));
}
