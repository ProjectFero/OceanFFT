//===============================================================================================================================
// OceanParameters
//
//===============================================================================================================================
// History
//
// -Created on 6/25/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANPARAMETERS_H
#define __OCEANPARAMETERS_H
//===============================================================================================================================
//===============================================================================================================================
//Includes
#include "D3D.h"
using namespace std;

#define PAD16(n) (((n)+15)/16*16)
//===============================================================================================================================
//===============================================================================================================================
struct OceanParameters
{
	// Must be power of 2.
	int displacement_map_dim;//512
	// Typical value is 1000 ~ 2000
	float patch_length;//2000.0f
	// Adjust the time interval for simulation.
	float time_scale;//0.8f
	// Amplitude for transverse wave. Around 1.0
	float wave_amplitude;//0.35f
	// Wind direction. Normalization not required.
	D3DXVECTOR2 wind_dir;//D3DXVECTOR2(0.8f, 0.6f)
	// Around 100 ~ 1000
	float wind_speed;//600
	// This value damps out the waves against the wind direction.
	// Smaller value means higher wind dependency.
	float wind_dependency;//0.07f
	// The amplitude for longitudinal wave. Must be positive.
	float choppy_scale;//1.3f
};

struct ocean_mesh_properties
{
	// Mesh grid dimension, must be 2^n. 4x4 ~ 256x256
	int g_MeshDim;//128;
	// Side length of square shaped mesh patch
	float g_PatchLength;
	// Dimension of displacement map
	int g_DisplaceMapDim;
	// Subdivision thredshold. Any quad covers more pixels than this value needs to be subdivided.
	float g_UpperGridCoverage;//64.0f;
	// Draw distance = g_PatchLength * 2^g_FurthestCover
	int g_FurthestCover;//8;

	D3DXVECTOR3 g_SkyColor;//D3DXVECTOR3(0.38f, 0.45f, 0.56f);
	D3DXVECTOR3 g_WaterbodyColor;//D3DXVECTOR3(0.07f, 0.15f, 0.2f);
	float g_Skyblending;//16.0f;

	//Perlin wave parameters
	float g_PerlinSize;//1.0f;
	float g_PerlinSpeed;//0.06f;
	D3DXVECTOR3 g_PerlinAmplitude;//D3DXVECTOR3(35, 42, 57);
	D3DXVECTOR3 g_PerlinGradient;//D3DXVECTOR3(1.4f, 1.6f, 2.2f);
	D3DXVECTOR3 g_PerlinOctave;//D3DXVECTOR3(1.12f, 0.59f, 0.23f);
	D3DXVECTOR2 g_WindDir;

	D3DXVECTOR3 g_BendParam;//D3DXVECTOR3(0.1f, -0.4f, 0.2f);

	//Sunspot params
	D3DXVECTOR3 g_SunDir;//D3DXVECTOR3(0.936016f, -0.343206f, 0.0780013f);
	D3DXVECTOR3 g_SunColor;//D3DXVECTOR3(1.0f, 1.0f, 0.6f);
	float g_Shineness;//400.0f
};
//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANPARAMETERS_H