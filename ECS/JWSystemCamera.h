#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;
	class JWECS;
	class JWDX;
	
	constexpr float	KDefFOV{ XM_PIDIV4 };
	constexpr float	KDefZNear{ 0.1f };
	constexpr float	KDefZFar{ 1000.0f };
	
	constexpr float	KDefMoveFactor{ 0.02f };
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
	
	struct SViewFrustumVertices
	{
		XMVECTOR NLU{};
		XMVECTOR NRU{};
		XMVECTOR NRD{};
		XMVECTOR NLD{};

		XMVECTOR FLU{};
		XMVECTOR FRU{};
		XMVECTOR FRD{};
		XMVECTOR FLD{};
	};
	
	struct SComponentCamera
	{
		SComponentCamera(EntityIndexType _EntityIndex, ComponentIndexType _ComponentIndex, const SSize2* _PtrWindowSize) :
			EntityIndex{ _EntityIndex }, ComponentIndex{ _ComponentIndex }, PtrWindowSize{ _PtrWindowSize } {};

		EntityIndexType		EntityIndex{};
		ComponentIndexType	ComponentIndex{};
		const SSize2*		PtrWindowSize{};
		
		ECameraType	Type{ ECameraType::Invalid };
		XMVECTOR	LookAt{};

		float		FOV{ KDefFOV };
		float		ZNear{ KDefZNear };
		float		ZFar{ KDefZFar };

		XMMATRIX	MatrixView{ XMMatrixIdentity() };
		XMMATRIX	MatrixProjection{ XMMatrixIdentity() };

		float		Zoom{ KDefZoom };
		float		ZoomNear{ KDefZoomNear };
		float		ZoomFar{ KDefZoomFar };

		float		MoveFactor{ KDefMoveFactor };
		float		RotateFactor{ KDefRotateFactor };
		float		ZoomFactor{ KDefZoomFactor };

		void CreatePerspectiveCamera(ECameraType CameraType) noexcept
		{
			if (CameraType == ECameraType::Orthographic)
			{
				JW_ERROR_ABORT("Camera type not matching.");
			}

			Type = CameraType;

			MatrixProjection = XMMatrixPerspectiveFovLH(FOV, PtrWindowSize->floatX() / PtrWindowSize->floatY(), ZNear, ZFar);
		}

		void CreateOrthographicCamera() noexcept
		{
			Type = ECameraType::Orthographic;

			ZNear = KOrthographicNearZ;
			ZFar = KOrthographicFarZ;

			MatrixProjection = XMMatrixOrthographicLH(PtrWindowSize->floatX(), PtrWindowSize->floatY(), ZNear, ZFar);
		}

		void SetFrustum(float _FOV, float _ZNear, float _ZFar)
		{
			FOV = _FOV;
			ZNear = _ZNear;
			ZFar = _ZFar;

			MatrixProjection = XMMatrixPerspectiveFovLH(FOV, PtrWindowSize->floatX() / PtrWindowSize->floatY(), ZNear, ZFar);
		}
	};

	class JWSystemCamera
	{
		friend class JWEntity;

	public:
		JWSystemCamera() = default;
		~JWSystemCamera() = default;

		void Create(JWECS& ECS, JWDX& DX, const SSize2& WindowSize) noexcept;
		void Destroy() noexcept;

		void CaptureViewFrustum() noexcept;
		const auto& GetCapturedViewFrustum() const noexcept { return m_CapturedViewFrustumVertices; }

		void Execute() noexcept;

		void SetCurrentCamera(size_t ComponentID) noexcept;
		auto GetCurrentCamera() const noexcept { return m_pCurrentCamera; }
		auto GetCurrentCameraComponentID() const noexcept { return m_pCurrentCamera->ComponentIndex; }

		// @important:
		// This function must be called when DisplayMode has been changed.
		void UpdateCamerasProjectionMatrix() noexcept;
		
		// NORMALLY
		// X_Pitch is mouse's y movement
		// Y_Yaw is mouse's x movement
		// Z_Roll doesn't need be specified unless you want a special camera effect.
		void RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept;

		// Third-person
		void ZoomCurrentCamera(float Factor) noexcept;

		void MoveCurrentCamera(ECameraDirection Direction) noexcept;

		void SetCurrentCameraPosition(const XMFLOAT3& Position) noexcept;
		auto GetCurrentCameraPosition() const noexcept->const XMVECTOR&;

		const auto& CurrentViewMatrix() noexcept { return m_pCurrentCamera->MatrixView; }
		const auto& CurrentProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixProjection; }
		const auto CurrentViewProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixView * m_pCurrentCamera->MatrixProjection; }
	
	// Only accesible for JWEntity
	private:
		auto CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType;
		void DestroyComponent(ComponentIndexType ComponentIndex) noexcept;
		auto GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentCamera*;

	private:
		inline void MoveFreeLook(ECameraDirection Direction) noexcept;
		inline void MoveFirstPerson(ECameraDirection Direction) noexcept;
		inline void MoveThirdPerson(ECameraDirection Direction) noexcept;
		inline void UpdateCurrentCameraViewMatrix() noexcept;

		// NRU(Near right up) vertex position
		inline auto GetCurrentCameraViewFrustumNRU() const noexcept->XMVECTOR;

		// FRU(Far right up) verex position
		inline auto GetCurrentCameraViewFrustumFRU() const noexcept->XMVECTOR;

	private:
		VECTOR<SComponentCamera>	m_vComponents;
		
		JWDX*						m_pDX{};
		JWECS*						m_pECS{};
		const SSize2*				m_pWindowSize{};
		
		SComponentCamera*			m_pCurrentCamera{};

		SViewFrustumVertices		m_CapturedViewFrustumVertices{};
	};
};