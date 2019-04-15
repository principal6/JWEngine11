#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWDX;

	enum class ECameraType
	{
		Invalid,

		FirstPerson,
		ThirdPerson,
		FreeLook,
		Camera2D,
	};
	
	enum class ECameraMoveDirection
	{
		Left,
		Right,
		Forward,
		Backward,
		Up2D,
		Down2D,
	};

	class JWCamera
	{
	public:
		JWCamera() = default;
		~JWCamera() = default;

		// Called in JWGame class
		void Create(JWDX& DX) noexcept;
		void Destroy() noexcept {};

		void MoveCamera(ECameraMoveDirection Direction, float Stride = 1.0f) noexcept;
		void RotateCamera(float PitchStride, float YawStride, float RollStride) noexcept;

		// Only used for third-person camera
		void ZoomCamera(float Factor) noexcept;

		auto SetCameraType(ECameraType Type) noexcept->JWCamera&;
		auto SetPosition(XMFLOAT3 Position) noexcept->JWCamera&;
		auto SetLookAt(XMFLOAT3 LookAt) noexcept->JWCamera&;

		auto GetPosition() const noexcept->const XMVECTOR&;
		auto GetPositionFloat4() const noexcept->const XMFLOAT4&;
		auto GetPositionFloat3() const noexcept->const XMFLOAT3&;

		auto GetViewMatrix() const noexcept->const XMMATRIX&;
		auto GetProjectionMatrix() const noexcept->const XMMATRIX&;
		auto GetViewProjectionMatrix() const noexcept->const XMMATRIX&;
		auto GetFixedOrthographicMatrix() const noexcept->const XMMATRIX&;
		auto GetTransformedOrthographicMatrix() const noexcept->const XMMATRIX&;

	private:
		inline auto GetFirstPersonForward() noexcept->XMVECTOR;
		inline auto GetFirstPersonRight() noexcept->XMVECTOR;

		void UpdateCamera() noexcept;

		void UpdateFirstPersonOrFreeLookCamera() noexcept;
		void UpdateThirdPersonCamera() noexcept;
		void Update2DCamera() noexcept;

	private:
		bool					m_IsCreated{ false };
		JWDX*					m_pDX{};

		static constexpr float	KFOV = 0.25f;
		static constexpr float	KNearZ = 0.1f;
		static constexpr float	KFarZ = 1000.0f;
		static constexpr float	KFactor = 0.01f;
		static constexpr float	KFactor2D = 0.001f;

		mutable XMMATRIX		m_MatrixView{};
		mutable XMMATRIX		m_MatrixProjection{};
		mutable XMMATRIX		m_MatrixViewProjection{};
		mutable XMMATRIX		m_MatrixOrthographicFixed{};
		mutable XMMATRIX		m_MatrixOrthographicTransformed{};

		ECameraType	m_CameraType{ ECameraType::Invalid };

		XMVECTOR	m_CameraUp{};
		XMVECTOR	m_CameraPosition{};
		XMVECTOR	m_CameraLookAt{};

		XMVECTOR	m_CameraDefaultForward{};
		XMVECTOR	m_CameraDefaultRight{};
		XMVECTOR	m_CameraForward{};
		XMVECTOR	m_CameraRight{};

		// Y-Z rotation (nod)
		float		m_Pitch{};
		
		// X-Z rotation (turn)
		float		m_Yaw{};

		// Y-X rotation (tilt)
		float		m_Roll{};

		XMMATRIX	m_CameraRotationMatrix{};

		XMFLOAT4	m_CameraPosition4{};
		XMFLOAT3	m_CameraPosition3{};
		XMFLOAT2	m_Camera2DPosition{};
	};
};