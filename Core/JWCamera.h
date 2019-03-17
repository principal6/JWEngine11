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

		auto GetPosition() noexcept->XMVECTOR;
		auto GetViewProjectionMatrix() const noexcept->XMMATRIX;
		auto GetOrthographicMatrix() const noexcept->XMMATRIX;

	private:
		JWDX* m_pDX{};

		static constexpr float NEAR_Z = 1.0f;
		static constexpr float FAR_Z = 1000.0f;

		mutable XMMATRIX m_MatrixView{};
		mutable XMMATRIX m_MatrixProjection{};
		mutable XMMATRIX m_MatrixOrthographic{};

		XMVECTOR m_CameraUp{};
		XMVECTOR m_CameraPosition{};
		XMVECTOR m_CameraLookAt{};
	};
};