//================================================================================================================================================================
// OceanMesh
//
// Based on the NVIDIA sample for OceanCS
//================================================================================================================================================================
// History
//
// -Created on 6/21/2014 by Dustin Watson
//================================================================================================================================================================
#ifndef __OCEANMESH_H
#define __OCEANMESH_H
//================================================================================================================================================================
//================================================================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "OceanParameters.h"
#include "Camera.h"
#include "OceanSurfShader.h"
#include "TextureManager.h"
#include "MathHelper.h"

//================================================================================================================================================================
//================================================================================================================================================================

//
// Globals
//

#define FRESNEL_TEX_SIZE  256
#define PERLIN_TEX_SIZE   64

//================================================================================================================================================================
//================================================================================================================================================================
class OceanMesh
{
	struct Const_Per_Call
	{
		D3DXMATRIX	g_matLocal;
		D3DXMATRIX	g_matWorldViewProj;
		D3DXMATRIX	g_matWorld;
		D3DXMATRIX	g_matWorldReflectionViewProj;
		D3DXVECTOR2 g_UVBase;
		D3DXVECTOR2 g_PerlinMovement;
		D3DXVECTOR3	g_LocalEye;
		float       g_SeaLevel;
	};

	struct ocean_vertex
	{
		float x;
		float y;
	};

	struct QuadNode
	{
		D3DXVECTOR2 bottom_left;
		float length;
		int lod;
		int sub_node[4];
	};

	struct QuadRenderParam
	{
		UINT num_inner_verts;
		UINT num_inner_faces;
		UINT inner_start_index;

		UINT num_boundary_verts;
		UINT num_boundary_faces;
		UINT boundary_start_index;
	};

	struct Const_Shading
	{
		// Water-reflected sky color
		D3DXVECTOR3		g_SkyColor;
		float			unused0;
		// The color of bottomless water body
		D3DXVECTOR3		g_WaterbodyColor;

		// The strength, direction and color of sun streak
		float			g_Shineness;
		D3DXVECTOR3		g_SunDir;
		float			unused1;
		D3DXVECTOR3		g_SunColor;
		float			unused2;
	
		// The parameter is used for fixing an artifact
		D3DXVECTOR3		g_BendParam;

		// Perlin noise for distant wave crest
		float			g_PerlinSize;
		D3DXVECTOR3		g_PerlinAmplitude;
		float			unused3;
		D3DXVECTOR3		g_PerlinOctave;
		float			unused4;
		D3DXVECTOR3		g_PerlinGradient;

		// Constants for calculating texcoord from position
		float			g_TexelLength_x2;
		float			g_UVScale;
		float			g_UVOffset;
	};
public:
	OceanMesh(D3D* d3d, ocean_mesh_properties ocean_mesh_prop, const OceanParameters& ocean_param);//, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	~OceanMesh();

	// init & cleanup
	void initRenderResource();//const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void cleanupRenderResource();
	
	// create a triangle strip mesh for ocean surface.
	void createSurfaceMesh();
	
	// create color/fresnel lookup table.
	void createFresnelMap();
	
	// create perlin noise texture for far-sight rendering
	void loadTextures();
	
	// Rendering routines
	void renderShaded(Camera* camera, float time, ID3D11ShaderResourceView* displacement_map, ID3D11ShaderResourceView* gradient_map);
	void renderWireframe(Camera* camera, float time, ID3D11ShaderResourceView* displacement_map);

	void SetReflectionMap(ID3D11ShaderResourceView* srv) { m_pSRVReflectionMap = srv; }
	void SetRefractionMap(ID3D11ShaderResourceView* srv) { m_pSRVRefractionMap = srv; }

	void SetSeaLevel(float level) { m_SeaLevel = level; }

private:

	//
	// Private Funcs
	//

	QuadRenderParam& SelectMeshPattern(const QuadNode& quad_node);
	int BuildNodeList(QuadNode& quad_node, Camera* camera);
	
	// Generate boundary mesh for a patch. Return the number of generated indices
	int generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree,
						 RECT vert_rect, DWORD* output);

	// Generate boundary mesh for a patch. Return the number of generated indices
	int generateInnerMesh(RECT vert_rect, DWORD* output);

	bool checkNodeVisibility(const QuadNode& quad_node, Camera* camera);

	float estimateGridCoverage(const QuadNode& quad_node, Camera* camera, float screen_area);

	bool isLeaf(const QuadNode& quad_node);

	int searchLeaf(const vector<QuadNode>& node_list, const D3DXVECTOR2& point);

	DWORD MeshIndex2D(float x, float y, int v_rect_left, int v_rect_bottom);

	float dot(XMFLOAT3 v1, XMFLOAT3 v2);
	float dot(XMFLOAT4 v1, XMFLOAT4 v2);

	//
	// Variables
	//

	D3D* m_pD3DSystem;

	//The Ocean Surf Shader
	OceanSurfShader* m_pOceanSurfShader;

	ID3D11RasterizerState* g_pRSState_Solid;
	ID3D11RasterizerState* g_pRSState_Wireframe;

	//ocean_mesh_params     m_internal_mesh_params;
	OceanParameters       m_ocean_parameters;
	ocean_mesh_properties m_ocean_mesh_properties;

	float m_SeaLevel;

	//Quad-tree LOD, 0 to 9 (1x1 ~ 512x512)
	int m_lods;// = 0;
	
	//Pattern lookup array
	QuadRenderParam m_mesh_patterns[9][3][3][3][3];

	//Rendering list
	vector<QuadNode> m_render_list;

	//Constant buffers
	ID3D11Buffer* m_pPerCallCB;
	ID3D11Buffer* m_pShadingCB;

	ID3D11Buffer* m_pMeshVB;
	ID3D11Buffer* m_pMeshIB;

	//Color and 1D texture
	ID3D11Texture1D* m_pFresnelMap;
	ID3D11ShaderResourceView* m_pSRVFresnel;

	//Distant Perlin Wave
	ID3D11ShaderResourceView* m_pSRVPerlin;

	//Environment Maps
	ID3D11ShaderResourceView* m_pSRVReflectCube;

	//Reflection Map
	ID3D11ShaderResourceView* m_pSRVReflectionMap;

	//Refraction Map
	ID3D11ShaderResourceView* m_pSRVRefractionMap;

	//Normal Map
	ID3D11ShaderResourceView* m_pSRVNormalMap;

	//Samplers
	ID3D11SamplerState* m_pHeightSampler;
	ID3D11SamplerState* m_pGradientSampler;
	ID3D11SamplerState* m_pFresnelSampler;
	ID3D11SamplerState* m_pPerlinSampler;
	ID3D11SamplerState* m_pCubeSampler;
};
//================================================================================================================================================================
//================================================================================================================================================================
#endif//__OCEANMESH_H