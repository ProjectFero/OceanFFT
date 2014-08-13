#include "OceanSurfShader.h"
#include "CUtility.h"
//================================================================================================================================================================
OceanSurfShader::OceanSurfShader(D3D* d3d)
:	Shader(d3d), m_EnableOceanSurfShading(true), m_EnableOceanSurfWireframe(false)
{
	Initialize(d3d);
}
//================================================================================================================================================================
OceanSurfShader::OceanSurfShader(D3D* d3d, string filename)
:	Shader(d3d, filename), m_EnableOceanSurfShading(true), m_EnableOceanSurfWireframe(false)
{
	Initialize(d3d);
}
//================================================================================================================================================================
OceanSurfShader::OceanSurfShader(const OceanSurfShader& other)
:	Shader(other)
{

}
//================================================================================================================================================================
OceanSurfShader::~OceanSurfShader()
{

}
//================================================================================================================================================================
bool OceanSurfShader::Initialize(D3D* d3d)
{
	bool result;

	result = InitializeShader11(d3d);
	
	if(!result)
	{
		return false;
	}

	return true;
}
//================================================================================================================================================================
void OceanSurfShader::Shutdown()
{
	ShutdownShader();
}
//================================================================================================================================================================
bool OceanSurfShader::Render11(D3D* d3d, int indexCount, int startIndex)
{
	//if (m_EnableOceanSurfShading)
	//{
	//	d3d->GetDeviceContext()->IASetInputLayout(m_LayoutShading11);

	//	D3DX11_TECHNIQUE_DESC techDesc;
	//	pOceanSurfShadingTech11->GetDesc(&techDesc);

	//	ID3DX11EffectPass* pass = 0;
	//	for (UINT i = 0; i < techDesc.Passes; ++i)
	//	{
	//		pass = pOceanSurfShadingTech11->GetPassByIndex(i);
	//		pass->Apply(0, d3d->GetDeviceContext());
	//		//d3d->GetDeviceContext()->Draw(indexCount, 0);
	//		d3d->GetDeviceContext()->DrawIndexed(indexCount, startIndex, 0);
	//	}

	//	return true;
	//}
	//
	//if (m_EnableOceanSurfWireframe)
	//{
	//	d3d->GetDeviceContext()->IASetInputLayout(m_LayoutWireframe11);

	//	D3DX11_TECHNIQUE_DESC techDesc;
	//	pOceanSurfWireframeTech11->GetDesc(&techDesc);

	//	ID3DX11EffectPass* pass = 0;
	//	for (UINT i = 0; i < techDesc.Passes; ++i)
	//	{
	//		pass = pOceanSurfWireframeTech11->GetPassByIndex(i);
	//		pass->Apply(0, d3d->GetDeviceContext());
	//		//d3d->GetDeviceContext()->Draw(indexCount, 0);
	//		d3d->GetDeviceContext()->DrawIndexed(indexCount, startIndex, 0);
	//	}

	//	return true;
	//}

	d3d->GetDeviceContext()->DrawIndexed(indexCount, startIndex, 0);

	return false;
}
//================================================================================================================================================================
bool OceanSurfShader::InitializeShader11(D3D* d3d)
{
	D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	SetInputLayoutDesc(mesh_layout_desc, 1);
	Compile( "Ocean\\OceanShading.hlsl", "OceanSurfVS", ST_VERTEX );
	Compile( "Ocean\\OceanShading.hlsl", "OceanSurfPS", ST_PIXEL );
	Compile( "Ocean\\OceanShading.hlsl", "WireframePS", ST_PIXEL );



	/*pOceanSurfShadingTech11     = mFX->GetTechniqueByName("OceanSurfTech");
	pOceanSurfWireframeTech11   = mFX->GetTechniqueByName("OceanWireTech");

	pSkyColor                   = mFX->GetVariableByName("g_SkyColor")->AsVector();
	pWaterbodyColor             = mFX->GetVariableByName("g_WaterbodyColor")->AsVector();
	pShineness                  = mFX->GetVariableByName("g_Shineness")->AsScalar();
	pSunDir                     = mFX->GetVariableByName("g_SunDir")->AsVector();
	pSunColor                   = mFX->GetVariableByName("g_SunColor")->AsVector();
	pBendParam                  = mFX->GetVariableByName("g_BendParam")->AsVector();
	pPerlinSize                 = mFX->GetVariableByName("g_PerlinSize")->AsScalar();
	pPerlinAmplitude            = mFX->GetVariableByName("g_PerlinAmplitude")->AsVector();
	pPerlinOctave               = mFX->GetVariableByName("g_PerlinOctave")->AsVector();
	pPerlinGradient             = mFX->GetVariableByName("g_PerlinGradient")->AsVector();
	pTexelLengthx2              = mFX->GetVariableByName("g_TexelLength_x2")->AsScalar();
	pUVScale                    = mFX->GetVariableByName("g_UVScale")->AsScalar();
	pUVOffset                   = mFX->GetVariableByName("g_UVOffset")->AsScalar();

	pMatLocal                   = mFX->GetVariableByName("g_matLocal")->AsMatrix();
	pMatWVP                     = mFX->GetVariableByName("g_matWorldViewProj")->AsMatrix();
	pUVBase                     = mFX->GetVariableByName("g_UVBase")->AsVector();
	pPerlinMovement             = mFX->GetVariableByName("g_PerlinMovement")->AsVector();
	pLocalEye                   = mFX->GetVariableByName("g_LocalEye")->AsVector();

	pTexDisplacement            = mFX->GetVariableByName("g_texDisplacement")->AsShaderResource();
	pTexPerlin                  = mFX->GetVariableByName("g_texPerlin")->AsShaderResource();
	pTexGradient                = mFX->GetVariableByName("g_texGradient")->AsShaderResource();
	pTexFresnel                 = mFX->GetVariableByName("g_texFresnel")->AsShaderResource();
	pTexReflectCube             = mFX->GetVariableByName("g_texReflectCube")->AsShaderResource();

	pSamplerDisplacement        = mFX->GetVariableByName("g_samplerDisplacement")->AsSampler();
	pSamplerPerlin              = mFX->GetVariableByName("g_samplerPerlin")->AsSampler();
	pSamplerGradient            = mFX->GetVariableByName("g_samplerGradient")->AsSampler();
	pSamplerFresnel             = mFX->GetVariableByName("g_samplerFresnel")->AsSampler();
	pSamplerCube                = mFX->GetVariableByName("g_samplerCube")->AsSampler();
	
	D3DX11_PASS_DESC passDesc;
	pOceanSurfShadingTech11->GetPassByIndex(0)->GetDesc(&passDesc);
	d3d->GetDevice11()->CreateInputLayout(mesh_layout_desc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_LayoutShading11);

	pOceanSurfWireframeTech11->GetPassByIndex(0)->GetDesc(&passDesc);
	d3d->GetDevice11()->CreateInputLayout(mesh_layout_desc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_LayoutWireframe11);*/

	return true;
}
//================================================================================================================================================================		
void OceanSurfShader::ShutdownShader()
{
	//SAFE_RELEASE(m_LayoutShading11);
	//SAFE_RELEASE(m_LayoutWireframe11);
}
//================================================================================================================================================================