#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;

	static constexpr const char* KLightModelFileName{ "lightbulb.obj" };

	enum class ELightType
	{
		Invalid,

		AmbientLight, // Entire scene's base light
		DirectionalLight, // Sunlight
		Pointlight, // Bulb
		Spotlight, // Flashlight
	};

	struct SLightData
	{
		ELightType LightType{ ELightType::Invalid };

		XMFLOAT3 LightColor{}; // Ambient | Directional | Pointlight | Spotlight
		float Intensity{}; // Ambient | Directional | Pointlight | Spotlight

		XMFLOAT3 Position{}; // ( Ambient | Directional ) | Pointlight | Spotlight
		float Range{}; // Pointlight | Spotlight ?? Radius??

		XMFLOAT3 Direction{}; // Directional | Spotlight

		XMFLOAT3 Attenuation{}; // Pointlight | Spotlight
		float Cone{}; // Spotlight
	};

	class JWComponentLight
	{
		friend class JWSystemLight;

	public:
		// Make AMBIENT light
		void MakeAmbientLight(XMFLOAT3 _Color, float _Intensity)
		{
			LightData.LightType = ELightType::AmbientLight;
			LightData.LightColor = _Color;
			LightData.Intensity = _Intensity;
		}

		// Make DIRECTIONAL light
		// @warning: '_Position' data WILL be used to calculate the DIRECTION of directional light.
		void MakeDirectionalLight(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity)
		{
			LightData.LightType = ELightType::DirectionalLight;
			LightData.LightColor = _Color;
			LightData.Intensity = _Intensity;
			LightData.Position = _Position;

			// Calculate direction
			XMVECTOR direction = XMVectorSet(LightData.Position.x, LightData.Position.y, LightData.Position.z, 0);
			direction = XMVector3Normalize(direction);
			XMStoreFloat3(&LightData.Direction, direction);
		}

		// Make POINT light
		void MakePointLight(XMFLOAT3 _Color, XMFLOAT3 _Position, float _Intensity, float _Range, XMFLOAT3 _Attenuation)
		{
			LightData.LightType = ELightType::Pointlight;
			LightData.LightColor = _Color;
			LightData.Position = _Position;
			LightData.Intensity = _Intensity;
			LightData.Range = _Range;
			LightData.Attenuation = _Attenuation;
		}

	public:
		JWEntity* pEntity{};

		SLightData LightData;

	protected:
		// Called by System's CreateComponent()
		JWComponentLight(uint32_t ComponentID) : m_ComponentID{ ComponentID } {};
		~JWComponentLight() {};

	protected:
		uint32_t m_ComponentID{};
	};
};