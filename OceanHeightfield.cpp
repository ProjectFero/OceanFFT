#include "OceanHeightfield.h"
//================================================================================================================================================================
//================================================================================================================================================================
void OceanHeightfield::CreateHeightfield(D3DXVECTOR2* out_h0, float* out_omega)
{
	int i, j;
	D3DXVECTOR2 K, Kn;

	D3DXVECTOR2 wind_dir;
	D3DXVec2Normalize(&wind_dir, &ocean_params.wind_dir);
	float a = ocean_params.wave_amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = ocean_params.wind_speed;
	float dir_depend = ocean_params.wind_dependency;

	int height_map_dim = ocean_params.displacement_map_dim;
	float patch_length = ocean_params.patch_length;

	// initialize random generator.
	srand(0);

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = (-height_map_dim / 2.0f + i) * (2 * D3DX_PI / patch_length);

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = (-height_map_dim / 2.0f + j) * (2 * D3DX_PI / patch_length);

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].x = float(phil * GaussRandom() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].y = float(phil * GaussRandom() * HALF_SQRT_2);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}
//================================================================================================================================================================
float OceanHeightfield::GaussRandom()
{
	float v1 = rand() / (float)RAND_MAX;
	float v2 = rand() / (float)RAND_MAX;

	if (v1 < 1e-6f)
		v1 = 1e-6f;

	return sqrtf(-2 * logf(v1)) * cosf(2 * D3DX_PI * v2);
}
//================================================================================================================================================================
float OceanHeightfield::Phillips(D3DXVECTOR2 K, D3DXVECTOR2 windDir, float v, float a, float dir_depend)
{
	//Largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;

	//Damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * windDir.x + K.y * windDir.y;

	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	//Filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	//Damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}
//================================================================================================================================================================
void OceanHeightfield::Initialize()
{
	//
	// Create the heightfield and Initialize some parameters
	//

	heightfield_size = (ocean_params.displacement_map_dim + 4) * (ocean_params.displacement_map_dim + 1);
	h0_data = new D3DXVECTOR2[heightfield_size * sizeof(D3DXVECTOR2)];
	omega_data = new float[heightfield_size * sizeof(float)];

	heightfield_dim = ocean_params.displacement_map_dim;
	input_full_size = (heightfield_dim + 4) * (heightfield_dim + 1);
	input_half_size = (heightfield_dim / 2 + 1) * heightfield_dim;
	output_size = heightfield_dim * heightfield_dim;

	zero_data_size = 3 * output_size * sizeof(float) * 2;
	
	float2_stride = 2 * sizeof(float);

	CreateHeightfield(h0_data, omega_data);
}
//================================================================================================================================================================