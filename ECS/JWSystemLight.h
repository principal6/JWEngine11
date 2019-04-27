#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWDX;
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
		ELightType	LightType{ ELightType::Invalid };
		XMFLOAT3	LightColor{}; // Ambient | Directional | Pointlight | Spotlight
		float		Intensity{}; // Ambient | Directional | Pointlight | Spotlight
		XMFLOAT3	Direction{}; // Directional | Spotlight
		float		Range{}; // Pointlight | Spotlight ?? Radius??
		XMFLOAT3	Attenuation{}; // Pointlight | Spotlight
		float		Cone{}; // Spotlight

		//XMFLOAT3	Position{}; // ( Ambient | Directional ) | Pointlight | Spotlight
		
	};

	struct SComponentLight
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		SLightData	LightData;

		// Make AMBIENT light
		void MakeAmbientLight(const XMFLOAT3& _Color, float _Intensity)
		{
			LightData.LightType = ELightType::AmbientLight;
			LightData.LightColor = _Color;
			LightData.Intensity = _Intensity;
		}

		// Make DIRECTIONAL light
		void MakeDirectionalLight(const XMFLOAT3& _Color, const XMFLOAT3& _Direction, float _Intensity)
		{
			LightData.LightType = ELightType::DirectionalLight;
			LightData.LightColor = _Color;
			LightData.Intensity = _Intensity;
			LightData.Direction = _Direction;
		}

		// Make POINT light
		void MakePointLight(const XMFLOAT3& _Color, float _Intensity, float _Range, const XMFLOAT3& _Attenuation)
		{
			LightData.LightType = ELightType::Pointlight;
			LightData.LightColor = _Color;
			LightData.Intensity = _Intensity;
			LightData.Range = _Range;
			LightData.Attenuation = _Attenuation;
		}
	};

	class JWSystemLight
	{
	public:
		JWSystemLight() = default;
		~JWSystemLight() = default;

		void Create(JWDX& DX) noexcept;
		void Destroy() noexcept;

		auto CreateComponent(JWEntity* pEntity) noexcept->SComponentLight&;
		void DestroyComponent(SComponentLight& Component) noexcept;

		void Execute() noexcept;

	private:
		VECTOR<SComponentLight*>	m_vpComponents;

		JWDX*						m_pDX{};
		bool						m_ShouldUpdate{ false };
		SPSCBLights					m_PSCBLights{};
	};
};