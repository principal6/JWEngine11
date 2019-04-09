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
		// m_Orientation.x = Pitch = Y-Z rotation (nod)
		// m_Orientation.y = Yaw   = X-Z rotation (turn)
		// m_Orientation.z = Roll  = Y-X rotation (tilt)
		XMFLOAT3	Orientation{};
		XMMATRIX	WorldMatrix{};
		EWorldMatrixCalculationOrder	WorldMatrixCalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };

		auto SetPosition(XMFLOAT3 _Position)
		{
			Position = _Position;
			return this;
		}

		auto SetScalingFactor(XMFLOAT3 _ScalingFactor)
		{
			ScalingFactor = _ScalingFactor;
			return this;
		}

		auto SetOrientation(XMFLOAT3 _Orientation)
		{
			Orientation = _Orientation;
			return this;
		}

		auto SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder _Order)
		{
			WorldMatrixCalculationOrder = _Order;
			return this;
		}
	};

	class JWSystemTransform
	{
	public:
		JWSystemTransform() {};
		~JWSystemTransform();

		auto CreateComponent() noexcept->SComponentTransform&;
		void DestroyComponent(SComponentTransform& Component) noexcept;

		void Execute() noexcept;

	private:
		VECTOR<SComponentTransform*>	m_vpComponents;
	};
};