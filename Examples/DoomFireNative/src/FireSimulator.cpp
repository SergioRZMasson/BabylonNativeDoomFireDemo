#include "FireSimulator.h"

struct Color
{
	Color( float r, float g, float b ) : r{ r }, g{ g }, b{ b }, a{ 1.0f } {};
	float r;
	float g;
	float b;
	float a;
};

const std::vector<Color> g_fireColorsPalette = { { 7.0f, 7.0f,  7.0f }, {  31.0f, 7.0f,  7.0f }, {  47.0f, 15.0f,  7.0f }, {  71.0f, 15.0f,  7.0f }, {  87.0f, 23.0f,  7.0f }, {  103.0f, 31.0f,  7.0f }, {  119.0f, 31.0f,  7.0f }, {  143.0f, 39.0f,  7.0f }, {  159.0f, 47.0f,  7.0f }, {  175.0f, 63.0f,  7.0f }, {  191.0f, 71.0f,  7.0f }, {  199.0f, 71.0f,  7.0f }, {  223.0f, 79.0f,  7.0f }, {  223.0f, 87.0f,  7.0f }, {  223.0f, 87.0f,  7.0f }, {  215.0f, 95.0f,  7.0f }, {  215.0f, 95.0f,  7.0f }, {  215.0f, 103.0f,  15.0f }, {  207.0f, 111.0f,  15.0f }, {  207.0f, 119.0f,  15.0f }, {  207.0f, 127.0f,  15.0f }, {  207.0f, 135.0f,  23.0f }, {  199.0f, 135.0f,  23.0f }, {  199.0f, 143.0f,  23.0f }, {  199.0f, 151.0f,  31.0f }, {  191.0f, 159.0f,  31.0f }, {  191.0f, 159.0f,  31.0f }, {  191.0f, 167.0f,  39.0f }, {  191.0f, 167.0f,  39.0f }, {  191.0f, 175.0f,  47.0f }, {  183.0f, 175.0f,  47.0f }, {  183.0f, 183.0f,  47.0f }, {  183.0f, 183.0f,  55.0f }, {  207.0f, 207.0f,  111.0f }, {  223.0f, 223.0f,  159.0f }, {  239.0f, 239.0f,  199.0f }, {  255.0f, 255.0f,  255.0f } };


void FireCubeSimulator::ResizeBuffers( size_t cubeSize )
{
	if( m_cubeSize == cubeSize )
	{
		return;
	}
	m_cubeSize = cubeSize;
	m_fireIntensityVector.resize( m_cubeSize * m_cubeSize * m_cubeSize );
}

void FireCubeSimulator::UpdateFire( Napi::Float32Array& fireColorData )
{
	IncreaseFireSource();
	CalculateFirePropagation();
	RenderFire( fireColorData );
}

void FireCubeSimulator::IncreaseFireSource()
{
	size_t lastInstance = (m_cubeSize * m_cubeSize * m_cubeSize) - 1;
	size_t sideSquare = m_cubeSize * m_cubeSize;

	for( int i = 0; i < sideSquare; i++ )
	{
		size_t instanceIndex = (lastInstance - i);
		int32_t currentFireIntensity = m_fireIntensityVector[instanceIndex];
		int32_t increase = m_generator.NextInt( 0, 7 );
		m_fireIntensityVector[instanceIndex] = std::min( 36, currentFireIntensity + increase );
	}
}

void FireCubeSimulator::CalculateFirePropagation()
{
	auto instanceCount = m_cubeSize * m_cubeSize * m_cubeSize;
	auto sideSquare = m_cubeSize * m_cubeSize;

	// For every instance except the floor.
	for( auto currentPixelIndex = 1; currentPixelIndex < instanceCount - sideSquare; currentPixelIndex++ )
	{
		// Get intensity of the instance below
		auto belowPixelIndex = currentPixelIndex + (sideSquare);
		int32_t belowPixelFireIntensity = m_fireIntensityVector[belowPixelIndex];

		// Calculate new intensity.
		int32_t decay = m_generator.NextInt( 0, FireDecay);
		int32_t newFireIntensity = std::max( belowPixelFireIntensity - decay, 0 );
		
		// Calculate wind contribution
		int32_t direction = m_generator.NextInt( 0, WindStrength ) - 1;
		direction = std::max( direction, 1 );
		int32_t decayDirection = decay * direction;
		
		 // Set new intensity value.
		m_fireIntensityVector[currentPixelIndex - decayDirection] = newFireIntensity;
	}
}

void FireCubeSimulator::RenderFire( Napi::Float32Array& fireColorData )
{
	auto instanceCount = m_cubeSize * m_cubeSize * m_cubeSize;

	for( size_t instanceID = 0; instanceID < instanceCount; instanceID++ )
	{
		uint32_t fireIntensity = m_fireIntensityVector[instanceID];
		const Color& fireColor = g_fireColorsPalette[fireIntensity];

		fireColorData[instanceID * 4 + 0] = fireColor.r / 255.0f;
		fireColorData[instanceID * 4 + 1] = fireColor.g / 255.0f;
		fireColorData[instanceID * 4 + 2] = fireColor.b / 255.0f;
		fireColorData[instanceID * 4 + 3] = fireColor.a / 255.0f;
	}
}
