#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWImage2D
	{
	public:
		JWImage2D() = default;
		~JWImage2D();

		// Called in JWGame class
		void Create(JWDX& DX, JWCamera& Camera) noexcept;

		// Called in JWGame class
		void LoadImageFromFile(STRING Directory, STRING FileName) noexcept;

		auto SetPosition(XMFLOAT2 Position) noexcept->JWImage2D&;
		auto SetSize(XMFLOAT2 Size) noexcept->JWImage2D&;

		// Called in JWGame class
		void Draw() noexcept;

	private:
		void CheckValidity() const noexcept;

		// Buffer creation
		inline void AddVertex(const SVertex& Vertex) noexcept;
		inline void AddIndex(const SIndex& Index) noexcept;
		void AddEnd() noexcept;
		void CreateVertexBuffer() noexcept;
		void CreateIndexBuffer() noexcept;

		// Texture creation
		void CreateTexture(WSTRING TextureFileName) noexcept;
		void CreateSamplerState() noexcept;
		
		// Called by SetPosition() or SetSize()
		void UpdateVertices() noexcept;

		// Called by UpdateVertices()
		void UpdateVertexBuffer() noexcept;

		void Update() noexcept;

	private:
		static constexpr float DEFAULT_WIDTH = 50.0f;
		static constexpr float DEFAULT_HEIGHT = 50.0f;

		bool m_IsValid{ false };
		bool m_IsTextureCreated{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SVertexData m_VertexData{};
		SIndexData m_IndexData{};

		ID3D11ShaderResourceView* m_TextureShaderResourceView{};
		ID3D11SamplerState* m_TextureSamplerState{};

		XMFLOAT2 m_Position{};
		XMFLOAT2 m_Size{ DEFAULT_WIDTH, DEFAULT_HEIGHT };

		XMMATRIX m_WVP{};
		XMMATRIX m_MatrixWorld{};

		XMMATRIX m_MatrixTranslation{};
		XMMATRIX m_MatrixRotation{};
		XMMATRIX m_MatrixScale{};
	};
};
