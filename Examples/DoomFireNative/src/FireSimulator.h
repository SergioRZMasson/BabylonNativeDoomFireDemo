#pragma once

#include <Babylon/AppRuntime.h>
#include <vector>
#include "Random.h"


class FireCubeSimulator
{
public:
	FireCubeSimulator() = default;
	void ResizeBuffers( size_t cubeSize );
	void UpdateFire( Napi::Float32Array& fireColorData );
	int FireDecay = 1;
	int WindStrength = 1;
	int GetSizeCount() {
		return m_cubeSize;
	};

private:
	void IncreaseFireSource();
	void CalculateFirePropagation();
	void RenderFire( Napi::Float32Array& fireColorData );
	std::vector<uint32_t> m_fireIntensityVector;
	Math::RandomNumberGenerator m_generator;
	size_t m_cubeSize{0};
};