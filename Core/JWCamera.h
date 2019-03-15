#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	class JWCamera
	{
	public:
		JWCamera() = default;
		~JWCamera() = default;

		// Called in JWGame class
		void Create(JWDX& DX) noexcept;

		void SetPosition(XMFLOAT4 Position) noexcept;
		void SetLookAt(XMFLOAT4 LookAt) noexcept;

		auto GetViewProjectionMatrix() const noexcept->XMMATRIX;

	private:
		JWDX* m_pDX{};

		mutable XMMATRIX m_MatrixView{};
		mutable XMMATRIX m_MatrixProjection{};

		XMVECTOR m_CameraUp{};
		XMVECTOR m_CameraPosition{};
		XMVECTOR m_CameraLookAt{};
	};
};