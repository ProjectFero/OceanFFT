#include "OceanFFTShader.h"
//==============================================================================================================================
OceanFFTShader::OceanFFTShader(D3D* d3d)
:	Shader(d3d)
{
	Initialize(d3d);
}
//==============================================================================================================================
OceanFFTShader::OceanFFTShader(D3D* d3d, string filename)
:	Shader(d3d, filename)
{
	Initialize(d3d);
}
//==============================================================================================================================
OceanFFTShader::OceanFFTShader(const OceanFFTShader& other)
:	Shader(other)
{

}
//==============================================================================================================================
OceanFFTShader::~OceanFFTShader()
{

}
//==============================================================================================================================
bool OceanFFTShader::Initialize(D3D* d3d)
{
	bool result;

	if (d3d->GetEngineOptions()->m_d3dVersion == DIRECTX10)
		;//result = InitializeShader10(d3d, "FX\\terrain2.fx");
	else if (d3d->GetEngineOptions()->m_d3dVersion == DIRECTX11)
		result = InitializeShader11(d3d);
	
	if(!result)
	{
		return false;
	}

	return true;
}
//==============================================================================================================================
void OceanFFTShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();
}
//==============================================================================================================================
bool OceanFFTShader::InitializeShader11(D3D* d3d)
{
	/*OceanFFTDisplacementTech  = mFX->GetTechniqueByName("OceanFFTDisplacementShader");
	OceanFFTGradientTech      = mFX->GetTechniqueByName("OceanFFTGradientShader");
	
	pActualDim       = mFX->GetVariableByName("g_ActualDim")->AsScalar();
	pInWidth         = mFX->GetVariableByName("g_InWidth")->AsScalar();
	pOutWidth        = mFX->GetVariableByName("g_OutWidth")->AsScalar();
	pOutHeight       = mFX->GetVariableByName("g_OutHeight")->AsScalar();
	pXAddressOffset  = mFX->GetVariableByName("g_DxAddressOffset")->AsScalar();
	pYAddressOffset  = mFX->GetVariableByName("g_DyAddressOffset")->AsScalar();
	pTime            = mFX->GetVariableByName("g_Time")->AsScalar();
	pChoppyScale     = mFX->GetVariableByName("g_ChoppyScale")->AsScalar();
	pGridLen         = mFX->GetVariableByName("g_GridLen")->AsScalar();

	pSRVVar          = mFX->GetVariableByName("g_InputDxyz")->AsShaderResource();
	pDisplacementMap = mFX->GetVariableByName("g_samplerDisplacementMap")->AsShaderResource();*/
	
	//
	// Create the vertex and pixel shaders
	//
	D3D11_INPUT_ELEMENT_DESC quad_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	SetInputLayoutDesc(quad_layout_desc, 1);
	Compile( "Ocean\\OceanFFTShader.hlsl", "QuadVS", ST_VERTEX );
	Compile( "Ocean\\OceanFFTShader.hlsl", "UpdateDisplacementPS", ST_PIXEL );
	Compile( "Ocean\\OceanFFTShader.hlsl", "GenGradientFoldingPS", ST_PIXEL );

	return true;
}
//==============================================================================================================================
void OceanFFTShader::ShutdownShader()
{
	SAFE_RELEASE(m_layout11);
}
//==============================================================================================================================