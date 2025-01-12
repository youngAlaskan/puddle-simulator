#pragma once

#include "../scene/terrain.h"
#include <FastNoise/FastNoise.h>

class TerrainGenerator {
public:
	TerrainGenerator() = default;

	void SetResX(uint32_t resX) { m_ResX = resX; }
	uint32_t GetResX() const { return m_ResX; }

	void SetResZ(uint32_t resZ) { m_ResZ = resZ; }
	uint32_t GetResZ() const { return m_ResZ; }

	void SetLength(float length) { m_Length = length; }
	float GetLength() const { return m_Length; }

	void SetWidth(float width) { m_Width = width; }
	float GetWidth() const { return m_Width; }

	void SetBaseElevation(float baseElevation) { m_BaseElevation = baseElevation; }
	float GetBaseElevation() const { return m_BaseElevation; }

	void SetSeed(int32_t seed) { m_Seed = seed; }
	int32_t GetSeed() const { return m_Seed; }

	void SetFreq(float freq) { m_Freq = freq; }
	float GetFreq() const { return m_Freq; }

	void SetHeightMul(float heightMul) { m_HeightMul = heightMul; }
	float GetHeightMul() const { return m_HeightMul; }

	std::vector<Vertex> GenerateVertices(FastNoise::SmartNode<FastNoise::Generator> generator);

private:
	uint32_t m_ResX = 1000U;
	uint32_t m_ResZ = 1000U;
	float m_Width = 100.0f;
	float m_Length = 100.0f;
	float m_BaseElevation = -10.0f;
	int32_t m_Seed = 1338U;
	float m_Freq = 0.005f;
	float m_HeightMul = 5.0f;
};