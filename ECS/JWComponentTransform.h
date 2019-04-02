#pragma once

#include "../Core/JWCommon.h"

namespace JWEngine
{
	class JWEntity;

	class JWComponentTransform
	{
		friend class JWSystemTransform;
		friend class JWSystemRender;

	public:
		auto SetPosition(XMFLOAT3 Position)
		{
			m_Position = Position;
			return this;
		}

		auto SetScalingFactor(XMFLOAT3 ScalingFactor)
		{
			m_ScalingFactor = ScalingFactor;
			return this;
		}

		auto SetOrientation(XMFLOAT3 Orientation)
		{
			m_Orientation = Orientation;
			return this;
		}

		auto SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder Order)
		{
			m_WorldMatrixCalculationOrder = Order;
			return this;
		}

	public:
		JWEntity* pEntity{};

	protected:
		// Called by System's CreateComponent()
		JWComponentTransform(uint32_t ComponentID) : m_ComponentID{ ComponentID } {};
		~JWComponentTransform() {};

	protected:
		uint32_t m_ComponentID{};

		XMFLOAT3 m_Position{};
		XMFLOAT3 m_ScalingFactor{ 1.0f, 1.0f, 1.0f };

		// m_Orientation.x = Pitch = Y-Z rotation (nod)
		// m_Orientation.y = Yaw   = X-Z rotation (turn)
		// m_Orientation.z = Roll  = Y-X rotation (tilt)
		XMFLOAT3 m_Orientation{};

		XMMATRIX m_WorldMatrix{};

		EWorldMatrixCalculationOrder m_WorldMatrixCalculationOrder{ EWorldMatrixCalculationOrder::ScaleRotTrans };
	};
};