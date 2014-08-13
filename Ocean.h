//===============================================================================================================================
// Ocean
//
// FFT Ocean with 2 Compute Shaders
// Based on the Direct Compute Ocean from NVIDIA
//===============================================================================================================================
// History
//
// -Created on 6/26/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEAN_H
#define __OCEAN_H
//===============================================================================================================================
//===============================================================================================================================

//
//Includes
//

#include "D3D.h"
#include "Vertex.h"
#include "Camera.h"
#include "SimpleTerrain.h"
#include "TextureShader.h"
#include "OceanHeightfieldCS.h"
#include "DCRenderTexture.h"
#include "OceanFFTCS.h"
#include "OceanFFTShader.h"
#include "OceanParameters.h"
#include "OceanMesh.h"
#include "OceanHeightfield.h"
#include "RenderTarget2D.h"
#include "DepthStencilBuffer.h"
using namespace std;

//===============================================================================================================================
//===============================================================================================================================
class Ocean
{
	D3D* m_pD3DSystem;
	OceanParameters ocean_params;
	ocean_mesh_properties ocean_mesh_prop;
	OceanHeightfieldCS* m_pHeightfieldCS;
	OceanFFTCS* m_pOceanFFTCS;
	OceanFFTShader* m_pOceanFFTShader;
	OceanMesh* m_pMesh;
public:
	Ocean(D3D* d3d, ocean_mesh_properties omp, OceanParameters params);
	~Ocean();

	void Initialize();

	void Update(float dt);
	void Render(float dt, Camera* camera);

	void SetWireframe( bool wireframe ) { m_wireframe = wireframe; }

	void SetSeaLevel(float level) { m_pMesh->SetSeaLevel(level); }

	void SetReflectionMap(ID3D11ShaderResourceView* srv) { m_pMesh->SetReflectionMap(srv); }
	void SetRefractionMap(ID3D11ShaderResourceView* srv) { m_pMesh->SetRefractionMap(srv); }

	void SetEnablePostProcessing(bool set) { m_pEnablePostProcessing = set; }

	void SetColorTarget(RenderTarget2D rt) { colorTarget = rt; }
	void SetDepthBuffer(DepthStencilBuffer dsb) { depthBuff = dsb; }

private:
	bool m_wireframe;

	bool m_pEnablePostProcessing;
	RenderTarget2D colorTarget;
	DepthStencilBuffer depthBuff;

	OceanHeightfield* m_pOceanHeightfield;

	ID3D11SamplerState* m_pPointSamplerState;

	ID3D11Buffer* m_pImmutableCB;
	ID3D11Buffer* m_pPerFrameCB;

	DCRenderTexture* m_pDisplacementRT;
	DCRenderTexture* m_pGradientRT;

	ID3D11Buffer* m_pBuffer_Float2_H0;
	ID3D11ShaderResourceView* m_pSRV_H0;
	ID3D11UnorderedAccessView* m_pUAV_H0;
	
	ID3D11Buffer* m_pBuffer_Float_Omega;
	ID3D11ShaderResourceView* m_pSRV_Omega;
	ID3D11UnorderedAccessView* m_pUAV_Omega;

	ID3D11Buffer* m_pBuffer_Float_Dxyz;
	ID3D11ShaderResourceView* m_pSRV_Dxyz;
	ID3D11UnorderedAccessView* m_pUAV_Dxyz;

	ID3D11Buffer* m_pBuffer_Float2_Ht;
	ID3D11ShaderResourceView* m_pSRV_Ht;
	ID3D11UnorderedAccessView* m_pUAV_Ht;

	ID3D11Buffer* m_pQuadVB;
};
//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEAN_H