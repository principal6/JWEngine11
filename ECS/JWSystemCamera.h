#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;
	class JWECS;
	
	const XMVECTOR	KDefUp{ XMVectorSet(0, 1, 0, 0) };

	constexpr float	KDefFOV{ XM_PIDIV4 };
	constexpr float	KDefZNear{ 0.1f };
	constexpr float	KDefZFar{ 1000.0f };
	
	constexpr float	KDefMoveFactor{ 0.01f };
	constexpr float	KDefRotateFactor{ 0.01f };
	constexpr float	KDefZoomFactor{ 0.005f };

	constexpr float	KDefZoom{ 2.0f };
	constexpr float	KDefZoomNear{ 1.0f };
	constexpr float	KDefZoomFar{ 12.0f };

	enum class ECameraType
	{
		Invalid,

		FreeLook,
		FirstPerson,
		ThirdPerson,
		Orthographic,
	};

	enum class ECameraDirection
	{
		Forward,
		Backward,
		Left,
		Right,
	};

	struct SComponentCamera
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		ECameraType	Type{ ECameraType::Invalid };
		XMVECTOR	CameraDirection{};
		XMVECTOR	LookAt{};
		XMVECTOR	Up{ KDefUp };

		float		Width{};
		float		Height{};
		float		FOV{ KDefFOV };
		float		ZNear{ KDefZNear };
		float		ZFar{ KDefZFar };

		XMMATRIX	MatrixView{ XMMatrixIdentity() };
		XMMATRIX	MatrixProjection{ XMMatrixIdentity() };

		XMVECTOR	DefaultForward{};

		float		Pitch{};
		float		Yaw{};
		float		Roll{};

		float		Zoom{ KDefZoom };
		float		ZoomNear{ KDefZoomNear };
		float		ZoomFar{ KDefZoomFar };

		float		MoveFactor{ KDefMoveFactor };
		float		RotateFactor{ KDefRotateFactor };
		float		ZoomFactor{ KDefZoomFactor };

		void CreatePerspectiveCamera(ECameraType CameraType, float ViewWidth, float ViewHeight) noexcept
		{
			Type = CameraType;

			Width = ViewWidth;
			Height = ViewHeight;

			MatrixProjection = XMMatrixPerspectiveFovLH(FOV, Width / Height, ZNear, ZFar);
		}

		void CreateOrthographicCamera(float ViewWidth, float ViewHeight) noexcept
		{
			Type = ECameraType::Orthographic;

			ZNear = KOrthographicNearZ;
			ZFar = KOrthographicFarZ;

			MatrixProjection = XMMatrixOrthographicLH(ViewWidth, ViewHeight, ZNear, ZFar);
		}

		void SetFrustum(float _FOV, float _ZNear, float _ZFar)
		{
			FOV = _FOV;
			ZNear = _ZNear;
			ZFar = _ZFar;

			MatrixProjection = XMMatrixPerspectiveFovLH(FOV, Width / Height, ZNear, ZFar);
		}
	};

	class JWSystemCamera
	{
	public:
		JWSystemCamera() = default;
		~JWSystemCamera() = default;

		void Create(JWECS& ECS, XMFLOAT2 WindowSize) noexcept;
		void Destroy() noexcept;

		auto CreateComponent() noexcept->SComponentCamera&;
		void DestroyComponent(SComponentCamera& Component) noexcept;

		void SetMainCamera(size_t CameraIndex) noexcept;
		
		// NORMALLY
		// X_Pitch is mouse's y movement
		// Y_Yaw is mouse's x movement
		// Z_Roll doesn't need be specified unless you want a special camera effect.
		void RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept;

		// Third-person
		void ZoomCurrentCamera(float Factor) noexcept;

		void MoveCurrentCamera(ECameraDirection Direction) noexcept;

		void Execute() noexcept;

		void SetCurrentCameraPosition(XMFLOAT3 Position) noexcept;
		auto GetCurrentCameraPosition() noexcept->const XMVECTOR&;

		const auto& OrthographicMatrix() noexcept { return m_MatrixProjOrthographic; }
		const auto& CurrentViewMatrix() noexcept { return m_pCurrentCamera->MatrixView; }
		const auto& CurrentProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixProjection; }
		const auto CurrentViewProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixView * m_pCurrentCamera->MatrixProjection; }

	private:
		inline void MoveFreeLook(ECameraDirection Direction) noexcept;
		inline void MoveFirstPerson(ECameraDirection Direction) noexcept;
		inline void MoveThirdPerson(ECameraDirection Direction) noexcept;
		inline void UpdateCurrentCameraViewMatrix() noexcept;

	private:
		VECTOR<SComponentCamera*>	m_vpComponents;

		SComponentCamera*			m_pCurrentCamera{};

		JWECS*						m_pECS{};
		XMMATRIX					m_MatrixProjOrthographic{};

		bool						m_AreCamerasSet{ false };
	};
};
