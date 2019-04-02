#pragma once

#include "JWComponentRender.h"

namespace JWEngine
{
	class JWCamera;

	class JWSystemRender
	{
		friend class JWEntity;

	public:
		JWSystemRender() {};
		~JWSystemRender();

		// Called in JWECS class
		void CreateSystem(JWDX& DX, JWCamera& Camera, STRING BaseDirectory) noexcept;

		void Update() noexcept;

	protected:
		// Called in JWEntity class
		auto CreateComponent() noexcept->JWComponentRender&;

		// Called in JWEntity class
		void DestroyComponent(JWComponentRender& Component) noexcept;

	private:
		void SetShaders(JWComponentRender& Component) noexcept;
		void Animate(JWComponentRender& Component) noexcept;

		void UpdateNodeAnimationIntoBones(bool UseInterpolation, float AnimationTime, SRiggedModelData& ModelData,
			const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;
		void UpdateNodeTPoseIntoBones(float AnimationTime, SRiggedModelData& ModelData, const SModelNode& CurrentNode, const XMMATRIX Accumulated) noexcept;

		void Draw(JWComponentRender& Component) noexcept;
		void DrawNormals(JWComponentRender& Component) noexcept;

	private:
		VECTOR<JWComponentRender*> m_vpComponents;

		JWDX* m_pDX{};
		JWCamera* m_pCamera{};
		STRING m_BaseDirectory{};

		SVSCBStatic m_VSCBStatic{};
		SVSCBRigged m_VSCBRigged{};
	};
};