#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"
#include "../light.h"

// Uniforms
layout(set = 0,binding = 0) uniform Light_st{
    LightObj LightData_st;
};

layout(set = 0,binding = 1) uniform sampler2D AlbedoSpecTexture;
layout(set = 0,binding = 2) uniform sampler2D PositionTexture;
layout(set = 0,binding = 3) uniform sampler2D NormalTexture;
layout(set = 0,binding = 4) uniform sampler2D ShadowCoordTexture;

layout(set = 0,binding = 5) uniform sampler2D shadowMap;


layout (location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
//Function
vec3 CalcDirLight(DirLight light,vec3 normal, vec3 viewDir);
float ComputePCF(vec4 sc /*shadow croodinate*/, int r /*filtering range*/);
float ShadowDepthProject(vec4 ShadowCoord, vec2 Offset);
// Manual resolve for MSAA samples 

void main(){
    ivec2 attDim = textureSize(PositionTexture,0);
    ivec2 UV = ivec2(inUV * attDim);
    // Ambient part
    vec4 alb = texture(AlbedoSpecTexture, UV);
	vec3 fragColor = vec3(0.0);
    //计算阴影
    float ShadowFactor = 1.0;
    vec4 shadowCoord = texture(ShadowCoordTexture, UV);
    ShadowFactor = ComputePCF(shadowCoord / shadowCoord.w, 2);


    if (LightData_st.lightSetting.UseLight != 0)
    {
        vec3 position = texture(PositionTexture, UV).rgb;
        vec3 normal = normalize(texture(NormalTexture, UV).rgb);
        vec3 viewDir = normalize(LightData_st.dirLight.viewPos - position);
        // Do lighting calculations
        fragColor = CalcDirLight(LightData_st.dirLight, normal, viewDir);
        vec3 ambient = 0.3 * alb.rgb;
        fragColor = ambient + fragColor;
        outColor = vec4(fragColor * ShadowFactor,1.0);

    }else{
        fragColor = alb.rgb;
        outColor = vec4(fragColor,1.0);
    }

}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightColor = vec3(1.0f);
  
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

    return (diffuse + specular) ;
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
