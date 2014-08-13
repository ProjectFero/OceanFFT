//===============================================================================================================================
// OceanFFTCS
//
//===============================================================================================================================
// History
//
// -Created on 6/26/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANFFTCS_H
#define __OCEANFFTCS_H
//===============================================================================================================================
//===============================================================================================================================

//
//Includes
//

#include "D3D.h"
#include "Shader.h"
#include "Vertex.h"
#include "OceanParameters.h"
using namespace std;
//===============================================================================================================================
//===============================================================================================================================

//
// Globals
//

//Memory access coherency (in threads)
#define COHERENCY_GRANULARITY 128

//===============================================================================================================================
//===============================================================================================================================
class OceanFFTCS : public Shader
{
public:
	OceanFFTCS(D3D* d3d);
	OceanFFTCS(D3D* d3d, string filename);
	OceanFFTCS(const OceanFFTCS& other);
	~OceanFFTCS();

	ID3D11ShaderResourceView* GetGPUSRV() { return p_textureSRV; }

	byte* imageCopyFromGPU();

	bool Initialize();

	void Shutdown();
	
	void PerformFFT(ID3D11UnorderedAccessView* pUAV_Dst,
					 ID3D11ShaderResourceView* pSRV_Dst,
					 ID3D11ShaderResourceView* pSRV_Src);
	void PerformFFT(ID3D11UnorderedAccessView* pUAV_Dst,
			   ID3D11ShaderResourceView* pSRV_Src,
			   UINT thread_count,
			   UINT istride);

	ID3D11ShaderResourceView* GetXYZ() { ID3D11ShaderResourceView* srv; pSRVVar->GetResource(&srv); return srv; }

	void SetSlices(UINT slices) { this->slices = slices; }

	void SetThreadCount(UINT m) { pThreadCount->SetInt(m); }
	void SetOStride(UINT m) { pOStride->SetInt(m); }
	void SetIStride(UINT m) { pIStride->SetInt(m); }
	void SetPStride(UINT m) { pPStride->SetInt(m); }
	void SetPhaseBase(UINT m) { pPhaseBase->SetFloat(m); }

private:

	//This will go to the pixel shader. It is
	// the texture from the GPU.
	ID3D11ShaderResourceView* p_textureSRV;

	// More than one array can be transformed at same time
	UINT slices;

	// For 512x512 config, we need 6 constant buffers
	ID3D11Buffer* pRadix008A_CB[6];

	//Buffers that come from the ocean
	ID3D11UnorderedAccessView* m_pUAV_Dst;
	ID3D11ShaderResourceView* m_pSRV_Dst;
	ID3D11ShaderResourceView* m_pSRV_Src;

	// Temporary buffers
	ID3D11Buffer* pBuffer_Tmp;
	ID3D11UnorderedAccessView* pUAV_Tmp;
	ID3D11ShaderResourceView* pSRV_Tmp;

	//Constant buffer FX
	ID3DX11EffectScalarVariable* pThreadCount;
	ID3DX11EffectScalarVariable* pOStride;
	ID3DX11EffectScalarVariable* pIStride;
	ID3DX11EffectScalarVariable* pPStride;
	ID3DX11EffectScalarVariable* pPhaseBase;

	ID3DX11EffectTechnique* OceanFFTCSTech;
	ID3DX11EffectTechnique* OceanFFTCS2Tech;
	ID3DX11EffectShaderResourceVariable* pSRVVar;
	ID3DX11EffectUnorderedAccessViewVariable* pUAVVar;
};
//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANFFTCS_H