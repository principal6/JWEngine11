#pragma once

#include "../Core/JWCommon.h"
#include "../Core/JWMath.h"

namespace JWEngine
{
	class JWEntity;
	class JWECS;

	static const XMVECTOR	KDefUp{ XMVectorSet(0, 1, 0, 0) };

	struct SComponentTransform
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		XMVECTOR	Position{ KVectorZero };
		XMVECTOR	ScalingFactor{ KVectorOne };

		// Rotation calculation order is always Roll -> Pitch -> Yaw
		XMFLOAT3	PitchYawRoll{ 0, 0, 0 };
		XMVECTOR	Up{ KDefUp };
		XMVECTOR	Forward{};
		XMVECTOR	Right{};

		XMMATRIX	WorldMatrix{};
		EWorldMatrixCalculationOrder	WorldMatrixCalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		inline void SetPitchYawRoll(const XMFLOAT3& _PitchYawRoll, bool IsCamera = false)
		{
			PitchYawRoll = _PitchYawRoll;

			if (IsCamera)
			{
				PitchYawRoll.x = max(PitchYawRoll.x, 0.01f);
				PitchYawRoll.x = min(PitchYawRoll.x, XM_PI - 0.01f);
			}

			auto rotation_matrix = XMMatrixRotationRollPitchYaw(PitchYawRoll.x, PitchYawRoll.y, PitchYawRoll.z);
			Forward = XMVector3TransformNormal(Up, rotation_matrix);
			Right = XMVector3Normalize(XMVector3Cross(Up, Forward));
		}

		inline void RotatePitchYawRoll(const XMFLOAT3& _PitchYawRoll, bool IsCamera = false)
		{
			PitchYawRoll.x += _PitchYawRoll.x;
			PitchYawRoll.y += _PitchYawRoll.y;
			PitchYawRoll.z += _PitchYawRoll.z;

			if (IsCamera)
			{
				PitchYawRoll.x = max(PitchYawRoll.x, 0.01f);
				PitchYawRoll.x = min(PitchYawRoll.x, XM_PI - 0.01f);
			}

			auto rotation_matrix = XMMatrixRotationRollPitchYaw(PitchYawRoll.x, PitchYawRoll.y, PitchYawRoll.z);
			Forward = XMVector3TransformNormal(Up, rotation_matrix);
			Right = XMVector3Normalize(XMVector3Cross(Up, Forward));
		}
	};

	class JWSystemTransform
	{
		friend class JWEntity;

	public:
		JWSystemTransform() = default;
		~JWSystemTransform() = default;

		void Create(JWECS& ECS) noexcept;
		void Destroy() noexcept;

		void Execute() noexcept;

	// Only accesible for JWEntity
	private:
		auto CreateComponent(JWEntity& Entity) noexcept->SComponentTransform&;
		void DestroyComponent(SComponentTransform& Component) noexcept;

	private:
		JWECS*							m_pECS{};
		VECTOR<SComponentTransform*>	m_vpComponents;
	};
};