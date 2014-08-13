// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#define PATCH_BLEND_BEGIN		800
#define PATCH_BLEND_END			20000

// Shading parameters
cbuffer cbShading : register(b2)
{
	// Water-reflected sky color
	float3		g_SkyColor			: packoffset(c0.x);
	// The color of bottomless water body
	float3		g_WaterbodyColor	: packoffset(c1.x);

	// The strength, direction and color of sun streak
	float		g_Shineness			: packoffset(c1.w);
	float3		g_SunDir			: packoffset(c2.x);
	float3		g_SunColor			: packoffset(c3.x);
	
	// The parameter is used for fixing an artifact
	float3		g_BendParam			: packoffset(c4.x);

	// Perlin noise for distant wave crest
	float		g_PerlinSize		: packoffset(c4.w);
	float3		g_PerlinAmplitude	: packoffset(c5.x);
	float3		g_PerlinOctave		: packoffset(c6.x);
	float3		g_PerlinGradient	: packoffset(c7.x);

	// Constants for calculating texcoord from position
	float		g_TexelLength_x2	: packoffset(c7.w);
	float		g_UVScale			: packoffset(c8.x);
	float		g_UVOffset			: packoffset(c8.y);
};

// Per draw call constants
cbuffer cbChangePerCall : register(b4)
{
	// Transform matrices
	float4x4	g_matLocal;
	float4x4	g_matWorldViewProj;
	float4x4    g_matWorld;

	// Reflection matrix from environment
	float4x4    g_matWorldReflectionViewProj;

	// Misc per draw call constants
	float2		g_UVBase;
	float2		g_PerlinMovement;
	float3		g_LocalEye;

	// The sea level elevation of the ocean
	float       g_SeaLevel;
}


//-----------------------------------------------------------------------------------
// Texture & Samplers
//-----------------------------------------------------------------------------------
Texture2D	g_texDisplacement	: register(t0); // FFT wave displacement map in VS
Texture2D	g_texPerlin			: register(t1); // FFT wave gradient map in PS
Texture2D	g_texGradient		: register(t2); // Perlin wave displacement & gradient map in both VS & PS
Texture1D	g_texFresnel		: register(t3); // Fresnel factor lookup table
Texture2D   g_ReflectionMap     : register(t4); // Reflection map for the scene
Texture2D   g_RefractionMap     : register(t5); // Refraction map for the scene
Texture2D   g_NormalMap         : register(t6); // Normal map for the scene
TextureCube	g_texReflectCube	: register(t7); // A small skybox cube texture for reflection

// FFT wave displacement map in VS, XY for choppy field, Z for height field
SamplerState g_samplerDisplacement	: register(s0);

// Perlin noise for composing distant waves, W for height field, XY for gradient
SamplerState g_samplerPerlin	: register(s1);

// FFT wave gradient map, converted to normal value in PS
SamplerState g_samplerGradient	: register(s2);

// Fresnel factor lookup table
SamplerState g_samplerFresnel	: register(s3);

// A small sky cubemap for reflection
SamplerState g_samplerCube		: register(s4);

// Reflection and Refraction sampler
SamplerState RRSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;//MIRROR;
	AddressV = WRAP;//MIRROR;
};


//-----------------------------------------------------------------------------
// Name: OceanSurfVS
// Type: Vertex shader                                      
// Desc: Ocean shading vertex shader. Check SDK document for more details
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position	                : SV_POSITION;
    float2 TexCoord	                : TEXCOORD0;
    float3 LocalPos	                : TEXCOORD1;
	float4 reflectionMapSamplingPos : TEXCOORD2;
	float4 refractionMapSamplingPos : TEXCOORD3;
	float4 position3D               : TEXCOORD4;
	float3 viewDirection            : TEXCOORD5;
};

VS_OUTPUT OceanSurfVS(float4 vPos : POSITION)
{
	VS_OUTPUT Output;

	// Local position
	float4 pos_local = mul(float4(vPos.xy, 0, 1), g_matLocal);
	// UV
	float2 uv_local = pos_local.xy * g_UVScale + g_UVOffset;

	// Blend displacement to avoid tiling artifact
	float3 eye_vec = pos_local.xyz - g_LocalEye;
	float dist_2d = length(eye_vec.xy);
	float blend_factor = (PATCH_BLEND_END - dist_2d) / (PATCH_BLEND_END - PATCH_BLEND_BEGIN);
	blend_factor = clamp(blend_factor, 0, 1);

	// Add perlin noise to distant patches
	float perlin = 0;
	if (blend_factor < 1)
	{
		float2 perlin_tc = uv_local * g_PerlinSize + g_UVBase;
		float perlin_0 = g_texPerlin.SampleLevel(g_samplerPerlin, perlin_tc * g_PerlinOctave.x + g_PerlinMovement, 0).w;
		float perlin_1 = g_texPerlin.SampleLevel(g_samplerPerlin, perlin_tc * g_PerlinOctave.y + g_PerlinMovement, 0).w;
		float perlin_2 = g_texPerlin.SampleLevel(g_samplerPerlin, perlin_tc * g_PerlinOctave.z + g_PerlinMovement, 0).w;
		
		perlin = perlin_0 * g_PerlinAmplitude.x + perlin_1 * g_PerlinAmplitude.y + perlin_2 * g_PerlinAmplitude.z;
	}

	// Displacement map
	float3 displacement = 0;
	if (blend_factor > 0)
		displacement = g_texDisplacement.SampleLevel(g_samplerDisplacement, uv_local, 0).xyz;
	displacement = lerp(float3(0, 0, perlin), displacement, blend_factor);
	pos_local.x += displacement.x;
	pos_local.y += displacement.y;
	pos_local.z += displacement.z + g_SeaLevel;

	// Transform
	Output.Position = mul(pos_local, g_matWorldViewProj);

	Output.LocalPos = pos_local.xyz;
	
	// Calculate refraction and reflection view matrices
	Output.reflectionMapSamplingPos = mul(pos_local, g_matWorldReflectionViewProj);
	Output.refractionMapSamplingPos = mul(pos_local, g_matWorldViewProj);
	Output.position3D = mul(pos_local, g_matWorld);

	//
	// Calculate the camera's view direction for fresnel calculations
	//

	// Calculate the position of the vertex in the world.
    float3 worldPosition = mul(pos_local, g_matWorld);

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    Output.viewDirection = g_LocalEye.xyz - worldPosition.xyz;

    // Normalize the viewing direction vector.
    Output.viewDirection = normalize(Output.viewDirection);

	// Pass thru texture coordinate
	Output.TexCoord = uv_local;

	return Output; 
}


//-----------------------------------------------------------------------------
// Name: OceanSurfPS
// Type: Pixel shader                                      
// Desc: Ocean shading pixel shader. Check SDK document for more details
//-----------------------------------------------------------------------------
float4 OceanSurfPS(VS_OUTPUT In) : SV_Target
{
	// Calculate eye vector.
	float3 eye_vec = g_LocalEye - In.LocalPos;
	float3 eye_dir = normalize(eye_vec);
	

	// --------------- Blend perlin noise for reducing the tiling artifacts

	// Blend displacement to avoid tiling artifact
	float dist_2d = length(eye_vec.xy);
	float blend_factor = (PATCH_BLEND_END - dist_2d) / (PATCH_BLEND_END - PATCH_BLEND_BEGIN);
	blend_factor = clamp(blend_factor * blend_factor * blend_factor, 0, 1);

	// Compose perlin waves from three octaves
	float2 perlin_tc = In.TexCoord * g_PerlinSize + g_UVBase;
	float2 perlin_tc0 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.x + g_PerlinMovement : 0;
	float2 perlin_tc1 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.y + g_PerlinMovement : 0;
	float2 perlin_tc2 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.z + g_PerlinMovement : 0;

	float2 perlin_0 = g_texPerlin.Sample(g_samplerPerlin, perlin_tc0).xy;
	float2 perlin_1 = g_texPerlin.Sample(g_samplerPerlin, perlin_tc1).xy;
	float2 perlin_2 = g_texPerlin.Sample(g_samplerPerlin, perlin_tc2).xy;
	
	float2 perlin = (perlin_0 * g_PerlinGradient.x + perlin_1 * g_PerlinGradient.y + perlin_2 * g_PerlinGradient.z);


	// --------------- Water body color

	// Texcoord mash optimization: Texcoord of FFT wave is not required when blend_factor > 1
	float2 fft_tc = (blend_factor > 0) ? In.TexCoord : 0;

	float2 grad = g_texGradient.Sample(g_samplerGradient, fft_tc).xy;
	grad = lerp(perlin, grad, blend_factor);

	// Calculate normal here.
	float3 normal = normalize(float3(grad, g_TexelLength_x2));
	// Reflected ray
	float3 reflect_vec = reflect(-eye_dir, normal);
	// dot(N, V)
	float cos_angle = dot(normal, eye_dir);

	// A coarse way to handle transmitted light
	float3 body_color = g_WaterbodyColor;


	// --------------- Reflected color

	// ramp.x for fresnel term. ramp.y for sky blending
	float4 ramp = g_texFresnel.Sample(g_samplerFresnel, cos_angle).xyzw;
	// A workaround to deal with "indirect reflection vectors" (which are rays requiring multiple
	// reflections to reach the sky).
	if (reflect_vec.z < g_BendParam.x)
		ramp = lerp(ramp, g_BendParam.z, (g_BendParam.x - reflect_vec.z)/(g_BendParam.x - g_BendParam.y));
	reflect_vec.z = max(0, reflect_vec.z);

	float3 reflection = g_texReflectCube.Sample(g_samplerCube, reflect_vec).xyz;
	// Hack bit: making higher contrast
	reflection = reflection * reflection * 2.5f;

	// Blend with predefined sky color
	float3 reflected_color = lerp(g_SkyColor, reflection, ramp.y);

	// Combine waterbody color and reflected color
	float3 water_color = lerp(body_color, reflected_color, ramp.x);

	// ----------------- Actual Scene Reflection and Refraction colors

	float4 bumpColor1 = g_NormalMap.Sample( RRSampler, In.TexCoord );
	float4 bumpColor2 = g_NormalMap.Sample( RRSampler, In.TexCoord );
	float3 normal1 = (bumpColor1.rgb * 2.0f) - 1.0f;
    float3 normal2 = (bumpColor2.rgb * 2.0f) - 1.0f;
	float3 bumpnormal = normalize(normal1);

	float2 reflectionCoords;
	float2 refractionCoords;
	
	// Set the reflection color
	reflectionCoords.x = In.reflectionMapSamplingPos.x / In.reflectionMapSamplingPos.w / 2.0f + 0.5f;
	reflectionCoords.y = -In.reflectionMapSamplingPos.y / In.reflectionMapSamplingPos.w / 2.0f + 0.5f;

	// Re-position the texture coordinate sampling position by the scaled normal map value to simulate the rippling wave effect.
	float2 reflectTexCoord = reflectionCoords + (bumpnormal.xy * 0.03f);
	float4 reflectiveColor = g_ReflectionMap.Sample( RRSampler, reflectTexCoord );
	
	// Set the refraction color
	refractionCoords.x = In.refractionMapSamplingPos.x / In.refractionMapSamplingPos.w / 2.0f + 0.5f;
	refractionCoords.y = -In.refractionMapSamplingPos.y / In.refractionMapSamplingPos.w / 2.0f + 0.5f;

	// Re-position the texture coordinate sampling position by the scaled normal map value to simulate the rippling wave effect.
	float2 refractTexCoord = refractionCoords + (bumpnormal.xy * 0.03f);
	float4 refractiveColor = g_RefractionMap.Sample( RRSampler, refractTexCoord );

	//
	//Add a water color tint to the refraction.
	//

    // Combine the tint with the refraction color.
	//float4(0.0f, 0.8f, 1.0f, 1.0f) is refraction tint
    refractiveColor = saturate(refractiveColor * float4(0.0f, 0.8f, 1.0f, 1.0f));

	// --------------- Fresnel term

	/*float3 eyeVector = normalize(g_LocalEye - In.position3D);
    float3 normalVector = float3(0,1,0);
    float fresnelTerm = dot(eyeVector, normalVector);    
    float4 final_color = lerp(reflectiveColor, refractiveColor, fresnelTerm);*/

	float3 eyeVector = normalize( g_LocalEye - In.position3D );
	float3 normalVector = float3(0,0,1);
	
	float fangle = 1 + dot(eyeVector, normalVector);
	
	fangle = pow(fangle, 5);
	
	float fresnelTerm = 1 / fangle;
	
	fresnelTerm = fresnelTerm * 3;
	fresnelTerm = fresnelTerm < 0 ? 0 : fresnelTerm;
	fresnelTerm = fresnelTerm > 1 ? 1 : fresnelTerm;

	float4 final_color = refractiveColor*(1-fresnelTerm) + reflectiveColor * fresnelTerm * float4(water_color, 1);

	// --------------- Sun spots

	float cos_spec = clamp(dot(reflect_vec, g_SunDir), 0, 1);
	float sun_spot = pow(cos_spec, g_Shineness);
	final_color += float4(g_SunColor,1) * sun_spot;


	return final_color;//float4(water_color, 1);
}

//-----------------------------------------------------------------------------
// Name: WireframePS
// Type: Pixel shader                                      
// Desc:
//-----------------------------------------------------------------------------
float4 WireframePS() : SV_Target
{
	return float4(0.9f, 0.9f, 0.9f, 1);
}
