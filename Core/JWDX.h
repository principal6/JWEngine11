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

	enum class ESamplerState
	{
		LinearWrap,
	};

	enum class EDepthStencilState
	{
		ZEnabled,
		ZDisabled,
	};

	class JWDX
	{
	public:
		JWDX() = default;
		~JWDX();

		void Create(const JWWin32Window& Window, STRING Directory) noexcept;

		// Factory functions
		void CreateDynamicVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateStaticVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateIndexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;

		void SetDepthStencilState(EDepthStencilState State) noexcept;
		void SetRasterizerState(ERasterizerState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;
		void SetPSSamplerState(ESamplerState State) noexcept;

		void SetDefaultVS() noexcept;
		void SetDefaultPS() noexcept;
		void SetDefaultVSConstantBufferData(SDefaultVSConstantBufferData Data) noexcept;
		void SetDefaultPSConstantBufferData(SDefaultPSConstantBufferData Data) noexcept;

		void SetColorVS() noexcept;
		void SetColorPS() noexcept;
		void SetColorVSConstantBufferData(SColorVSConstantBufferData Data) noexcept;

		void BeginDrawing(const SClearColor& ClearColor) noexcept;
		void EndDrawing() noexcept;

		auto GetDevice() const noexcept->ID3D11Device*;
		auto GetDeviceContext() const noexcept->ID3D11DeviceContext*;
		auto GetWindowSize() const noexcept->SSizeInt;

	private:
		// Called in Create()
		void CreateDeviceAndSwapChain(HWND hWnd) noexcept;

		// Shader & input layout creation
		// Called in Create()
		void CreateDefaultVS() noexcept;
		void CreateDefaultPS() noexcept;
		void CreateDefaultVSConstantBuffer() noexcept;
		void CreateDefaultPSConstantBuffer() noexcept;
		void CreateColorVS() noexcept;
		void CreateColorPS() noexcept;
		void CreateColorVSConstantBuffer() noexcept;

		// Called in Create()
		void CreateDepthStencilView() noexcept;

		// Called in Create()
		void CreateDepthStencilStates() noexcept;

		// Called in Create()
		void CreateRenderTargetView() noexcept;

		// Called in Create()
		void CreateRasterizerStates() noexcept;

		// Called in Create()
		void CreateBlendStates() noexcept;

		// Called in Create()
		void CreateSamplerStates() noexcept;

		// Called in Create()
		void CreateDefaultViewport() noexcept;

	private:
		bool m_IsValid{ false };

		STRING m_BaseDirectory;
		SSizeInt m_WindowSize{};

		IDXGISwapChain* m_SwapChain{};
		ID3D11Device* m_Device11{};
		ID3D11DeviceContext* m_DeviceContext11{};

		ID3D10Blob* m_DefaultVSBuffer{};
		ID3D11VertexShader* m_DefaultVS11{};
		ID3D11InputLayout* m_DefaultVSInputLayout11{};
		ID3D10Blob* m_DefaultPSBuffer{};
		ID3D11PixelShader* m_DefaultPS11{};
		ID3D11Buffer* m_DefaultVSConstantBuffer{};
		SDefaultVSConstantBufferData m_DefaultVSConstantBufferData{};
		ID3D11Buffer* m_DefaultPSConstantBuffer{};
		SDefaultPSConstantBufferData m_DefaultPSConstantBufferData{};

		ID3D10Blob* m_ColorVSBuffer{};
		ID3D11VertexShader* m_ColorVS11{};
		ID3D11InputLayout* m_ColorVSInputLayout11{};
		ID3D10Blob* m_ColorPSBuffer{};
		ID3D11PixelShader* m_ColorPS11{};
		ID3D11Buffer* m_ColorVSConstantBuffer{};
		SColorVSConstantBufferData m_ColorVSConstantBufferData;

		ID3D11DepthStencilView* m_DepthStencilView11{};
		ID3D11DepthStencilState* m_DepthStencilStateZEnabled11{};
		ID3D11DepthStencilState* m_DepthStencilStateZDisabled11{};

		ID3D11RenderTargetView* m_RenderTargetView11{};

		ID3D11RasterizerState* m_RasterizerStateWireFrame11{};
		ID3D11RasterizerState* m_RasterizerStateSolidNoCull11{};
		ID3D11RasterizerState* m_RasterizerStateSolidBackCullCCW11{};
		ID3D11RasterizerState* m_RasterizerStateSolidBackCullCW11{};

		ID3D11BlendState* m_BlendStateTransparent{};
		ID3D11BlendState* m_BlendStateOpaque{};

		ID3D11SamplerState* m_SamplerStateLinearWrap{};

		D3D11_VIEWPORT m_DefaultViewPort{};
	};
};