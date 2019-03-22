#pragma once

#include "JWCommon.h"
#include "JWAssimpLoader.h"
#include "JWModel.h"
#include "JWLine.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWDesignerUI
	{
	public:
		JWDesignerUI() = default;
		~JWDesignerUI();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;

		// Called in JWGame class
		void UpdateLightData(const SLightData& Ambient, const SLightData& Directional, const VECTOR<SLightData>& LightsList) noexcept;

		// Called in JWGame class
		void Draw() noexcept;

		// Called in JWGame class
		void DrawMiniAxis() noexcept;

	private:
		void MakeGrid(float XSize, float ZSize, float GridInterval = 1.0f) noexcept;

		void MakeMiniAxis() noexcept;

		auto AddVertex(const SVertexColor& Vertex) noexcept->JWDesignerUI&;
		auto AddIndex(const SIndex2& Index) noexcept->JWDesignerUI&;
		void AddEnd() noexcept;

		void LoadLightModel() noexcept;
		
		void Update() noexcept;

		void DrawLightModel() noexcept;

	private:
		static constexpr float KAxisLength = 1000.0f;
		static constexpr const char* KLightModelFileName{ "lightbulb.obj" };
		static constexpr XMFLOAT4 KXAxisColor = XMFLOAT4(1, 0, 0, 1); // X = R
		static constexpr XMFLOAT4 KYAxisColor = XMFLOAT4(0, 1, 0, 1); // Y = G
		static constexpr XMFLOAT4 KZAxisColor = XMFLOAT4(0, 0, 1, 1); // Z = B
		bool m_IsValid{ false };
		bool m_IsLightDataUpdated{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		STRING m_BaseDirectory{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};
		SVertexColorData m_VertexData{};
		SIndex2Data m_IndexData{};
		
		SColorVSCBData m_ColorVSConstantBufferData{};

		const SLightData* m_pAmbientLightData{};
		const SLightData* m_pDirectionalLightData{};
		const VECTOR<SLightData>* m_pLightsData{};
		JWModel m_LightModel;

		JWLine m_MiniAxis;
	};
};
