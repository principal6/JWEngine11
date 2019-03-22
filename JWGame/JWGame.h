#pragma once

#include "../Core/JWWin32Window.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"
#include "../Core/JWAssimpLoader.h"
#include "../Core/JWModel.h"
#include "../Core/JWImage.h"
#include "../Core/JWInstantText.h"
#include "../Core/JWTimer.h"
#include "../Core/JWDesignerUI.h"
#include "../Core/JWInput.h"

namespace JWEngine
{
	class JWGame
	{
	public:
		JWGame() = default;
		~JWGame() = default;

		void Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING BaseDirectory, STRING GameFontFileName) noexcept;
		void LoadCursorImage(STRING FileName) noexcept;

		void SetOnInputFunction(FP_ON_INPUT OnInput) noexcept;
		void SetOnRenderFunction(FP_ON_RENDER OnRender) noexcept;

		void SetRasterizerState(ERasterizerState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;

		void AddOpaqueModel(STRING ModelFileName) noexcept;
		auto GetOpaqueModel(size_t OpaqueModelIndex) const noexcept->JWModel&;

		void AddTransparentModel(STRING ModelFileName) noexcept;
		auto GetTransparentModel(size_t TransparentModelIndex) const noexcept->JWModel&;

		void AddImage(STRING ImageFileName) noexcept;
		auto GetImage(size_t Image2DIndex) const noexcept->JWImage&;

		void AddLight(SLightData LightData) noexcept;


		auto GetCameraObject() noexcept->JWCamera&;
		auto GetInstantTextObject() noexcept->JWInstantText&;
		auto GetFPS() noexcept->int;

		void Run() noexcept;

		void DrawDesignerUI() noexcept;
		void DrawModels() noexcept;
		void DrawImages() noexcept;
		void DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept;

	private:
		void CheckValidity() const noexcept;

		void DrawAllOpaqueModels() const noexcept;
		void DrawAllTransparentModels() const noexcept;
		void DrawAll2DImages() const noexcept;

	private:
		bool m_IsValid{ false };
		bool m_IsWindowCreated{ false };
		bool m_IsDXCreated{ false };
		
		STRING m_BaseDirectory;
		SClearColor m_ClearColor{};

		JWWin32Window m_Window{};
		JWDX m_DX{};
		JWInput m_Input{};

		FP_ON_INPUT m_fpOnInput{};
		FP_ON_RENDER m_fpOnRender{};

		JWCamera m_Camera{};

		JWInstantText m_InstantText{};

		JWDesignerUI m_DesignerUI{};
		bool m_ShouldDrawMiniAxis{ false };

		JWTimer m_Timer{};
		long long m_FPSCount{};
		int m_FPS{};

		JWAssimpLoader m_AssimpLoader{};
		VECTOR<UNIQUE_PTR<JWModel>> m_pOpaqueModels;
		VECTOR<UNIQUE_PTR<JWModel>> m_pTransparentModels;
		VECTOR<UNIQUE_PTR<JWImage>> m_p2DImages;

		JWImage m_MouseCursorImage{};
		XMFLOAT2 m_MouseCursorPosition{};

		SDirectInputDeviceState m_InputDeviceState{};

		SLightData m_AmbientLightData{};
		SLightData m_DirectionalLightData{};
		VECTOR<SLightData> m_LightsData;
	};
};