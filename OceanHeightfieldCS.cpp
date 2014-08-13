#include "OceanHeightfieldCS.h"
#include "TextureManager.h"
#include "MathHelper.h"
//==============================================================================================================================
//==============================================================================================================================
OceanHeightfieldCS::OceanHeightfieldCS(D3D* d3d)
:	Shader(d3d)
{
	Initialize();
}
//==============================================================================================================================
OceanHeightfieldCS::OceanHeightfieldCS(D3D* d3d, string filename)
:	Shader(d3d, filename)
{
}
//==============================================================================================================================
OceanHeightfieldCS::OceanHeightfieldCS(const OceanHeightfieldCS& other)
:	Shader(other)
{
}
//==============================================================================================================================
OceanHeightfieldCS::~OceanHeightfieldCS()
{
}
//==============================================================================================================================
bool OceanHeightfieldCS::Initialize()
{
	//
	// Create the Compute Shader
	//

	Compile( "Ocean\\OceanHeightfieldCS.hlsl", "UpdateSpectrumCS", ST_COMPUTE );

	/*OceanHeightfieldCSTech = mFX->GetTechniqueByName("OceanHeightfieldCS");

	pSRVVar_h0 = mFX->GetVariableByName("g_InputH0")->AsShaderResource();
	pSRVVar_omega = mFX->GetVariableByName("g_InputOmega")->AsShaderResource();
	pUAVVar_ht = mFX->GetVariableByName("g_OutputHt")->AsUnorderedAccessView();

	pActualDim       = mFX->GetVariableByName("g_ActualDim")->AsScalar();
	pInWidth         = mFX->GetVariableByName("g_InWidth")->AsScalar();
	pOutWidth        = mFX->GetVariableByName("g_OutWidth")->AsScalar();
	pOutHeight       = mFX->GetVariableByName("g_OutHeight")->AsScalar();
	pXAddressOffset  = mFX->GetVariableByName("g_DtxAddressOffset")->AsScalar();
	pYAddressOffset  = mFX->GetVariableByName("g_DtyAddressOffset")->AsScalar();
	pTime            = mFX->GetVariableByName("g_Time")->AsScalar();
	pChoppyScale     = mFX->GetVariableByName("g_ChoppyScale")->AsScalar();

	pSRVVar_h0->SetResource( m_pSRV_H0 );
	pSRVVar_omega->SetResource( m_pSRV_Omega );
	pUAVVar_ht->SetUnorderedAccessView( m_pUAV_Ht );*/

	return true;
}
//==============================================================================================================================
void OceanHeightfieldCS::Shutdown()
{
}
//==============================================================================================================================