//================================================================================================================================================================
// FFT Real Time Ocean
// OceanSurfShader
//
// Based on the NVIDIA sample for OceanCS
//================================================================================================================================================================
// History
//
// -Created on 6/21/2014 by Dustin Watson
//================================================================================================================================================================
#ifndef __OCEANSURFSHADER_H
#define __OCEANSURFSHADER_H
//================================================================================================================================================================
//================================================================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Camera.h"
#include "Vertex.h"
#include "Shader.h"

//================================================================================================================================================================
//================================================================================================================================================================
class OceanSurfShader : public Shader
{
public:
	OceanSurfShader(D3D* d3d);
	OceanSurfShader(D3D* d3d, string filename);
	OceanSurfShader(const OceanSurfShader& other);
	~OceanSurfShader();

	bool Initialize(D3D* d3d);
	void Shutdown();
	
	bool Render11(D3D* d3d, int indexCount, int startIndex);

	void SetEnableOceanSurfShading(bool enable)     { m_EnableOceanSurfShading = enable; }
	void SetEnableOceanSurfWireframe(bool enable)   { m_EnableOceanSurfWireframe = enable; }

	/*void SetSkyColor(XMFLOAT3 m)                  { pSkyColor->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetWaterbodyColor(XMFLOAT3 m)            { pWaterbodyColor->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetShineness(float m)                    { pShineness->SetFloat(m); }
	void SetSunDir(XMFLOAT3 m)                    { pSunDir->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetSunColor(XMFLOAT3 m)                  { pSunColor->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetBendParam(XMFLOAT3 m)                 { pBendParam->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetPerlinSize(float m)                   { pPerlinSize->SetFloat(m); }
	void SetPerlinAmplitude(XMFLOAT3 m)           { pPerlinAmplitude->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetPerlinOctave(XMFLOAT3 m)              { pPerlinOctave->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetPerlinGradient(XMFLOAT3 m)            { pPerlinGradient->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }
	void SetTexelLengthx2(float m)                { pTexelLengthx2->SetFloat(m); }
	void SetUVScale(float m)                      { pUVScale->SetFloat(m); }
	void SetUVOffset(float m)                     { pUVOffset->SetFloat(m); }

	void SetLocalMatrix(CXMMATRIX m)              { pMatLocal->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWVPMatrix(CXMMATRIX m)                { pMatWVP->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetUVBase(XMFLOAT2 m)                    { pUVBase->SetRawValue(&m, 0, sizeof(XMFLOAT2)); }
	void SetPerlinMovement(XMFLOAT2 m)            { pPerlinMovement->SetRawValue(&m, 0, sizeof(XMFLOAT2)); }
	void SetLocalEye(XMFLOAT3 m)                  { pLocalEye->SetRawValue(&m, 0, sizeof(XMFLOAT3)); }

	void SetTexDisplacement(ID3D11ShaderResourceView* m)    { pTexDisplacement->SetRawValue(m, 0, sizeof(XMFLOAT3)); }
	void SetTexPerlin(ID3D11ShaderResourceView* m)          { pTexPerlin->SetResource(m); }
	void SetTexGradient(ID3D11ShaderResourceView* m)        { pTexGradient->SetResource(m); }
	void SetTexFresnel(ID3D11ShaderResourceView* m)         { pTexFresnel->SetResource(m); }
	void SetTexReflectCube(ID3D11ShaderResourceView* m)     { pTexReflectCube->SetResource(m); }

	void SetSamplerDisplacement(ID3D11SamplerState* samp)   { pSamplerDisplacement->SetSampler(0, samp); }
	void SetSamplerPerlin(ID3D11SamplerState* samp)         { pSamplerPerlin->SetSampler(1, samp); }
	void SetSamplerGradient(ID3D11SamplerState* samp)       { pSamplerGradient->SetSampler(2, samp); }
	void SetSamplerFresnel(ID3D11SamplerState* samp)        { pSamplerFresnel->SetSampler(3, samp); }
	void SetSamplerCube(ID3D11SamplerState* samp)           { pSamplerCube->SetSampler(4, samp); }*/

private:
	bool InitializeShader11(D3D* d3d);
		
	void ShutdownShader();
	
	//
	// Variables
	//

	bool m_EnableOceanSurfShading;
	bool m_EnableOceanSurfWireframe;

	////Shading Params
	//ID3DX11EffectVectorVariable* pSkyColor;
	//ID3DX11EffectVectorVariable* pWaterbodyColor;
	//ID3DX11EffectScalarVariable* pShineness;
	//ID3DX11EffectVectorVariable* pSunDir;
	//ID3DX11EffectVectorVariable* pSunColor;
	//ID3DX11EffectVectorVariable* pBendParam;
	//ID3DX11EffectScalarVariable* pPerlinSize;
	//ID3DX11EffectVectorVariable* pPerlinAmplitude;
	//ID3DX11EffectVectorVariable* pPerlinOctave;
	//ID3DX11EffectVectorVariable* pPerlinGradient;
	//ID3DX11EffectScalarVariable* pTexelLengthx2;
	//ID3DX11EffectScalarVariable* pUVScale;
	//ID3DX11EffectScalarVariable* pUVOffset;

	////Draw Call Params
	//ID3DX11EffectMatrixVariable* pMatLocal;
	//ID3DX11EffectMatrixVariable* pMatWVP;
	//ID3DX11EffectVectorVariable* pUVBase;
	//ID3DX11EffectVectorVariable* pPerlinMovement;
	//ID3DX11EffectVectorVariable* pLocalEye;

	//ID3DX11EffectShaderResourceVariable* pTexDisplacement;
	//ID3DX11EffectShaderResourceVariable* pTexPerlin;
	//ID3DX11EffectShaderResourceVariable* pTexGradient;
	//ID3DX11EffectShaderResourceVariable* pTexFresnel;
	//ID3DX11EffectShaderResourceVariable* pTexReflectCube;

	//ID3DX11EffectSamplerVariable* pSamplerDisplacement;
	//ID3DX11EffectSamplerVariable* pSamplerPerlin;
	//ID3DX11EffectSamplerVariable* pSamplerGradient;
	//ID3DX11EffectSamplerVariable* pSamplerFresnel;
	//ID3DX11EffectSamplerVariable* pSamplerCube;

	//ID3D11InputLayout* m_LayoutShading11;
	//ID3D11InputLayout* m_LayoutWireframe11;

	//ID3DX11EffectTechnique* pOceanSurfShadingTech11;
	//ID3DX11EffectTechnique* pOceanSurfWireframeTech11;
};
//================================================================================================================================================================
//================================================================================================================================================================
#endif//__OCEANSURFSHADER_H