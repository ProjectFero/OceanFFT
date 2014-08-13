#include "Ocean.h"
#include "TextureManager.h"
#include "MathHelper.h"
//===============================================================================================================================
//===============================================================================================================================
Ocean::Ocean(D3D* d3d, ocean_mesh_properties omp, OceanParameters params)
:	m_pD3DSystem(d3d), ocean_params(params), ocean_mesh_prop(omp)
{
	Initialize();
}
//===============================================================================================================================
Ocean::~Ocean()
{
	SAFE_RELEASE(m_pPointSamplerState);
	SAFE_RELEASE(m_pImmutableCB);
	SAFE_RELEASE(m_pPerFrameCB);
	SAFE_RELEASE(m_pBuffer_Float2_H0);
	SAFE_RELEASE(m_pSRV_H0);
	SAFE_RELEASE(m_pUAV_H0);
	SAFE_RELEASE(m_pBuffer_Float_Omega);
	SAFE_RELEASE(m_pSRV_Omega);
	SAFE_RELEASE(m_pUAV_Omega);
	SAFE_RELEASE(m_pBuffer_Float_Dxyz);
	SAFE_RELEASE(m_pSRV_Dxyz);
	SAFE_RELEASE(m_pUAV_Dxyz);
	SAFE_RELEASE(m_pBuffer_Float2_Ht);
	SAFE_RELEASE(m_pSRV_Ht);
	SAFE_RELEASE(m_pUAV_Ht);
	SAFE_RELEASE(m_pQuadVB);
}
//===============================================================================================================================
void Ocean::Initialize()
{
	//
	// Create the heightfield
	//

	m_pOceanHeightfield = new OceanHeightfield();
	m_pOceanHeightfield->ocean_params = ocean_params;
	m_pOceanHeightfield->Initialize();

	//
	//Load the Ocean FFT Shaders
	//

	m_pHeightfieldCS = new OceanHeightfieldCS(m_pD3DSystem);
	m_pOceanFFTCS = new OceanFFTCS(m_pD3DSystem);
	m_pOceanFFTShader = new OceanFFTShader(m_pD3DSystem);
	
	//
	// Create the Input and Output buffers
	//

	TextureManager* tm = TextureManager::GetInstance();
	
	tm->CreateStructuredBuffer( m_pD3DSystem->GetDevice11(), m_pOceanHeightfield->float2_stride, m_pOceanHeightfield->input_full_size * m_pOceanHeightfield->float2_stride, m_pOceanHeightfield->h0_data, &m_pBuffer_Float2_H0 );
	tm->CreateBufferSRV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float2_H0, &m_pSRV_H0 );
	tm->CreateBufferUAV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float2_H0, &m_pUAV_H0 );

	tm->CreateStructuredBuffer( m_pD3DSystem->GetDevice11(), sizeof(float), m_pOceanHeightfield->input_full_size * sizeof(float), m_pOceanHeightfield->omega_data, &m_pBuffer_Float_Omega );
	tm->CreateBufferSRV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float_Omega, &m_pSRV_Omega );
	tm->CreateBufferUAV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float_Omega, &m_pUAV_Omega );

	tm->CreateStructuredBuffer( m_pD3DSystem->GetDevice11(), m_pOceanHeightfield->float2_stride, 3 * m_pOceanHeightfield->input_half_size * m_pOceanHeightfield->float2_stride, nullptr, &m_pBuffer_Float2_Ht );
	tm->CreateBufferSRV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float2_Ht, &m_pSRV_Ht );
	tm->CreateBufferUAV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float2_Ht, &m_pUAV_Ht );
	
	tm->CreateStructuredBuffer( m_pD3DSystem->GetDevice11(), m_pOceanHeightfield->float2_stride, 3 * m_pOceanHeightfield->output_size * m_pOceanHeightfield->float2_stride, nullptr, &m_pBuffer_Float_Dxyz );
	tm->CreateBufferSRV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float_Dxyz, &m_pSRV_Dxyz );
	tm->CreateBufferUAV( m_pD3DSystem->GetDevice11(), m_pBuffer_Float_Dxyz, &m_pUAV_Dxyz );

	SAFE_DELETE_ARRAY(m_pOceanHeightfield->h0_data);
	SAFE_DELETE_ARRAY(m_pOceanHeightfield->omega_data);

	//
	// Create the Samplers
	//

	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.MipLODBias = 0; 
	sam_desc.MaxAnisotropy = 1; 
	sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
	sam_desc.BorderColor[0] = 1.0f;
	sam_desc.BorderColor[1] = 1.0f;
	sam_desc.BorderColor[2] = 1.0f;
	sam_desc.BorderColor[3] = 1.0f;
	sam_desc.MinLOD = -FLT_MAX;
	sam_desc.MaxLOD = FLT_MAX;
	m_pD3DSystem->GetDevice11()->CreateSamplerState(&sam_desc, &m_pPointSamplerState);
	assert(m_pPointSamplerState);

	//
	// Create the Constant buffers
	//

	UINT actual_dim = ocean_params.displacement_map_dim;
	UINT input_width = actual_dim + 4;
	// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
	UINT output_width = actual_dim;
	UINT output_height = actual_dim;
	UINT dtx_offset = actual_dim * actual_dim;
	UINT dty_offset = actual_dim * actual_dim * 2;
	UINT immutable_consts[] = {actual_dim, input_width, output_width, output_height, dtx_offset, dty_offset};

	tm->CreateConstantBuffer( m_pD3DSystem->GetDevice11(), &m_pImmutableCB, PAD16(sizeof(immutable_consts)), &immutable_consts[0] );
	tm->CreateConstantBuffer( m_pD3DSystem->GetDevice11(), &m_pPerFrameCB, PAD16(sizeof(float) * 3), nullptr, true );

	//
	//Create the Displacement and Gradient RT
	//

	m_pDisplacementRT = new DCRenderTexture( m_pD3DSystem, ocean_params.displacement_map_dim, ocean_params.displacement_map_dim, DXGI_FORMAT_R32G32B32A32_FLOAT );
	m_pGradientRT = new DCRenderTexture( m_pD3DSystem, ocean_params.displacement_map_dim, ocean_params.displacement_map_dim, DXGI_FORMAT_R16G16B16A16_FLOAT );

	//
	// Quad vertex buffer
	//

	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = 4 * sizeof(D3DXVECTOR4);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;

	float quad_verts[] =
	{
		-1, -1, 0, 1,
		-1,  1, 0, 1,
		 1, -1, 0, 1,
		 1,  1, 0, 1,
	};
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = &quad_verts[0];
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;
	
	m_pD3DSystem->GetDevice11()->CreateBuffer(&vb_desc, &init_data, &m_pQuadVB);
	assert(m_pQuadVB);

	//
	// Set FFT Shader data
	//

	m_pOceanFFTCS->SetSlices( 3 );
	m_pOceanFFTCS->Initialize();

	//
	// Create the Mesh for the ocean
	//

	m_pMesh = new OceanMesh(m_pD3DSystem, ocean_mesh_prop, ocean_params);
}
//===============================================================================================================================
void Ocean::Update(float dt)
{
	//
	//Update the Pre-Pass Heightfield Compute Shader
	//

	m_pHeightfieldCS->SetComputeShader();

	// Buffers
	ID3D11ShaderResourceView* cs0_srvs[2] = {m_pSRV_H0, m_pSRV_Omega};
	m_pD3DSystem->GetDeviceContext()->CSSetShaderResources(0, 2, cs0_srvs);

	ID3D11UnorderedAccessView* cs0_uavs[1] = {m_pUAV_Ht};
	m_pD3DSystem->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));

	// Consts
	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	m_pD3DSystem->GetDeviceContext()->Map(m_pPerFrameCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	{
		assert(mapped_res.pData);
		float* per_frame_data = (float*)mapped_res.pData;
		// g_Time
		per_frame_data[0] = dt * ocean_params.time_scale;
		// g_ChoppyScale
		per_frame_data[1] = ocean_params.choppy_scale;
		// g_GridLen
		per_frame_data[2] = ocean_params.displacement_map_dim / ocean_params.patch_length;
	}
	m_pD3DSystem->GetDeviceContext()->Unmap(m_pPerFrameCB, 0);

	ID3D11Buffer* cs_cbs[2] = {m_pImmutableCB, m_pPerFrameCB};
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 2, cs_cbs);

	// Run the CS
	UINT group_count_x = (ocean_params.displacement_map_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
	UINT group_count_y = (ocean_params.displacement_map_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;

	m_pHeightfieldCS->RenderCS11( group_count_x, group_count_y, 1 );

	// Unbind resources for CS
	cs0_uavs[0] = NULL;
	m_pD3DSystem->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));
	cs0_srvs[0] = NULL;
	cs0_srvs[1] = NULL;
	m_pD3DSystem->GetDeviceContext()->CSSetShaderResources(0, 2, cs0_srvs);

	//
	// Perform the FFT
	//

	m_pOceanFFTCS->PerformFFT( m_pUAV_Dxyz, m_pSRV_Dxyz, m_pSRV_Ht );

	//
	// Wrap X,Y,Z in Ocean FFT Shader
	//

	// Set Displacement RT
	m_pDisplacementRT->SetRenderTarget();
	{
		// Set Shaders
		
		m_pOceanFFTShader->SetVertexShader();
		m_pOceanFFTShader->SwitchTo( "UpdateDisplacementPS", ST_PIXEL );
		m_pOceanFFTShader->SetPixelShader();

		// Constants
		ID3D11Buffer* ps_cbs[2] = {m_pImmutableCB, m_pPerFrameCB};
		m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 2, ps_cbs);

		// Buffer resources
		ID3D11ShaderResourceView* ps_srvs[1] = {m_pSRV_Dxyz};
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);

		UINT strides[1] = {sizeof(D3DXVECTOR4)};
		UINT offsets[1] = {0};
		m_pD3DSystem->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pQuadVB, &strides[0], &offsets[0]);
		m_pD3DSystem->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_pOceanFFTShader->SetInputLayout();

		//Perform Displacement
		m_pOceanFFTShader->RenderDraw11( 4 );

		// Unbind
		ps_srvs[0] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	}

	//
	// Generate the Normal
	//

	// Set Gradient RT
	m_pGradientRT->SetRenderTarget();
	{
		// Set Shaders

		m_pOceanFFTShader->SetVertexShader();
		m_pOceanFFTShader->SwitchTo( "GenGradientFoldingPS", ST_PIXEL );
		m_pOceanFFTShader->SetPixelShader();

		// Texture resource and sampler
		ID3D11ShaderResourceView* ps_srvs[1] = {m_pDisplacementRT->GetSRV()};
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);

		ID3D11SamplerState* samplers[1] = {m_pPointSamplerState};
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, &samplers[0]);

		//Perform Gradient
		m_pOceanFFTShader->RenderDraw11( 4 );

		// Unbind
		ps_srvs[0] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	}

	if (m_pEnablePostProcessing)
	{
		m_pD3DSystem->SetColorRenderTarget();
	}
	else
		m_pD3DSystem->SetBackBufferRenderTarget(false);

	// Pop RT and reset to the normal view
	m_pD3DSystem->ResetViewport();

	//Generate the Gradient Mip-Map
	m_pD3DSystem->GetDeviceContext()->GenerateMips( m_pGradientRT->GetSRV() );
}
//===============================================================================================================================
void Ocean::Render(float dt, Camera* camera)
{
	if (!m_wireframe)
		m_pMesh->renderShaded(camera, dt, m_pDisplacementRT->GetSRV(), m_pGradientRT->GetSRV());
	else
		m_pMesh->renderWireframe(camera, dt, m_pDisplacementRT->GetSRV());
}
//===============================================================================================================================