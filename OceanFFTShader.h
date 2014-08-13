//===============================================================================================================================
// OceanFFTShader
//
//===============================================================================================================================
// History
//
// -Created on 6/26/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANFFTSHADER_H
#define __OCEANFFTSHADER_H
//===============================================================================================================================
//===============================================================================================================================
//Includes
#include "D3D.h"
#include "Shader.h"
#include "Vertex.h"
#include "TextureManager.h"
#include "OceanParameters.h"
using namespace std;
//===============================================================================================================================
//===============================================================================================================================
class OceanFFTShader : public Shader
{
public:
	OceanFFTShader(D3D* d3d);
	OceanFFTShader(D3D* d3d, string filename);
	OceanFFTShader(const OceanFFTShader& other);
	~OceanFFTShader();

	bool Initialize(D3D* d3d);
	void Shutdown();
	
	void RenderDisplacement11(D3D* d3d, int vertexCount);
	void RenderGradient11(D3D* d3d, int vertexCount);

	/*void SetActualDim(UINT m)         { pActualDim->SetInt(m); }
	void SetInWidth(UINT m)           { pInWidth->SetInt(m); }
	void SetOutWidth(UINT m)          { pOutWidth->SetInt(m); }
	void SetOutHeight(UINT m)         { pOutHeight->SetInt(m); }
	void SetXAddressOffset(UINT m)    { pXAddressOffset->SetInt(m); }
	void SetYAddressOffset(UINT m)    { pYAddressOffset->SetInt(m); }
	void SetTime(float m)             { pTime->SetFloat(m); }
	void SetChoppyScale(float m)      { pChoppyScale->SetFloat(m); }
	void SetGridLen(float m)          { pGridLen->SetFloat(m); }

	void SetCSInputXYZ(ID3D11ShaderResourceView* m)              { pSRVVar->SetResource(m); }
	void SetDisplacementMap(ID3D11ShaderResourceView* m)         { pDisplacementMap->SetResource(m); }*/
	
private:
	bool InitializeShader11(D3D* d3d);
	
	void ShutdownShader();
	
	//ID3DX11EffectTechnique* OceanFFTDisplacementTech;
	//ID3DX11EffectTechnique* OceanFFTGradientTech;

	//ID3DX11EffectScalarVariable* pActualDim;
	//ID3DX11EffectScalarVariable* pInWidth;
	//ID3DX11EffectScalarVariable* pOutWidth;
	//ID3DX11EffectScalarVariable* pOutHeight;
	//ID3DX11EffectScalarVariable* pXAddressOffset;
	//ID3DX11EffectScalarVariable* pYAddressOffset;
	//ID3DX11EffectScalarVariable* pTime;
	//ID3DX11EffectScalarVariable* pChoppyScale;
	//ID3DX11EffectScalarVariable* pGridLen;

	//ID3DX11EffectShaderResourceVariable* pSRVVar;//Input from CS
	//ID3DX11EffectShaderResourceVariable* pDisplacementMap;
};
//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANFFTSHADER_H