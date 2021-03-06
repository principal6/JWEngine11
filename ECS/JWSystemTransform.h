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
		SComponentTransform(EntityIndexType _EntityIndex, ComponentIndexType _ComponentIndex) :
			EntityIndex{ _EntityIndex }, ComponentIndex{ _ComponentIndex } {};

		EntityIndexType		EntityIndex{};
		ComponentIndexType	ComponentIndex{};

		XMVECTOR	Position{ KVectorZero };
		XMVECTOR	ScalingFactor{ KVectorOne };

		// x = Pitch, y = Yaw, z = Roll
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

		inline void SetPitchYawRoll(float Pitch, float Yaw, float Roll, bool IsCamera = false)
		{
			SetPitchYawRoll(XMFLOAT3(Pitch, Yaw, Roll), IsCamera);
		}

		inline void SetPitchYawRoll(const XMVECTOR& _PitchYawRoll, bool IsCamera = false)
		{
			XMFLOAT3 xmfloat3{};
			XMStoreFloat3(&xmfloat3, _PitchYawRoll);

			SetPitchYawRoll(xmfloat3, IsCamera);
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

		inline void RotatePitchYawRoll(const XMVECTOR& _PitchYawRoll, bool IsCamera = false)
		{
			XMFLOAT3 xmfloat3{};
			XMStoreFloat3(&xmfloat3, _PitchYawRoll);

			RotatePitchYawRoll(xmfloat3, IsCamera);
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
		auto CreateComponent(EntityIndexType EntityIndex) noexcept->ComponentIndexType;
		void DestroyComponent(ComponentIndexType ComponentIndex) noexcept;
		auto GetComponentPtr(ComponentIndexType ComponentIndex) noexcept->SComponentTransform*;

	private:
		VECTOR<SComponentTransform>	m_vComponents;

		JWECS*						m_pECS{};
	};
};