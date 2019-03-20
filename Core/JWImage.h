#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;
	class JWCamera;

	class JWImage
	{
	public:
		JWImage() = default;
		virtual ~JWImage();

		// Called in JWGame class
		virtual void Create(JWDX& DX, JWCamera& Camera) noexcept;

		// Called in JWGame class
		virtual void LoadImageFromFile(STRING Directory, STRING FileName) noexcept;

		virtual auto SetPosition(XMFLOAT2 Position) noexcept->JWImage&;
		virtual auto SetSize(XMFLOAT2 Size) noexcept->JWImage&;
		
		// Called in JWGame class
		virtual void UpdateAll() noexcept;

		// Called in JWGame class
		virtual void Draw() noexcept;

	protected:
		virtual void CheckValidity() const noexcept;

		// Buffer creation
		virtual void AddVertex(const SVertex& Vertex) noexcept;
		virtual void AddIndex(const SIndex3& Index) noexcept;
		virtual void AddEnd() noexcept;

		// Texture creation
		virtual void CreateTexture(WSTRING TextureFileName) noexcept;

		// Called by SetPosition() or SetSize()
		virtual void UpdateScreenPositionAndSize() noexcept;

		// Called by UpdateVertices()
		virtual void UpdateVertexBuffer() noexcept;

		// Called by UpdateAll()
		virtual void UpdateDefaultVSConstantBuffer() noexcept;
		virtual void UpdateDefaultPSConstantBuffer() noexcept;
		virtual void UpdateTexture() noexcept;

	protected:
		bool m_IsValid{ false };
		bool m_IsTextureCreated{ false };

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};

		ID3D11Buffer* m_VertexBuffer{};
		ID3D11Buffer* m_IndexBuffer{};

		SVertexData m_VertexData{};
		SIndexData m_IndexData{};

		ID3D11ShaderResourceView* m_TextureShaderResourceView{};

		XMFLOAT2 m_Position{};
		XMFLOAT2 m_Size{};
		SSizeInt m_ImageOriginalSize{};

		XMMATRIX m_WVP{};
	};
};
