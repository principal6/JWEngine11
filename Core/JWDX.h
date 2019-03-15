#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWWin32Window;

	enum class ERasterizerState
	{
		WireFrame,
		SolidNoCull,
		SolidBackCullCCW,
		SolidBackCullCW,
	};

	enum class EBlendState
	{
		Transprent,
		Opaque,
	};

	class JWDX
	{
	public:
		JWDX() = default;
		~JWDX();

		void Create(const JWWin32Window& Window) noexcept;

		void SetRasterizerState(ERasterizerState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;
		void SetConstantBufferData(SConstantBufferDataPerObject Data) noexcept;

		void BeginDrawing(const SClearColor& ClearColor) noexcept;
		void EndDrawing() noexcept;

		auto GetDevice() const noexcept->ID3D11Device*;
		auto GetDeviceContext() const noexcept->ID3D11DeviceContext*;
		auto GetWindowSize() const noexcept->SSizeInt;

	private:
		// Called in Create()
		void CreateDeviceAndSwapChain(HWND hWnd) noexcept;

		// Called in Create()
		void CreateShaders() noexcept;

		// Called in Create()
		void CreateViewport() noexcept;
		
		// Called in Create()
		void CreateInputLayout() noexcept;

		// Called in Create()
		void CreateDepthStencil() noexcept;

		// Called in Create()
		void CreateRenderTarget() noexcept;

		// Called in Create()
		void CreateRasterizerStates() noexcept;

		// Called in Create()
		void CreateBlendStates() noexcept;

		// Called in Create()
		void CreateConstantBuffer() noexcept;

	private:
		SSizeInt m_WindowSize{};

		IDXGISwapChain* m_SwapChain{};

		ID3D11Device* m_Device11{};
		ID3D11DeviceContext* m_DeviceContext11{};

		ID3D10Blob* m_VertexShaderBuffer{};
		ID3D10Blob* m_PixelShaderBuffer{};
		
		ID3D11VertexShader* m_VertexShader11{};
		ID3D11PixelShader* m_PixelShader11{};

		ID3D11InputLayout* m_InputLayout11{};

		ID3D11DepthStencilView* m_DepthStencilView11{};

		ID3D11RenderTargetView* m_RenderTargetView11{};

		ID3D11RasterizerState* m_RasterizerStateWireFrame11{};
		ID3D11RasterizerState* m_RasterizerStateSolidNoCull11{};
		ID3D11RasterizerState* m_RasterizerStateSolidBackCullCCW11{};
		ID3D11RasterizerState* m_RasterizerStateSolidBackCullCW11{};

		ID3D11BlendState* m_BlendStateTransparent{};
		ID3D11BlendState* m_BlendStateOpaque{};

		ID3D11Buffer* m_ConstantBufferPerObject{};
		SConstantBufferDataPerObject m_ConstantBufferDataPerObject;
	};
};