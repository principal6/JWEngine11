#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	enum class ECameraType
	{
		FirstPerson,
		ThirdPerson,
		FreeLook,
	};

	enum class ECameraMoveDirection
	{
		Left,
		Right,
		Forward,
		Backward
	};

	class JWCamera
	{
	public:
		JWCamera() = default;
		~JWCamera() = default;

		// Called in JWGame class
		void Create(JWDX& DX) noexcept;

		void MoveCamera(ECameraMoveDirection Direction, float Stride = 1.0f) noexcept;
		void RotateCamera(float PitchStride, float YawStride, float RollStride) noexcept;

		// Only used for third-person camera
		void ZoomCamera(float Factor) noexcept;

		auto SetCameraType(ECameraType Type) noexcept->JWCamera&;
		auto SetPosition(XMFLOAT3 Position) noexcept->JWCamera&;
		auto SetLookAt(XMFLOAT3 LookAt) noexcept->JWCamera&;

		auto GetPosition() const noexcept->XMVECTOR;
		auto GetLookAt() const noexcept->XMVECTOR;
		auto GetForward() const noexcept->XMVECTOR;
		auto GetRight() const noexcept->XMVECTOR;

		auto GetViewMatrix() const noexcept->XMMATRIX;
		auto GetProjectionMatrix() const noexcept->XMMATRIX;
		auto GetViewProjectionMatrix() const noexcept->XMMATRIX;
		auto GetOrthographicMatrix() const noexcept->XMMATRIX;
		auto GetViewOrthographicMatrix() const noexcept->XMMATRIX;

	private:
		auto GetFirstPersonForward() noexcept->XMVECTOR;
		auto GetFirstPersonRight() noexcept->XMVECTOR;

		void UpdateCamera() noexcept;
		void UpdateFirstPersonOrFreeLookCamera() noexcept;
		void UpdateThirdPersonCamera() noexcept;

	private:
		bool m_IsValid{ false };

		JWDX* m_pDX{};

		static constexpr float KNearZ = 1.0f;
		static constexpr float KFarZ = 1000.0f;
		static constexpr float KFactor = 0.01f;

		mutable XMMATRIX m_MatrixView{};
		mutable XMMATRIX m_MatrixProjection{};
		mutable XMMATRIX m_MatrixOrthographic{};

		ECameraType m_CameraType{ ECameraType::FirstPerson };

		XMVECTOR m_CameraUp{};
		XMVECTOR m_CameraPosition{};
		XMVECTOR m_CameraLookAt{};

		XMVECTOR m_CameraDefaultForward{};
		XMVECTOR m_CameraDefaultRight{};
		XMVECTOR m_CameraForward{};
		XMVECTOR m_CameraRight{};

		// Y-Z rotation (nod)
		float m_Pitch{};
		
		// X-Z rotation (turn)
		float m_Yaw{};

		// Y-X rotation (tilt)
		float m_Roll{};

		XMMATRIX m_CameraRotationMatrix{};
	};
};