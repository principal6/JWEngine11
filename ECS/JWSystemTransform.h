#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;

	struct SComponentTransform
	{
		JWEntity*	PtrEntity{};
		uint32_t	ComponentID{};

		XMFLOAT3	Position{};
		XMFLOAT3	ScalingFactor{ 1.0f, 1.0f, 1.0f };
		// Pitch/Yaw/Roll orientation
		// m_Orientation.x = Pitch = Y-Z rotation (nod)
		// m_Orientation.y = Yaw   = X-Z rotation (turn)
		// m_Orientation.z = Roll  = Y-X rotation (tilt)
		XMFLOAT3	Orientation{};
		XMMATRIX	WorldMatrix{};
		EWorldMatrixCalculationOrder	WorldMatrixCalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		inline auto SetPosition(const XMVECTOR& _Position)
		{
			XMStoreFloat3(&Position, _Position);
			return this;

		}
		inline auto SetPosition(const XMFLOAT3& _Position)
		{
			Position = _Position;
			return this;
		}

		inline auto SetScalingFactor(const XMFLOAT3& _ScalingFactor)
		{
			ScalingFactor = _ScalingFactor;
			return this;
		}

		inline auto SetOrientation(const XMFLOAT3& _Orientation)
		{
			Orientation = _Orientation;
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

		auto CreateComponent() noexcept->SComponentTransform&;
		void DestroyComponent(SComponentTransform& Component) noexcept;

		void Execute() noexcept;

	private:
		VECTOR<SComponentTransform*>	m_vpComponents;
	};
};