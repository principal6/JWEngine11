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

		auto SetPosition(XMFLOAT4 Position) noexcept->JWCamera&;
		auto SetLookAt(XMFLOAT4 LookAt) noexcept->JWCamera&;

		auto GetPosition() const noexcept->XMVECTOR;
		auto GetViewMatrix() const noexcept->XMMATRIX;
		auto GetProjectionMatrix() const noexcept->XMMATRIX;
		auto GetViewProjectionMatrix() const noexcept->XMMATRIX;
		auto GetOrthographicMatrix() const noexcept->XMMATRIX;
		auto GetViewOrthographicMatrix() const noexcept->XMMATRIX;

	private:
		bool m_IsValid{ false };

		JWDX* m_pDX{};

		static constexpr float KNearZ = 1.0f;
		static constexpr float KFarZ = 1000.0f;

		mutable XMMATRIX m_MatrixView{};
		mutable XMMATRIX m_MatrixProjection{};
		mutable XMMATRIX m_MatrixOrthographic{};

		XMVECTOR m_CameraUp{};
		XMVECTOR m_CameraPosition{};
		XMVECTOR m_CameraLookAt{};
	};
};