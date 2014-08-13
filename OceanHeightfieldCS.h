//===============================================================================================================================
// OceanHeightfieldCS
//
//===============================================================================================================================
// History
//
// -Created on 6/25/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANHEIGHTFIELDCS_H
#define __OCEANHEIGHTFIELDCS_H
//===============================================================================================================================
//===============================================================================================================================
//Includes
#include "D3D.h"
#include "Shader.h"
#include "Vertex.h"
#include "OceanParameters.h"
#include "DCRenderTexture.h"
using namespace std;
//===============================================================================================================================
//===============================================================================================================================
class OceanHeightfieldCS : public Shader
{
public:
	OceanHeightfieldCS(D3D* d3d);
	OceanHeightfieldCS(D3D* d3d, string filename);
	OceanHeightfieldCS(const OceanHeightfieldCS& other);
	~OceanHeightfieldCS();

	//ID3D11RenderTargetView* GetDisplacementRTV() { return m_pDisplacementRTV; }
	//DCRenderTexture* GetDisplacementRT() { return m_pDisplacementRT; }
	ID3D11ShaderResourceView* GetGPUSRV() { return p_textureSRV; }
	//ID3D11ShaderResourceView* GetSRVHT() { return m_pSRV_Ht; }

	bool Initialize();

	void Shutdown();
	
	void SetActualDim(UINT m)         { pActualDim->SetInt(m); }
	void SetInWidth(UINT m)           { pInWidth->SetInt(m); }
	void SetOutWidth(UINT m)          { pOutWidth->SetInt(m); }
	void SetOutHeight(UINT m)         { pOutHeight->SetInt(m); }
	void SetXAddressOffset(UINT m)    { pXAddressOffset->SetInt(m); }
	void SetYAddressOffset(UINT m)    { pYAddressOffset->SetInt(m); }
	void SetTime(float m)             { pTime->SetFloat(m); }
	void SetChoppyScale(float m)      { pChoppyScale->SetFloat(m); }

private:

	//This will go to the pixel shader. It is
	// the texture from the GPU.
	ID3D11ShaderResourceView* p_textureSRV;

	string m_ImageName;
	int m_TextureDataSize;
	byte* p_TextureData;
	ID3D11Texture2D* p_srcTexture;
	ID3D11Texture2D* p_dstTexture;

	ID3D11Buffer* p_dstGPUBuffer;

	ID3DX11EffectScalarVariable* pActualDim;
	ID3DX11EffectScalarVariable* pInWidth;
	ID3DX11EffectScalarVariable* pOutWidth;
	ID3DX11EffectScalarVariable* pOutHeight;
	ID3DX11EffectScalarVariable* pXAddressOffset;
	ID3DX11EffectScalarVariable* pYAddressOffset;
	ID3DX11EffectScalarVariable* pTime;
	ID3DX11EffectScalarVariable* pChoppyScale;

	ID3DX11EffectTechnique* OceanHeightfieldCSTech;
	ID3DX11EffectShaderResourceVariable* pSRVVar_h0;
	ID3DX11EffectShaderResourceVariable* pSRVVar_omega;
	ID3DX11EffectUnorderedAccessViewVariable* pUAVVar_ht;
};
//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANHEIGHTFIELDCS_H