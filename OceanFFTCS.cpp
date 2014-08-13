#include "OceanFFTCS.h"
#include "TextureManager.h"
#include "MathHelper.h"
//==============================================================================================================================
//==============================================================================================================================
OceanFFTCS::OceanFFTCS(D3D* d3d)
:	Shader(d3d)
{
}
OceanFFTCS::OceanFFTCS(D3D* d3d, string filename)
:	Shader(d3d, filename)
{
}
//==============================================================================================================================
OceanFFTCS::OceanFFTCS(const OceanFFTCS& other)
:	Shader(other)
{
}
//==============================================================================================================================
OceanFFTCS::~OceanFFTCS()
{
}
//==============================================================================================================================
bool OceanFFTCS::Initialize()
{
	// Create 6 cbuffers for 512x512 transform.
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = 32;//sizeof(float) * 5;
	cb_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cb_data;
	cb_data.SysMemPitch = 0;
	cb_data.SysMemSlicePitch = 0;

	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;
	};

	// Buffer 0
	const UINT thread_count = slices * (512 * 512) / 8;
	UINT ostride = 512 * 512 / 8;
	UINT istride = ostride;
	double phase_base = -TWO_PI / (512.0 * 512.0);
	
	CB_Structure cb_data_buf0 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf0;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[0]);
	assert(pRadix008A_CB[0]);

	// Buffer 1
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf1 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf1;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[1]);
	assert(pRadix008A_CB[1]);

	// Buffer 2
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf2 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf2;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[2]);
	assert(pRadix008A_CB[2]);

	// Buffer 3
	istride /= 8;
	phase_base *= 8.0;
	ostride /= 512;
	
	CB_Structure cb_data_buf3 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf3;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[3]);
	assert(pRadix008A_CB[3]);

	// Buffer 4
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf4 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf4;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[4]);
	assert(pRadix008A_CB[4]);

	// Buffer 5
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf5 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf5;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[5]);
	assert(pRadix008A_CB[5]);
	

	// Temp buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = sizeof(float) * 2 * (512 * slices) * 512;
    buf_desc.Usage = D3D11_USAGE_DEFAULT;
    buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    buf_desc.CPUAccessFlags = 0;
    buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    buf_desc.StructureByteStride = sizeof(float) * 2;

	m_pD3DSystem->GetDevice11()->CreateBuffer(&buf_desc, NULL, &pBuffer_Tmp);
	assert(pBuffer_Tmp);

	//
	// Create the Input buffer
	//

	// Temp shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = (512 * slices) * 512;

	m_pD3DSystem->GetDevice11()->CreateShaderResourceView(pBuffer_Tmp, &srv_desc, &pSRV_Tmp);
	assert(pSRV_Tmp);

	//
	// Create the Output buffer
	//

	// Temp undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = (512 * slices) * 512;
	uav_desc.Buffer.Flags = 0;

	m_pD3DSystem->GetDevice11()->CreateUnorderedAccessView(pBuffer_Tmp, &uav_desc, &pUAV_Tmp);
	assert(pUAV_Tmp);

	//
	// Compile the compute shader
	//

	Compile( "Ocean\\OceanFFTCS.hlsl", "Radix008A_CS", ST_COMPUTE );
	Compile( "Ocean\\OceanFFTCS.hlsl", "Radix008A_CS2", ST_COMPUTE );

	/*OceanFFTCSTech  = mFX->GetTechniqueByName("OceanFFTCS");
	OceanFFTCS2Tech = mFX->GetTechniqueByName("OceanFFTCS2");

	pThreadCount = mFX->GetVariableByName("thread_count")->AsScalar();
	pOStride     = mFX->GetVariableByName("ostride")->AsScalar();
	pIStride     = mFX->GetVariableByName("istride")->AsScalar();
	pPStride     = mFX->GetVariableByName("pstride")->AsScalar();
	pPhaseBase   = mFX->GetVariableByName("phase_base")->AsScalar();

	pSRVVar = mFX->GetVariableByName("g_SrcData")->AsShaderResource();
	pUAVVar = mFX->GetVariableByName("g_DstData")->AsUnorderedAccessView();*/

	//This will be done in RenderCS11
	//pSRVVar->SetResource( m_pSRV_Src );
	//pUAVVar->SetUnorderedAccessView( m_pUAV_Dst );

	return true;
}
//==============================================================================================================================
void OceanFFTCS::Shutdown()
{
	SAFE_RELEASE(pSRV_Tmp);
	SAFE_RELEASE(pUAV_Tmp);
	SAFE_RELEASE(pBuffer_Tmp);
	for (int i = 0; i < 6; i++)
		SAFE_RELEASE(pRadix008A_CB[i]);
}
//==============================================================================================================================
void OceanFFTCS::PerformFFT(ID3D11UnorderedAccessView* pUAV_Dst,
					 ID3D11ShaderResourceView* pSRV_Dst,
					 ID3D11ShaderResourceView* pSRV_Src)
{
	const UINT thread_count = slices * (512 * 512) / 8;
	ID3D11UnorderedAccessView* p_UAV_Tmp = pUAV_Tmp;
	ID3D11ShaderResourceView* p_SRV_Tmp = pSRV_Tmp;
	ID3D11Buffer* cs_cbs[1];

	UINT istride = 512 * 512 / 8;
	cs_cbs[0] = pRadix008A_CB[0];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(p_UAV_Tmp, pSRV_Src, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = pRadix008A_CB[1];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(pUAV_Dst, p_SRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = pRadix008A_CB[2];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(p_UAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = pRadix008A_CB[3];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(pUAV_Dst, p_SRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = pRadix008A_CB[4];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(p_UAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = pRadix008A_CB[5];
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	PerformFFT(pUAV_Dst, p_SRV_Tmp, thread_count, istride);
}
//==============================================================================================================================
void OceanFFTCS::PerformFFT(ID3D11UnorderedAccessView* pUAV_Dst,
			   ID3D11ShaderResourceView* pSRV_Src,
			   UINT thread_count,
			   UINT istride)
{
	// Setup execution configuration
	UINT grid = thread_count / COHERENCY_GRANULARITY;

	ID3D11DeviceContext* pd3dImmediateContext = m_pD3DSystem->GetDeviceContext();

	// Buffers
	ID3D11ShaderResourceView* cs_srvs[1] = {pSRV_Src};
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	ID3D11UnorderedAccessView* cs_uavs[1] = {pUAV_Dst};
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));

	// Shader
	if (istride > 1)
	{
		SwitchTo( "Radix008A_CS", ST_COMPUTE );
		pd3dImmediateContext->CSSetShader(m_pComputeShader, NULL, 0);
	}
	else
	{
		SwitchTo( "Radix008A_CS2", ST_COMPUTE );
		pd3dImmediateContext->CSSetShader(m_pComputeShader, NULL, 0);
	}

	// Execute
	pd3dImmediateContext->Dispatch(grid, 1, 1);

	// Unbind resource
	cs_srvs[0] = NULL;
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	cs_uavs[0] = NULL;
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));

	/*pSRVVar->SetResource( pSRV_Src );
	pUAVVar->SetUnorderedAccessView( pUAV_Dst );

	if (istride > 1)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		OceanFFTCSTech->GetDesc(&techDesc);
		ID3DX11EffectPass* pass = 0;
		pass = OceanFFTCSTech->GetPassByIndex(0);
		pass->Apply(0, m_pD3DSystem->GetDeviceContext());
		m_pD3DSystem->GetDeviceContext()->Dispatch( grid, 1, 1 );
	}
	else
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		OceanFFTCS2Tech->GetDesc(&techDesc);
		ID3DX11EffectPass* pass = 0;
		pass = OceanFFTCS2Tech->GetPassByIndex(0);
		pass->Apply(0, m_pD3DSystem->GetDeviceContext());
		m_pD3DSystem->GetDeviceContext()->Dispatch( grid, 1, 1 );
	}*/
}
//==============================================================================================================================