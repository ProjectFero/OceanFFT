#include "OceanMesh.h"

#define MESH_INDEX_2D(x, y)	(((y) + vert_rect.bottom) * (128 + 1) + (x) + vert_rect.left)
//================================================================================================================================================================
//================================================================================================================================================================
OceanMesh::OceanMesh(D3D* d3d, ocean_mesh_properties ocean_mesh_prop, const OceanParameters& ocean_param)//, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
: m_pD3DSystem(d3d), m_ocean_mesh_properties(ocean_mesh_prop), m_ocean_parameters(ocean_param),
  m_pMeshVB(0)
{
	initRenderResource();
}
//================================================================================================================================================================
OceanMesh::~OceanMesh()
{

}
//================================================================================================================================================================
DWORD OceanMesh::MeshIndex2D(float x, float y, int v_rect_left, int v_rect_bottom)
{
	return (((y) + v_rect_bottom) * (m_ocean_mesh_properties.g_MeshDim + 1) + (x) + v_rect_left);
}
//================================================================================================================================================================
void OceanMesh::initRenderResource()//const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	//Create the D3D Buffers for the ocean mesh
	createSurfaceMesh();
	createFresnelMap();
	loadTextures();

	m_pOceanSurfShader = new OceanSurfShader(m_pD3DSystem);

	//
	// Set internal constant resources in the ocean_mesh_params
	//

	////Grid side length * 2
	//m_internal_mesh_params.g_TexelLengthx2 = m_ocean_mesh_properties.g_PatchLength / m_ocean_mesh_properties.g_DisplaceMapDim * 2;
	//
	////Color
	//m_internal_mesh_params.g_SkyColor = m_ocean_mesh_properties.g_SkyColor;
	//m_internal_mesh_params.g_WaterbodyColor = m_ocean_mesh_properties.g_WaterbodyColor;
	//
	////Texcoord
	//m_internal_mesh_params.g_UVScale = 1.0f / m_ocean_mesh_properties.g_PatchLength;
	//m_internal_mesh_params.g_UVOffset = 0.5f / m_ocean_mesh_properties.g_DisplaceMapDim;
	//
	////Perlin data
	//m_internal_mesh_params.g_PerlinSize = m_ocean_mesh_properties.g_PerlinSize;
	//m_internal_mesh_params.g_PerlinAmplitude = m_ocean_mesh_properties.g_PerlinAmplitude;
	//m_internal_mesh_params.g_PerlinGradient = m_ocean_mesh_properties.g_PerlinGradient;
	//m_internal_mesh_params.g_PerlinOctave = m_ocean_mesh_properties.g_PerlinOctave;

	////Multiple reflection workaround
	//m_internal_mesh_params.g_BendParam = m_ocean_mesh_properties.g_BendParam;

	////Sun Streaks
	//m_internal_mesh_params.g_SunColor = m_ocean_mesh_properties.g_SunColor;
	//m_internal_mesh_params.g_SunDir = m_ocean_mesh_properties.g_SunDir;
	//m_internal_mesh_params.g_Shineness = m_ocean_mesh_properties.g_Shineness;

	//Constant buffers
	TextureManager* tm = TextureManager::GetInstance();

	tm->CreateConstantBuffer( m_pD3DSystem->GetDevice11(), &m_pPerCallCB, PAD16(sizeof(Const_Per_Call)), nullptr, true );

	Const_Shading shading_data;
	// Grid side length * 2
	shading_data.g_TexelLength_x2 = m_ocean_mesh_properties.g_PatchLength / m_ocean_mesh_properties.g_DisplaceMapDim * 2;;
	// Color
	shading_data.g_SkyColor = m_ocean_mesh_properties.g_SkyColor;
	shading_data.g_WaterbodyColor = m_ocean_mesh_properties.g_WaterbodyColor;
	// Texcoord
	shading_data.g_UVScale = 1.0f / m_ocean_mesh_properties.g_PatchLength;
	shading_data.g_UVOffset = 0.5f / m_ocean_mesh_properties.g_DisplaceMapDim;
	// Perlin
	shading_data.g_PerlinSize = m_ocean_mesh_properties.g_PerlinSize;
	shading_data.g_PerlinAmplitude = m_ocean_mesh_properties.g_PerlinAmplitude;
	shading_data.g_PerlinGradient = m_ocean_mesh_properties.g_PerlinGradient;
	shading_data.g_PerlinOctave = m_ocean_mesh_properties.g_PerlinOctave;
	// Multiple reflection workaround
	shading_data.g_BendParam = m_ocean_mesh_properties.g_BendParam;
	// Sun streaks
	shading_data.g_SunColor = m_ocean_mesh_properties.g_SunColor;
	shading_data.g_SunDir = m_ocean_mesh_properties.g_SunDir;
	shading_data.g_Shineness = m_ocean_mesh_properties.g_Shineness;

	tm->CreateConstantBuffer( m_pD3DSystem->GetDevice11(), &m_pShadingCB, PAD16(sizeof(Const_Shading)), &shading_data );

	//Create Samplers
	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
	sam_desc.MinLOD = 0;
	sam_desc.MaxLOD = FLT_MAX;

	m_pD3DSystem->GetDevice11()->CreateSamplerState( &sam_desc, &m_pHeightSampler );
	assert( m_pHeightSampler );

	sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_pD3DSystem->GetDevice11()->CreateSamplerState( &sam_desc, &m_pCubeSampler );
	assert( m_pCubeSampler );

	sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sam_desc.MaxAnisotropy = 8;
	m_pD3DSystem->GetDevice11()->CreateSamplerState( &sam_desc, &m_pGradientSampler );
	assert( m_pGradientSampler );

	sam_desc.MaxLOD = FLT_MAX;
	sam_desc.MaxAnisotropy = 4;
	m_pD3DSystem->GetDevice11()->CreateSamplerState( &sam_desc, &m_pPerlinSampler );
	assert( m_pPerlinSampler );

	sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_pD3DSystem->GetDevice11()->CreateSamplerState(&sam_desc, &m_pFresnelSampler);
	assert( m_pFresnelSampler );

	// State blocks
	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.FillMode = D3D11_FILL_SOLID; 
	ras_desc.CullMode = D3D11_CULL_NONE; 
	ras_desc.FrontCounterClockwise = FALSE; 
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.DepthClipEnable= TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = TRUE;
	ras_desc.AntialiasedLineEnable = FALSE;

	m_pD3DSystem->GetDevice11()->CreateRasterizerState(&ras_desc, &g_pRSState_Solid);
	assert(g_pRSState_Solid);

	ras_desc.FillMode = D3D11_FILL_WIREFRAME; 

	m_pD3DSystem->GetDevice11()->CreateRasterizerState(&ras_desc, &g_pRSState_Wireframe);
	assert(g_pRSState_Wireframe);
}
//================================================================================================================================================================
void OceanMesh::cleanupRenderResource()
{
	SAFE_RELEASE(m_pMeshIB);
	SAFE_RELEASE(m_pMeshVB);

	SAFE_RELEASE(m_pFresnelMap);
	SAFE_RELEASE(m_pSRVFresnel);
	SAFE_RELEASE(m_pSRVPerlin);
	SAFE_RELEASE(m_pSRVReflectCube);

	SAFE_RELEASE(m_pHeightSampler);
	SAFE_RELEASE(m_pGradientSampler);
	SAFE_RELEASE(m_pFresnelSampler);
	SAFE_RELEASE(m_pPerlinSampler);
	SAFE_RELEASE(m_pCubeSampler);

	m_render_list.clear();
}
//================================================================================================================================================================
void OceanMesh::createSurfaceMesh()
{
	// --------------------------------- Vertex Buffer -------------------------------
	int num_verts = (m_ocean_mesh_properties.g_MeshDim + 1) * (m_ocean_mesh_properties.g_MeshDim + 1);
	ocean_vertex* pV = new ocean_vertex[num_verts];
	assert(pV);

	int i, j;
	for (i = 0; i <= m_ocean_mesh_properties.g_MeshDim; i++)
	{
		for (j = 0; j <= m_ocean_mesh_properties.g_MeshDim; j++)
		{
			pV[i * (m_ocean_mesh_properties.g_MeshDim + 1) + j].x = (float)j;
			pV[i * (m_ocean_mesh_properties.g_MeshDim + 1) + j].y = (float)i;
		}
	}

	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = num_verts * sizeof(ocean_vertex);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;
	vb_desc.StructureByteStride = sizeof(ocean_vertex);

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = pV;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;
	
	SAFE_RELEASE(m_pMeshVB);
	m_pD3DSystem->GetDevice11()->CreateBuffer(&vb_desc, &init_data, &m_pMeshVB);
	assert(m_pMeshVB);

	SAFE_DELETE_ARRAY(pV);


	// --------------------------------- Index Buffer -------------------------------
	// The index numbers for all mesh LODs (up to 256x256)
	const int index_size_lookup[] = {0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836};

	memset(&m_mesh_patterns[0][0][0][0][0], 0, sizeof(m_mesh_patterns));

	m_lods = 0;
	for (i = m_ocean_mesh_properties.g_MeshDim; i > 1; i >>= 1)
		m_lods ++;

	// Generate patch meshes. Each patch contains two parts: the inner mesh which is a regular
	// grids in a triangle strip. The boundary mesh is constructed w.r.t. the edge degrees to
	// meet water-tight requirement.
	DWORD* index_array = new DWORD[index_size_lookup[m_lods]];
	assert(index_array);

	int offset = 0;
	int level_size = m_ocean_mesh_properties.g_MeshDim;

	// Enumerate patterns
	for (int level = 0; level <= m_lods - 2; level ++)
	{
		int left_degree = level_size;

		for (int left_type = 0; left_type < 3; left_type ++)
		{
			int right_degree = level_size;

			for (int right_type = 0; right_type < 3; right_type ++)
			{
				int bottom_degree = level_size;

				for (int bottom_type = 0; bottom_type < 3; bottom_type ++)
				{
					int top_degree = level_size;

					for (int top_type = 0; top_type < 3; top_type ++)
					{
						QuadRenderParam* pattern = &m_mesh_patterns[level][left_type][right_type][bottom_type][top_type];

						// Inner mesh (triangle strip)
						RECT inner_rect;
						inner_rect.left   = (left_degree   == level_size) ? 0 : 1;
						inner_rect.right  = (right_degree  == level_size) ? level_size : level_size - 1;
						inner_rect.bottom = (bottom_degree == level_size) ? 0 : 1;
						inner_rect.top    = (top_degree    == level_size) ? level_size : level_size - 1;

						int num_new_indices = generateInnerMesh(inner_rect, index_array + offset);

						pattern->inner_start_index = offset;
						pattern->num_inner_verts = (level_size + 1) * (level_size + 1);
						pattern->num_inner_faces = num_new_indices - 2;
						offset += num_new_indices;

						// Boundary mesh (triangle list)
						int l_degree = (left_degree   == level_size) ? 0 : left_degree;
						int r_degree = (right_degree  == level_size) ? 0 : right_degree;
						int b_degree = (bottom_degree == level_size) ? 0 : bottom_degree;
						int t_degree = (top_degree    == level_size) ? 0 : top_degree;

						RECT outer_rect = {0, level_size, level_size, 0};
						num_new_indices = generateBoundaryMesh(l_degree, r_degree, b_degree, t_degree, outer_rect, index_array + offset);

						pattern->boundary_start_index = offset;
						pattern->num_boundary_verts = (level_size + 1) * (level_size + 1);
						pattern->num_boundary_faces = num_new_indices / 3;
						offset += num_new_indices;

						top_degree /= 2;
					}
					bottom_degree /= 2;
				}
				right_degree /= 2;
			}
			left_degree /= 2;
		}
		level_size /= 2;
	}

	assert(offset == index_size_lookup[m_lods]);

	D3D11_BUFFER_DESC ib_desc;
	ib_desc.ByteWidth = index_size_lookup[m_lods] * sizeof(DWORD);
	ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
	ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_desc.CPUAccessFlags = 0;
	ib_desc.MiscFlags = 0;
	ib_desc.StructureByteStride = sizeof(DWORD);

	init_data.pSysMem = index_array;

	//SAFE_RELEASE(m_pMeshIB);
	m_pD3DSystem->GetDevice11()->CreateBuffer(&ib_desc, &init_data, &m_pMeshIB);
	assert(m_pMeshIB);

	SAFE_DELETE_ARRAY(index_array);
}
//================================================================================================================================================================
void OceanMesh::createFresnelMap()
{
	DWORD* buffer = new DWORD[FRESNEL_TEX_SIZE];
	for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
	{
		float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
		// Using water's refraction index 1.33
		DWORD fresnel = (DWORD)(D3DXFresnelTerm(cos_a, 1.33f) * 255);

		DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), m_ocean_mesh_properties.g_Skyblending) * 255);

		buffer[i] = (sky_blend << 8) | fresnel;
	}
	
	D3D11_TEXTURE1D_DESC tex_desc;
	tex_desc.Width = FRESNEL_TEX_SIZE;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = buffer;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	m_pD3DSystem->GetDevice11()->CreateTexture1D(&tex_desc, &init_data, &m_pFresnelMap);
	assert(m_pFresnelMap);

	SAFE_DELETE_ARRAY(buffer);

	// Create shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srv_desc.Texture1D.MipLevels = 1;
	srv_desc.Texture1D.MostDetailedMip = 0;

	m_pD3DSystem->GetDevice11()->CreateShaderResourceView(m_pFresnelMap, &srv_desc, &m_pSRVFresnel);
	assert(m_pSRVFresnel);
}
//================================================================================================================================================================
void OceanMesh::loadTextures()
{
	D3DX11CreateShaderResourceViewFromFile(m_pD3DSystem->GetDevice11(), "Textures\\perlin_noise.dds", NULL, NULL, &m_pSRVPerlin, NULL);
	assert(m_pSRVPerlin);
	//sky_cube
	//reflect_cube
	D3DX11CreateShaderResourceViewFromFile(m_pD3DSystem->GetDevice11(), "Textures\\reflect_cube.dds", NULL, NULL, &m_pSRVReflectCube, NULL);
	assert(m_pSRVReflectCube);

	D3DX11CreateShaderResourceViewFromFile(m_pD3DSystem->GetDevice11(), "Textures\\waternormal.dds", NULL, NULL, &m_pSRVNormalMap, NULL);
	assert(m_pSRVNormalMap);
}
//================================================================================================================================================================
void OceanMesh::renderShaded(Camera* camera, float time, ID3D11ShaderResourceView* displacement_map, ID3D11ShaderResourceView* gradient_map)
{
	ID3D11DeviceContext* pd3dContext = m_pD3DSystem->GetDeviceContext();

	//
	// Build rendering list
	//

	m_render_list.clear();

	float ocean_extent = m_ocean_mesh_properties.g_PatchLength * (1 << m_ocean_mesh_properties.g_FurthestCover);

	QuadNode root_node = {D3DXVECTOR2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0, {-1, -1, -1, -1}};

	BuildNodeList( root_node, camera );

	//
	// Setup matrices
	//

	D3DXMATRIX view = MathHelper::ConvertToD3DXMatrix( camera->View() );
	D3DXMATRIX proj = MathHelper::ConvertToD3DXMatrix( camera->Proj() );
	D3DXMATRIX reflView = MathHelper::ConvertToD3DXMatrix( camera->ReflectionView() );

	D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * view;
	D3DXMATRIX matProj = proj;
	D3DXMATRIX matReflView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * reflView;

	//
	// Set Shaders
	//

	m_pOceanSurfShader->SetVertexShader();

	m_pOceanSurfShader->SwitchTo( "OceanSurfPS", ST_PIXEL );

	m_pOceanSurfShader->SetPixelShader();

	//
	// Set Shader resource textures
	//

	ID3D11ShaderResourceView* vs_srvs[2] = {displacement_map, m_pSRVPerlin};
	pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

	ID3D11ShaderResourceView* ps_srvs[7] = {m_pSRVPerlin, gradient_map, m_pSRVFresnel, m_pSRVReflectionMap, m_pSRVRefractionMap, m_pSRVNormalMap, m_pSRVReflectCube};
    pd3dContext->PSSetShaderResources(1, 7, &ps_srvs[0]);

	// Samplers
	ID3D11SamplerState* vs_samplers[2] = {m_pHeightSampler, m_pPerlinSampler};
	pd3dContext->VSSetSamplers(0, 2, &vs_samplers[0]);

	ID3D11SamplerState* ps_samplers[4] = {m_pPerlinSampler, m_pGradientSampler, m_pFresnelSampler, m_pCubeSampler};
	pd3dContext->PSSetSamplers(1, 4, &ps_samplers[0]);

	//
	// Set Input Assembler
	//

	ID3D11Buffer* vbs[1] = {m_pMeshVB};
	UINT strides[1] = {sizeof(ocean_vertex)};
	UINT offsets[1] = {0};
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	pd3dContext->IASetIndexBuffer(m_pMeshIB, DXGI_FORMAT_R32_UINT, 0);
	m_pOceanSurfShader->SetInputLayout();

	// State blocks
	//pd3dContext->RSSetState(g_pRSState_Solid);
	m_pD3DSystem->SetOceanRenderState( true );
	m_pD3DSystem->TurnOffCulling();

	//
	// Set Constants
	//
	ID3D11Buffer* cbs[1] = {m_pShadingCB};
	pd3dContext->VSSetConstantBuffers(2, 1, cbs);
	pd3dContext->PSSetConstantBuffers(2, 1, cbs);

	//Assuming the center of the ocean is (0, 0, 0)
	for (int i = 0; i < (int)m_render_list.size(); i++)
	{
		QuadNode& node = m_render_list[i];
		
		if (!isLeaf(node))
			continue;

		// Check adjacent patches and select mesh pattern
		QuadRenderParam& render_param = SelectMeshPattern(node);

		// Find the right LOD to render
		int level_size = m_ocean_mesh_properties.g_MeshDim;
		for (int lod = 0; lod < node.lod; lod++)
			level_size >>= 1;

		// Matrices and constants
		Const_Per_Call call_consts;

		call_consts.g_SeaLevel = m_SeaLevel;

		// Expand of the local coordinate to world space patch size
		D3DXMATRIX matScale;
		D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 0);
		D3DXMatrixTranspose(&call_consts.g_matLocal, &matScale);

		// WVP matrix
		D3DXMATRIX matWorld;
		D3DXMatrixTranslation(&matWorld, node.bottom_left.x, node.bottom_left.y, 0);
		D3DXMATRIX matWVP = matWorld * matView * matProj;
		D3DXMatrixTranspose(&call_consts.g_matWorldViewProj, &matWVP);

		// World matrix
		call_consts.g_matWorld = matWorld;

		// WRVP matrix
		D3DXMATRIX matWRVP = matWorld * matReflView * matProj;
		D3DXMatrixTranspose(&call_consts.g_matWorldReflectionViewProj, &matWRVP);

		// Texcoord for perlin noise
		D3DXVECTOR2 uv_base = node.bottom_left / m_ocean_mesh_properties.g_PatchLength * m_ocean_mesh_properties.g_PerlinSize;
		call_consts.g_UVBase = uv_base;

		// Constant g_PerlinSpeed need to be adjusted mannually
		D3DXVECTOR2 perlin_move = -m_ocean_mesh_properties.g_WindDir * time * m_ocean_mesh_properties.g_PerlinSpeed;
		call_consts.g_PerlinMovement = perlin_move;

		// Eye point
		D3DXMATRIX matInvWV = matWorld * matView;
		D3DXMatrixInverse(&matInvWV, NULL, &matInvWV);
		D3DXVECTOR3 vLocalEye(0, 0, 0);
		D3DXVec3TransformCoord(&vLocalEye, &vLocalEye, &matInvWV);
		call_consts.g_LocalEye = vLocalEye;

		// Update constant buffer
		D3D11_MAPPED_SUBRESOURCE mapped_res;            
		pd3dContext->Map(m_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(Const_Per_Call*)mapped_res.pData = call_consts;
		}
		pd3dContext->Unmap(m_pPerCallCB, 0);

		cbs[0] = m_pPerCallCB;
		pd3dContext->VSSetConstantBuffers(4, 1, cbs);
		pd3dContext->PSSetConstantBuffers(4, 1, cbs);

		//
		// Set internal mesh parameter per call Shader resources
		//
		
		m_pOceanSurfShader->SetEnableOceanSurfShading( true );
		m_pOceanSurfShader->SetEnableOceanSurfWireframe( false );

		//
		// Perform draw call and shading
		//

		// Perform draw call
		if (render_param.num_inner_faces > 0)
		{
			// Inner mesh of the patch
			pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			//pd3dContext->DrawIndexed(render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
			m_pOceanSurfShader->Render11( m_pD3DSystem, render_param.num_inner_faces + 2, render_param.inner_start_index );
		}

		if (render_param.num_boundary_faces > 0)
		{
			// Boundary mesh of the patch
			pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//pd3dContext->DrawIndexed(render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
			m_pOceanSurfShader->Render11( m_pD3DSystem, render_param.num_boundary_faces * 3, render_param.boundary_start_index );
		}
	}

	// Unbind
	vs_srvs[0] = NULL;
	vs_srvs[1] = NULL;
	pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

	ps_srvs[0] = NULL;
	ps_srvs[1] = NULL;
	ps_srvs[2] = NULL;
	ps_srvs[3] = NULL;
    pd3dContext->PSSetShaderResources(1, 4, &ps_srvs[0]);

	//Reset the render state
	m_pD3DSystem->SetOceanRenderState( false );
	m_pD3DSystem->TurnOnCulling();
}
//================================================================================================================================================================
void OceanMesh::renderWireframe(Camera* camera, float time, ID3D11ShaderResourceView* displacement_map)
{
	ID3D11DeviceContext* pd3dContext = m_pD3DSystem->GetDeviceContext();

	//
	// Build rendering list
	//

	m_render_list.clear();

	float ocean_extent = m_ocean_mesh_properties.g_PatchLength * (1 << m_ocean_mesh_properties.g_FurthestCover);

	QuadNode root_node = {D3DXVECTOR2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0, {-1, -1, -1, -1}};

	BuildNodeList( root_node, camera );

	//
	// Setup matrices
	//

	D3DXMATRIX view = MathHelper::ConvertToD3DXMatrix( camera->View() );
	D3DXMATRIX proj = MathHelper::ConvertToD3DXMatrix( camera->Proj() );
	D3DXMATRIX reflView = MathHelper::ConvertToD3DXMatrix( camera->ReflectionView() );

	D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * view;
	D3DXMATRIX matProj = proj;
	D3DXMATRIX matReflView = reflView;

	//
	// Set Shaders
	//

	m_pOceanSurfShader->SetVertexShader();

	m_pOceanSurfShader->SwitchTo( "WireframePS", ST_PIXEL );

	m_pOceanSurfShader->SetPixelShader();

	//
	// Set Shader resource textures
	//

	ID3D11ShaderResourceView* vs_srvs[2] = {displacement_map, m_pSRVPerlin};
	pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

	// Samplers
	ID3D11SamplerState* vs_samplers[2] = {m_pHeightSampler, m_pPerlinSampler};
	pd3dContext->VSSetSamplers(0, 2, &vs_samplers[0]);

	ID3D11SamplerState* ps_samplers[4] = {NULL, NULL, NULL, NULL};
	pd3dContext->PSSetSamplers(1, 4, &ps_samplers[0]);

	//
	// Set Input Assembler
	//

	ID3D11Buffer* vbs[1] = {m_pMeshVB};
	UINT strides[1] = {sizeof(ocean_vertex)};
	UINT offsets[1] = {0};
	pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	pd3dContext->IASetIndexBuffer(m_pMeshIB, DXGI_FORMAT_R32_UINT, 0);
	m_pOceanSurfShader->SetInputLayout();

	// State blocks
	//pd3dContext->RSSetState(g_pRSState_Wireframe);
	m_pD3DSystem->SetOceanRenderState( true );
	m_pD3DSystem->TurnOnWireframe();

	//
	// Set Constants
	//
	ID3D11Buffer* cbs[1] = {m_pShadingCB};
	pd3dContext->VSSetConstantBuffers(2, 1, cbs);
	pd3dContext->PSSetConstantBuffers(2, 1, cbs);

	//Assuming the center of the ocean is (0, 0, 0)
	for (int i = 0; i < (int)m_render_list.size(); i++)
	{
		QuadNode& node = m_render_list[i];
		
		if (!isLeaf(node))
			continue;

		// Check adjacent patches and select mesh pattern
		QuadRenderParam& render_param = SelectMeshPattern(node);

		// Find the right LOD to render
		int level_size = m_ocean_mesh_properties.g_MeshDim;
		for (int lod = 0; lod < node.lod; lod++)
			level_size >>= 1;

		// Matrices and constants
		Const_Per_Call call_consts;

		call_consts.g_SeaLevel = m_SeaLevel;

		// Expand of the local coordinate to world space patch size
		D3DXMATRIX matScale;
		D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 0);
		D3DXMatrixTranspose(&call_consts.g_matLocal, &matScale);

		// WVP matrix
		D3DXMATRIX matWorld;
		D3DXMatrixTranslation(&matWorld, node.bottom_left.x, node.bottom_left.y, 0);
		D3DXMATRIX matWVP = matWorld * matView * matProj;
		D3DXMatrixTranspose(&call_consts.g_matWorldViewProj, &matWVP);

		// World matrix
		call_consts.g_matWorld = matWorld;

		// WRVP matrix
		D3DXMATRIX matWRVP = matWorld * matReflView * matProj;
		D3DXMatrixTranspose(&call_consts.g_matWorldReflectionViewProj, &matWRVP);

		// Texcoord for perlin noise
		D3DXVECTOR2 uv_base = node.bottom_left / m_ocean_mesh_properties.g_PatchLength * m_ocean_mesh_properties.g_PerlinSize;
		call_consts.g_UVBase = uv_base;

		// Constant g_PerlinSpeed need to be adjusted mannually
		D3DXVECTOR2 perlin_move = -m_ocean_mesh_properties.g_WindDir * time * m_ocean_mesh_properties.g_PerlinSpeed;
		call_consts.g_PerlinMovement = perlin_move;

		// Eye point
		D3DXMATRIX matInvWV = matWorld * matView;
		D3DXMatrixInverse(&matInvWV, NULL, &matInvWV);
		D3DXVECTOR3 vLocalEye(0, 0, 0);
		D3DXVec3TransformCoord(&vLocalEye, &vLocalEye, &matInvWV);
		call_consts.g_LocalEye = vLocalEye;

		// Update constant buffer
		D3D11_MAPPED_SUBRESOURCE mapped_res;            
		pd3dContext->Map(m_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(Const_Per_Call*)mapped_res.pData = call_consts;
		}
		pd3dContext->Unmap(m_pPerCallCB, 0);

		cbs[0] = m_pPerCallCB;
		pd3dContext->VSSetConstantBuffers(4, 1, cbs);
		pd3dContext->PSSetConstantBuffers(4, 1, cbs);

		//
		// Set internal mesh parameter per call Shader resources
		//
		
		m_pOceanSurfShader->SetEnableOceanSurfShading( false );
		m_pOceanSurfShader->SetEnableOceanSurfWireframe( true );

		//
		// Perform draw call and shading
		//

		// Perform draw call
		if (render_param.num_inner_faces > 0)
		{
			// Inner mesh of the patch
			pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			//pd3dContext->DrawIndexed(render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
			m_pOceanSurfShader->Render11( m_pD3DSystem, render_param.num_inner_faces + 2, render_param.inner_start_index );
		}

		if (render_param.num_boundary_faces > 0)
		{
			// Boundary mesh of the patch
			pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//pd3dContext->DrawIndexed(render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
			m_pOceanSurfShader->Render11( m_pD3DSystem, render_param.num_boundary_faces * 3, render_param.boundary_start_index );
		}
	}

	// Unbind
	vs_srvs[0] = NULL;
	vs_srvs[1] = NULL;
	pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

	// Restore states
	//Reset the render state
	m_pD3DSystem->SetOceanRenderState( false );
	m_pD3DSystem->TurnOnCulling();
}
//================================================================================================================================================================
int OceanMesh::generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree, RECT vert_rect, DWORD* output)
{
	// Triangle list for bottom boundary
	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;

	if (bottom_degree > 0)
	{
		int b_step = width / bottom_degree;

		for (i = 0; i < width; i += b_step)
		{
			output[counter++] = MESH_INDEX_2D(i, 0);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(i + b_step / 2, 1);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(i + b_step, 0);//, vert_rect.left, vert_rect.bottom);

			for (j = 0; j < b_step / 2; j ++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(i, 0);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j, 1);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j + 1, 1);//, vert_rect.left, vert_rect.bottom);
			}

			for (j = b_step / 2; j < b_step; j ++)
			{
				if (i == width - b_step && j == b_step - 1 && right_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(i + b_step, 0);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j, 1);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j + 1, 1);//, vert_rect.left, vert_rect.bottom);
			}
		}
	}

	// Right boundary
	int height = vert_rect.top - vert_rect.bottom;

	if (right_degree > 0)
	{
		int r_step = height / right_degree;

		for (i = 0; i < height; i += r_step)
		{
			output[counter++] = MESH_INDEX_2D(width, i);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(width - 1, i + r_step / 2);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(width, i + r_step);//, vert_rect.left, vert_rect.bottom);

			for (j = 0; j < r_step / 2; j ++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(width, i);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);//, vert_rect.left, vert_rect.bottom);
			}

			for (j = r_step / 2; j < r_step; j ++)
			{
				if (i == height - r_step && j == r_step - 1 && top_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(width, i + r_step);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);//, vert_rect.left, vert_rect.bottom);
			}
		}
	}

	// Top boundary
	if (top_degree > 0)
	{
		int t_step = width / top_degree;

		for (i = 0; i < width; i += t_step)
		{
			output[counter++] = MESH_INDEX_2D(i, height);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(i + t_step / 2, height - 1);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(i + t_step, height);//, vert_rect.left, vert_rect.bottom);

			for (j = 0; j < t_step / 2; j ++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(i, height);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j, height - 1);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);//, vert_rect.left, vert_rect.bottom);
			}

			for (j = t_step / 2; j < t_step; j ++)
			{
				if (i == width - t_step && j == t_step - 1 && right_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(i + t_step, height);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j, height - 1);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);//, vert_rect.left, vert_rect.bottom);
			}
		}
	}

	// Left boundary
	if (left_degree > 0)
	{
		int l_step = height / left_degree;

		for (i = 0; i < height; i += l_step)
		{
			output[counter++] = MESH_INDEX_2D(0, i);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(1, i + l_step / 2);//, vert_rect.left, vert_rect.bottom);
			output[counter++] = MESH_INDEX_2D(0, i + l_step);//, vert_rect.left, vert_rect.bottom);

			for (j = 0; j < l_step / 2; j ++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(0, i);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(1, i + j);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(1, i + j + 1);//, vert_rect.left, vert_rect.bottom);
			}

			for (j = l_step / 2; j < l_step; j ++)
			{
				if (i == height - l_step && j == l_step - 1 && top_degree > 0)
					continue;

				output[counter++] = MESH_INDEX_2D(0, i + l_step);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(1, i + j);//, vert_rect.left, vert_rect.bottom);
				output[counter++] = MESH_INDEX_2D(1, i + j + 1);//, vert_rect.left, vert_rect.bottom);
			}
		}
	}

	return counter;
}
//================================================================================================================================================================
int OceanMesh::generateInnerMesh(RECT vert_rect, DWORD* output)
{
	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;
	int height = vert_rect.top - vert_rect.bottom;

	bool reverse = false;
	for (i = 0; i < height; i++)
	{
		if (reverse == false)
		{
			output[counter++] = MeshIndex2D(0, i, vert_rect.left, vert_rect.bottom);
			output[counter++] = MeshIndex2D(0, i + 1, vert_rect.left, vert_rect.bottom);
			for (j = 0; j < width; j++)
			{
				output[counter++] = MeshIndex2D(j + 1, i, vert_rect.left, vert_rect.bottom);
				output[counter++] = MeshIndex2D(j + 1, i + 1, vert_rect.left, vert_rect.bottom);
			}
		}
		else
		{
			output[counter++] = MeshIndex2D(width, i, vert_rect.left, vert_rect.bottom);
			output[counter++] = MeshIndex2D(width, i + 1, vert_rect.left, vert_rect.bottom);
			for (j = width - 1; j >= 0; j--)
			{
				output[counter++] = MeshIndex2D(j, i, vert_rect.left, vert_rect.bottom);
				output[counter++] = MeshIndex2D(j, i + 1, vert_rect.left, vert_rect.bottom);
			}
		}

		reverse = !reverse;
	}

	return counter;
}
//================================================================================================================================================================
bool OceanMesh::checkNodeVisibility(const QuadNode& quad_node, Camera* camera)
{
	//
	// Plane equation setup
	//

	D3DXMATRIX view = MathHelper::ConvertToD3DXMatrix( camera->View() );
	D3DXMATRIX proj = MathHelper::ConvertToD3DXMatrix( camera->Proj() );

	D3DXMATRIX matProj = proj;

	// Left plane
	float fov_x = atan(1.0f / matProj(0, 0));
	D3DXVECTOR4 plane_left(cos(fov_x), 0, sin(fov_x), 0);
	// Right plane
	D3DXVECTOR4 plane_right(-cos(fov_x), 0, sin(fov_x), 0);

	// Bottom plane
	float fov_y = atan(1.0f / matProj(1, 1));
	D3DXVECTOR4 plane_bottom(0, cos(fov_y), sin(fov_y), 0);
	// Top plane
	D3DXVECTOR4 plane_top(0, -cos(fov_y), sin(fov_y), 0);

	// Test quad corners against view frustum in view space
	D3DXVECTOR4 corner_verts[4];
	corner_verts[0] = D3DXVECTOR4(quad_node.bottom_left.x, quad_node.bottom_left.y, 0, 1);
	corner_verts[1] = corner_verts[0] + D3DXVECTOR4(quad_node.length, 0, 0, 0);
	corner_verts[2] = corner_verts[0] + D3DXVECTOR4(quad_node.length, quad_node.length, 0, 0);
	corner_verts[3] = corner_verts[0] + D3DXVECTOR4(0, quad_node.length, 0, 0);

	D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * view;
	D3DXVec4Transform(&corner_verts[0], &corner_verts[0], &matView);
	D3DXVec4Transform(&corner_verts[1], &corner_verts[1], &matView);
	D3DXVec4Transform(&corner_verts[2], &corner_verts[2], &matView);
	D3DXVec4Transform(&corner_verts[3], &corner_verts[3], &matView);

	int test_value = -2000;

	// Test against eye plane
	if (corner_verts[0].z < test_value && corner_verts[1].z < test_value && corner_verts[2].z < test_value && corner_verts[3].z < test_value)
		return false;

	// Test against left plane
	float dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_left);
	float dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_left);
	float dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_left);
	float dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_left);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
		return false;

	// Test against right plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_right);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_right);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_right);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_right);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
		return false;

	// Test against bottom plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_bottom);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_bottom);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_bottom);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_bottom);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
		return false;

	// Test against top plane
	dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_top);
	dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_top);
	dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_top);
	dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_top);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
		return false;

	return true;
}
//================================================================================================================================================================
float OceanMesh::estimateGridCoverage(const QuadNode& quad_node, Camera* camera, float screen_area)
{
	// Estimate projected area

	// Test 16 points on the quad and find out the biggest one.
	const static float sample_pos[16][2] =
	{
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1},
		{0.5f, 0.333f},
		{0.25f, 0.667f},
		{0.75f, 0.111f},
		{0.125f, 0.444f},
		{0.625f, 0.778f},
		{0.375f, 0.222f},
		{0.875f, 0.556f},
		{0.0625f, 0.889f},
		{0.5625f, 0.037f},
		{0.3125f, 0.37f},
		{0.8125f, 0.704f},
		{0.1875f, 0.148f},
	};

	D3DXMATRIX view = MathHelper::ConvertToD3DXMatrix( camera->View() );
	D3DXMATRIX proj = MathHelper::ConvertToD3DXMatrix( camera->Proj() );

	XMFLOAT3 eye = camera->Position();

	D3DXMATRIX matProj = proj;
	D3DXVECTOR3 eye_point = D3DXVECTOR3(eye.x, eye.y, eye.z);
	eye_point = D3DXVECTOR3(eye_point.x, eye_point.z, eye_point.y);
	float grid_len_world = quad_node.length / m_ocean_mesh_properties.g_MeshDim;

	float max_area_proj = 0;
	for (int i = 0; i < 16; i++)
	{
		D3DXVECTOR3 test_point(quad_node.bottom_left.x + quad_node.length * sample_pos[i][0], quad_node.bottom_left.y + quad_node.length * sample_pos[i][1], 0);
		D3DXVECTOR3 eye_vec = test_point - eye_point;
		float dist = D3DXVec3Length(&eye_vec);

		float area_world = grid_len_world * grid_len_world;// * abs(eye_point.z) / sqrt(nearest_sqr_dist);
		float area_proj = area_world * matProj(0, 0) * matProj(1, 1) / (dist * dist);

		if (max_area_proj < area_proj)
			max_area_proj = area_proj;
	}

	float pixel_coverage = max_area_proj * screen_area * 0.25f;

	return pixel_coverage;
}
//================================================================================================================================================================
bool OceanMesh::isLeaf(const QuadNode& quad_node)
{
	return (quad_node.sub_node[0] == -1 && quad_node.sub_node[1] == -1 && quad_node.sub_node[2] == -1 && quad_node.sub_node[3] == -1);
}
//================================================================================================================================================================
int OceanMesh::searchLeaf(const vector<QuadNode>& node_list, const D3DXVECTOR2& point)
{
	int index = -1;
	
	int size = (int)node_list.size();
	QuadNode node = node_list[size - 1];

	while (!isLeaf(node))
	{
		bool found = false;

		for (int i = 0; i < 4; i++)
		{
			index = node.sub_node[i];
			if (index == -1)
				continue;

			QuadNode sub_node = node_list[index];
			if (point.x >= sub_node.bottom_left.x && point.x <= sub_node.bottom_left.x + sub_node.length &&
				point.y >= sub_node.bottom_left.y && point.y <= sub_node.bottom_left.y + sub_node.length)
			{
				node = sub_node;
				found = true;
				break;
			}
		}

		if (!found)
			return -1;
	}

	return index;
}
//================================================================================================================================================================
OceanMesh::QuadRenderParam& OceanMesh::SelectMeshPattern(const QuadNode& quad_node)
{
	// Check 4 adjacent quad.
	D3DXVECTOR2 point_left = quad_node.bottom_left + D3DXVECTOR2(-m_ocean_mesh_properties.g_PatchLength * 0.5f, quad_node.length * 0.5f);
	int left_adj_index = searchLeaf(m_render_list, point_left);

	D3DXVECTOR2 point_right = quad_node.bottom_left + D3DXVECTOR2(quad_node.length + m_ocean_mesh_properties.g_PatchLength * 0.5f, quad_node.length * 0.5f);
	int right_adj_index = searchLeaf(m_render_list, point_right);

	D3DXVECTOR2 point_bottom = quad_node.bottom_left + D3DXVECTOR2(quad_node.length * 0.5f, -m_ocean_mesh_properties.g_PatchLength * 0.5f);
	int bottom_adj_index = searchLeaf(m_render_list, point_bottom);

	D3DXVECTOR2 point_top = quad_node.bottom_left + D3DXVECTOR2(quad_node.length * 0.5f, quad_node.length + m_ocean_mesh_properties.g_PatchLength * 0.5f);
	int top_adj_index = searchLeaf(m_render_list, point_top);

	int left_type = 0;
	if (left_adj_index != -1 && m_render_list[left_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = m_render_list[left_adj_index];
		float scale = adj_node.length / quad_node.length * (m_ocean_mesh_properties.g_MeshDim >> quad_node.lod) / (m_ocean_mesh_properties.g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			left_type = 2;
		else if (scale > 1.999f)
			left_type = 1;
	}

	int right_type = 0;
	if (right_adj_index != -1 && m_render_list[right_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = m_render_list[right_adj_index];
		float scale = adj_node.length / quad_node.length * (m_ocean_mesh_properties.g_MeshDim >> quad_node.lod) / (m_ocean_mesh_properties.g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			right_type = 2;
		else if (scale > 1.999f)
			right_type = 1;
	}

	int bottom_type = 0;
	if (bottom_adj_index != -1 && m_render_list[bottom_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = m_render_list[bottom_adj_index];
		float scale = adj_node.length / quad_node.length * (m_ocean_mesh_properties.g_MeshDim >> quad_node.lod) / (m_ocean_mesh_properties.g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			bottom_type = 2;
		else if (scale > 1.999f)
			bottom_type = 1;
	}

	int top_type = 0;
	if (top_adj_index != -1 && m_render_list[top_adj_index].length > quad_node.length * 0.999f)
	{
		QuadNode adj_node = m_render_list[top_adj_index];
		float scale = adj_node.length / quad_node.length * (m_ocean_mesh_properties.g_MeshDim >> quad_node.lod) / (m_ocean_mesh_properties.g_MeshDim >> adj_node.lod);
		if (scale > 3.999f)
			top_type = 2;
		else if (scale > 1.999f)
			top_type = 1;
	}

	// Check lookup table, [L][R][B][T]
	return m_mesh_patterns[quad_node.lod][left_type][right_type][bottom_type][top_type];
}
//================================================================================================================================================================
int OceanMesh::BuildNodeList(QuadNode& quad_node, Camera* camera)
{
	// Check against view frustum
	if (!checkNodeVisibility(quad_node, camera))
		return -1;

	// Estimate the min grid coverage
	UINT num_vps = 1;
	D3D11_VIEWPORT vp;
	//DXUTGetD3D11DeviceContext()->RSGetViewports(&num_vps, &vp);
	m_pD3DSystem->GetDeviceContext()->RSGetViewports(&num_vps, &vp);
	float min_coverage = estimateGridCoverage(quad_node, camera, (float)vp.Width * vp.Height);

	// Recursively attatch sub-nodes.
	bool visible = true;
	if (min_coverage > m_ocean_mesh_properties.g_UpperGridCoverage && quad_node.length > m_ocean_mesh_properties.g_PatchLength)
	{
		// Recursive rendering for sub-quads.
		QuadNode sub_node_0 = {quad_node.bottom_left, quad_node.length / 2, 0, {-1, -1, -1, -1}};
		quad_node.sub_node[0] = BuildNodeList(sub_node_0, camera);

		QuadNode sub_node_1 = {quad_node.bottom_left + D3DXVECTOR2(quad_node.length/2, 0), quad_node.length / 2, 0, {-1, -1, -1, -1}};
		quad_node.sub_node[1] = BuildNodeList(sub_node_1, camera);

		QuadNode sub_node_2 = {quad_node.bottom_left + D3DXVECTOR2(quad_node.length/2, quad_node.length/2), quad_node.length / 2, 0, {-1, -1, -1, -1}};
		quad_node.sub_node[2] = BuildNodeList(sub_node_2, camera);

		QuadNode sub_node_3 = {quad_node.bottom_left + D3DXVECTOR2(0, quad_node.length/2), quad_node.length / 2, 0, {-1, -1, -1, -1}};
		quad_node.sub_node[3] = BuildNodeList(sub_node_3, camera);

		visible = !isLeaf(quad_node);
	}

	if (visible)
	{
		// Estimate mesh LOD
		int lod = 0;
		for (lod = 0; lod < m_lods - 1; lod++)
		{
			if (min_coverage > m_ocean_mesh_properties.g_UpperGridCoverage)
				break;
			min_coverage *= 4;
		}

		// We don't use 1x1 and 2x2 patch. So the highest level is g_Lods - 2.
		quad_node.lod = min(lod, m_lods - 2);
	}
	else
		return -1;

	// Insert into the list
	int position = (int)m_render_list.size();
	m_render_list.push_back(quad_node);

	return position;
}
//================================================================================================================================================================
float OceanMesh::dot(XMFLOAT3 v1, XMFLOAT3 v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}
//================================================================================================================================================================
float OceanMesh::dot(XMFLOAT4 v1, XMFLOAT4 v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w));
}
//================================================================================================================================================================