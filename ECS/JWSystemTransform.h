#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;

	const XMVECTOR	KDefUp{ XMVectorSet(0, 1, 0, 0) };

	struct SComponentTransform
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		XMFLOAT3	ScalingFactor{ 1.0f, 1.0f, 1.0f };

		XMVECTOR	Position{ XMVectorZero() };

		// Rotation calculation order is always Roll -> Pitch -> Yaw
		XMFLOAT3	PitchYawRoll{ 0, 0, 0 };
		XMVECTOR	Up{ KDefUp };
		XMVECTOR	Forward{};
		XMVECTOR	Right{};

		XMMATRIX	WorldMatrix{};
		EWorldMatrixCalculationOrder	WorldMatrixCalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		// This will be 'false'd in JWSystemPhysics
		bool		ShouldUpdateBoundingEllipsoid{ false };

		inline auto SetPosition(const XMVECTOR& _Position)
		{
			Position = _Position;
			ShouldUpdateBoundingEllipsoid = true;
			return this;
		}

		inline auto SetPosition(const XMFLOAT3& _Position)
		{
			Position = XMVectorSet(_Position.x, _Position.y, _Position.z, 1);
			ShouldUpdateBoundingEllipsoid = true;
			return this;
		}

		inline auto Translate(const XMFLOAT3& dPosition)
		{
			Position += XMVectorSet(dPosition.x, dPosition.y, dPosition.z, 0);
			ShouldUpdateBoundingEllipsoid = true;
			return this;
		}

		inline auto SetScalingFactor(const XMFLOAT3& _ScalingFactor)
		{
			ScalingFactor = _ScalingFactor;
			return this;
		}

		inline auto SetPitchYawRoll(float dPitch, float dYaw, float dRoll, bool IsCamera = false)
		{
			PitchYawRoll.x = dPitch;
			PitchYawRoll.y = dYaw;
			PitchYawRoll.z = dRoll;

			if (IsCamera)
			{
				PitchYawRoll.x = max(PitchYawRoll.x, 0.01f);
				PitchYawRoll.x = min(PitchYawRoll.x, XM_PI - 0.01f);
			}

			auto rotation_matrix = XMMatrixRotationRollPitchYaw(PitchYawRoll.x, PitchYawRoll.y, PitchYawRoll.z);
			Forward = XMVector3TransformNormal(Up, rotation_matrix);
			Right = XMVector3Normalize(XMVector3Cross(Up, Forward));

			return this;
		}

		inline auto RotatePitchYawRoll(float dPitch, float dYaw, float dRoll, bool IsCamera = false)
		{
			PitchYawRoll.x += dPitch;
			PitchYawRoll.y += dYaw;
			PitchYawRoll.z += dRoll;

			if (IsCamera)
			{
				PitchYawRoll.x = max(PitchYawRoll.x, 0.01f);
				PitchYawRoll.x = min(PitchYawRoll.x, XM_PI - 0.01f);
			}

			auto rotation_matrix = XMMatrixRotationRollPitchYaw(PitchYawRoll.x, PitchYawRoll.y, PitchYawRoll.z);
			Forward = XMVector3TransformNormal(Up, rotation_matrix);
			Right = XMVector3Normalize(XMVector3Cross(Up, Forward));

			return this;
		}

		inline auto SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder _Order)
		{
			WorldMatrixCalculationOrder = _Order;
			return this;
		}
	};

	class JWSystemTransform
	{
	public:
		JWSystemTransform() = default;
		~JWSystemTransform() = default;

		void Create() noexcept {};
		void Destroy() noexcept;

		auto CreateComponent(JWEntity* pEntity) noexcept->SComponentTransform&;
		void DestroyComponent(SComponentTransform& Component) noexcept;

		void Execute() noexcept;

	private:
		VECTOR<SComponentTransform*>	m_vpComponents;
	};
};