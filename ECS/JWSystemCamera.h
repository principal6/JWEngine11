#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;
	class JWECS;
	
	const XMVECTOR	KDefPosition{ XMVectorSet(0, 0, 0, 0) };
	const XMVECTOR	KDefLookAt{ XMVectorSet(0, 0, 1, 0) };
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
		XMVECTOR	Position{ KDefPosition };
		XMVECTOR	LookAt{ KDefLookAt };
		XMVECTOR	Up{ KDefUp };

		float		Width{};
		float		Height{};
		float		FOV{ KDefFOV };
		float		ZNear{ KDefZNear };
		float		ZFar{ KDefZFar };

		XMMATRIX	MatrixView{ XMMatrixIdentity() };
		XMMATRIX	MatrixProjection{ XMMatrixIdentity() };

		XMVECTOR	Forward{};
		XMVECTOR	DefaultForward{};
		XMVECTOR	Right{};

		float		Pitch{};
		float		Yaw{};
		float		Roll{};

		float		Zoom{ KDefZoom };
		float		ZoomNear{ KDefZoomNear };
		float		ZoomFar{ KDefZoomFar };

		float		MoveFactor{ KDefMoveFactor };
		float		RotateFactor{ KDefRotateFactor };
		float		ZoomFactor{ KDefZoomFactor };

		void CreatePerspectiveCamera(ECameraType CameraType, float ViewWidth, float ViewHeight, XMFLOAT3 _Position, XMFLOAT3 _Direction) noexcept
		{
			Type = CameraType;

			Width = ViewWidth;
			Height = ViewHeight;

			// @important
			if (Type == ECameraType::ThirdPerson)
			{
				LookAt = XMVectorSet(_Position.x, _Position.y, _Position.z, 0);
				DefaultForward = XMVector3Normalize(XMVectorSet(_Direction.x, _Direction.y, _Direction.z, 0));
				Position = LookAt - DefaultForward;
			}
			else
			{
				Position = XMVectorSet(_Position.x, _Position.y, _Position.z, 0);
				DefaultForward = XMVector3Normalize(XMVectorSet(_Direction.x, _Direction.y, _Direction.z, 0));
				LookAt = Position + DefaultForward;
			}

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

		void SetMainCamera(size_t CameraIndex) noexcept
		{
			if (m_vpComponents.size() == 0)
			{
				JW_ERROR_ABORT("You didn't create any camera.");
			}

			CameraIndex = max(CameraIndex, m_vpComponents.size() - 1);

			m_pCurrentCamera = m_vpComponents[CameraIndex];
		}

		void SetCurrentCameraPosition(XMFLOAT3 Position) noexcept
		{
			UpdateCurrentCameraVectors();

			const auto& type = m_pCurrentCamera->Type;

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;

			const auto& forward = m_pCurrentCamera->Forward;

			if (type == ECameraType::ThirdPerson)
			{
				// @important

				lookat = XMVectorSet(Position.x, Position.y, Position.z, 0);

				const auto& zoom = m_pCurrentCamera->Zoom;
				position = lookat - forward * zoom;
			}
			else
			{
				position = XMVectorSet(Position.x, Position.y, Position.z, 0);
				lookat = position + forward;
			}

			UpdateCurrentCameraViewMatrix();
		}
		
		// NORMALLY
		// X_Pitch is mouse's y movement
		// Y_Yaw is mouse's x movement
		// Z_Roll doesn't need be specified unless you want a special camera effect.
		void RotateCurrentCamera(float X_Pitch, float Y_Yaw, float Z_Roll) noexcept
		{
			UpdateCurrentCameraVectors();

			const auto& type = m_pCurrentCamera->Type;
			
			const auto& rotate_factor = m_pCurrentCamera->RotateFactor;
			auto& pitch = m_pCurrentCamera->Pitch;
			auto& yaw = m_pCurrentCamera->Yaw;
			auto& roll = m_pCurrentCamera->Roll;

			pitch += X_Pitch * rotate_factor;
			yaw += Y_Yaw * rotate_factor;
			roll += Z_Roll * rotate_factor;

			// This is needed to eliminate Y-axis flipping
			pitch = min(pitch, XM_PIDIV2 - 0.01f);
			pitch = max(pitch, -XM_PIDIV2 + 0.01f);

			XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;

			const auto& default_forward = m_pCurrentCamera->DefaultForward;
			auto& forward = m_pCurrentCamera->Forward;

			forward = XMVector3TransformNormal(default_forward, rotation);

			if (type == ECameraType::ThirdPerson)
			{
				const auto& zoom = m_pCurrentCamera->Zoom;
				position = lookat - forward * zoom;
			}
			else
			{
				lookat = position + forward;
			}

			UpdateCurrentCameraViewMatrix();
		}

		// Third-person
		void ZoomCurrentCamera(float Factor) noexcept
		{
			const auto& type = m_pCurrentCamera->Type;
			if (type != ECameraType::ThirdPerson)
			{
				return;
			}

			const auto& zoom_factor = m_pCurrentCamera->ZoomFactor;
			const auto& zoom_near = m_pCurrentCamera->ZoomNear;
			const auto& zoom_far = m_pCurrentCamera->ZoomFar;

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;
			
			const auto& forward = m_pCurrentCamera->Forward;

			auto& zoom = m_pCurrentCamera->Zoom;
			
			zoom += -Factor * zoom_factor;
			zoom = max(zoom, zoom_near);
			zoom = min(zoom, zoom_far);

			position = lookat - forward * zoom;

			UpdateCurrentCameraViewMatrix();
		}

		void MoveCurrentCamera(ECameraDirection Direction) noexcept
		{
			UpdateCurrentCameraVectors();

			const auto& type = m_pCurrentCamera->Type;
			switch (type)
			{
			case JWEngine::ECameraType::FreeLook:
				MoveFreeLook(Direction);
				break;
			case JWEngine::ECameraType::FirstPerson:
				MoveFirstPerson(Direction);
				break;
			case JWEngine::ECameraType::ThirdPerson:
				MoveThirdPerson(Direction);
				break;
			default:
				break;
			}

			UpdateCurrentCameraViewMatrix();
		}

		void Execute() noexcept {};

		const auto& OrthographicMatrix() noexcept { return m_MatrixProjOrthographic; }
		const auto& CurrentCameraPosition() noexcept { return m_pCurrentCamera->Position; }
		const auto& CurrentViewMatrix() noexcept { return m_pCurrentCamera->MatrixView; }
		const auto& CurrentProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixProjection; }
		const auto CurrentViewProjectionMatrix() noexcept { return m_pCurrentCamera->MatrixView * m_pCurrentCamera->MatrixProjection; }

	private:
		inline void MoveFreeLook(ECameraDirection Direction) noexcept
		{
			const auto& move_factor = m_pCurrentCamera->MoveFactor;

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;
			
			const auto& forward = m_pCurrentCamera->Forward;
			const auto& right = m_pCurrentCamera->Right;

			switch (Direction)
			{
			case JWEngine::ECameraDirection::Forward:
				position += forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Backward:
				position -= forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Left:
				position -= right * move_factor;
				break;
			case JWEngine::ECameraDirection::Right:
				position += right * move_factor;
				break;
			default:
				break;
			}

			lookat = position + forward;
		}

		inline void MoveFirstPerson(ECameraDirection Direction) noexcept
		{
			const auto& move_factor = m_pCurrentCamera->MoveFactor;

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;

			const auto& forward = m_pCurrentCamera->Forward;
			const auto& right = m_pCurrentCamera->Right;

			auto temp_forward = XMVectorSetY(forward, 0);
			auto temp_right = XMVectorSetY(right, 0);

			switch (Direction)
			{
			case JWEngine::ECameraDirection::Forward:
				position += temp_forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Backward:
				position -= temp_forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Left:
				position -= temp_right * move_factor;
				break;
			case JWEngine::ECameraDirection::Right:
				position += temp_right * move_factor;
				break;
			default:
				break;
			}

			lookat = position + forward;
		}

		inline void MoveThirdPerson(ECameraDirection Direction) noexcept
		{
			const auto& move_factor = m_pCurrentCamera->MoveFactor;

			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;

			const auto& forward = m_pCurrentCamera->Forward;
			const auto& right = m_pCurrentCamera->Right;

			auto temp_forward =  XMVectorSetY(forward, 0);
			auto temp_right = XMVectorSetY(right, 0);

			switch (Direction)
			{
			case JWEngine::ECameraDirection::Forward:
				lookat += temp_forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Backward:
				lookat -= temp_forward * move_factor;
				break;
			case JWEngine::ECameraDirection::Left:
				lookat -= temp_right * move_factor;
				break;
			case JWEngine::ECameraDirection::Right:
				lookat += temp_right * move_factor;
				break;
			default:
				break;
			}

			auto& zoom = m_pCurrentCamera->Zoom;
			position = lookat - forward * zoom;
		}

		inline void UpdateCurrentCameraVectors() noexcept
		{
			auto& forward = m_pCurrentCamera->Forward;
			auto& right = m_pCurrentCamera->Right;
			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;
			auto& up = m_pCurrentCamera->Up;

			forward = XMVector3Normalize(lookat - position);
			right = XMVector3Normalize(XMVector3Cross(up, forward));
		}

		inline void UpdateCurrentCameraViewMatrix() noexcept
		{
			auto& matrix_view = m_pCurrentCamera->MatrixView;
			auto& position = m_pCurrentCamera->Position;
			auto& lookat = m_pCurrentCamera->LookAt;
			auto& up = m_pCurrentCamera->Up;

			matrix_view = XMMatrixLookAtLH(position, lookat, up);
		}

	private:
		VECTOR<SComponentCamera*>	m_vpComponents;

		SComponentCamera*			m_pCurrentCamera{};

		JWECS*						m_pECS{};
		XMMATRIX					m_MatrixProjOrthographic{};
	};
};
