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
		MinMagMipLinearWrap,
		MinMagMipPointWrap,
	};

	enum class EDepthStencilState
	{
		ZEnabled,
		ZDisabled,
	};

	enum class EVertexShader
	{
		VSBase,
		VSAnim,
		VSRaw,
		VSSkyMap,
	};

	enum class EPixelShader
	{
		PSBase,
		PSRaw,
		PSSkyMap,
	};

	class JWDX
	{
	public:
		JWDX() = default;
		~JWDX();

		void Create(const JWWin32Window& Window, STRING Directory, const SClearColor& ClearColor) noexcept;

		// Factory functions
		void CreateDynamicVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateStaticVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateIndexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;

		// Rasterizer state
		auto GetRasterizerState() const noexcept { return m_eRasterizerState; }
		auto GetPreviousRasterizerState() const noexcept { return m_ePreviousRasterizerState; }
		void SetRasterizerState(ERasterizerState State) noexcept;
		void SwitchRasterizerState() noexcept;

		void SetDepthStencilState(EDepthStencilState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;
		void SetPSSamplerState(ESamplerState State) noexcept;

		void SetVS(EVertexShader VS) noexcept;
		void SetPS(EPixelShader PS) noexcept;

		// Called in each JWModel(/JWImage/JWLineModel)'s Draw()-Update() function
		void UpdateVSCBSpace(const SVSCBSpace& Data) noexcept;
		// Called in each animated JWModel's Animate() function
		void UpdateVSCBFlags(const SVSCBFlags& Data) noexcept;
		void UpdateVSCBCPUAnimation(const SVSCBCPUAnimation& Data) noexcept;
		void UpdateVSCBGPUAnimation(const SVSCBGPUAnimation& Data) noexcept;
		// Called in each JWModel(/JWImage/JWLineModel)'s Draw()-Update() function
		void UpdatePSCBFlags(bool HasTexture, bool UseLighting) noexcept;
		void UpdatePSCBLights(const SPSCBLights& Data) noexcept;
		// Called once per game loop, which is when the camera's position would probably be changed.
		void UpdatePSCBCamera(const XMFLOAT4& CameraPosition) noexcept;

		void BeginDrawing() noexcept;
		void EndDrawing() noexcept;

		auto GetDevice() const noexcept->ID3D11Device*;
		auto GetDeviceContext() const noexcept->ID3D11DeviceContext*;
		auto GetWindowSize() const noexcept->SSizeInt;

	private:
		// Called in Create()
		void CreateDeviceAndSwapChain(HWND hWnd) noexcept;

		// VS Shader & input layout creation
		// Called in Create()
		void CreateVSBase() noexcept;
		void CreateVSAnim() noexcept;
		void CreateVSRaw() noexcept;
		void CreateVSSkyMap() noexcept;
		void CreateVSCBs() noexcept;

		// PS Shader creation
		void CreatePSBase() noexcept;
		void CreatePSRaw() noexcept;
		void CreatePSSkyMap() noexcept;
		void CreatePSCBs() noexcept;
		
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
		bool		m_IsCreated{ false };

		STRING		m_BaseDirectory;
		SSizeInt	m_WindowSize{};
		FLOAT		m_ClearColor[4]{};

		IDXGISwapChain*			m_SwapChain{};
		ID3D11Device*			m_Device11{};
		ID3D11DeviceContext*	m_DeviceContext11{};

		// Shader and input layout
		ID3D11InputLayout*	m_IAInputLayoutBase{};
		ID3D11InputLayout*	m_IAInputLayoutAnim{};
		ID3D10Blob*			m_VSBaseBuffer{};
		ID3D11VertexShader*	m_VSBase{};
		ID3D10Blob*			m_VSAnimBuffer{};
		ID3D11VertexShader*	m_VSAnim{};
		ID3D10Blob*			m_VSRawBuffer{};
		ID3D11VertexShader*	m_VSRaw{};
		ID3D10Blob*			m_VSSkyMapBuffer{};
		ID3D11VertexShader*	m_VSSkyMap{};
		ID3D10Blob*			m_PSBaseBuffer{};
		ID3D11PixelShader*	m_PSBase{};
		ID3D10Blob*			m_PSRawBuffer{};
		ID3D11PixelShader*	m_PSRaw{};
		ID3D10Blob*			m_PSSkyMapBuffer{};
		ID3D11PixelShader*	m_PSSkyMap{};

		// Shader constant buffer
		ID3D11Buffer*		m_VSCBSpace{};
		ID3D11Buffer*		m_VSCBFlags{};
		ID3D11Buffer*		m_VSCBCPUAnimation{};
		ID3D11Buffer*		m_VSCBGPUAnimation{};
		ID3D11Buffer*		m_PSCBFlags{};
		SPSCBFlags			m_PSCBFlagsData{};
		ID3D11Buffer*		m_PSCBLights{};
		ID3D11Buffer*		m_PSCBCamera{};
		SPSCBCamera			m_PSCBCameraData{};

		ID3D11DepthStencilView*		m_DepthStencilView11{};
		ID3D11DepthStencilState*	m_DepthStencilStateZEnabled11{};
		ID3D11DepthStencilState*	m_DepthStencilStateZDisabled11{};

		ID3D11RenderTargetView*		m_RenderTargetView11{};

		ERasterizerState			m_eRasterizerState{};
		ERasterizerState			m_ePreviousRasterizerState{};
		ID3D11RasterizerState*		m_RasterizerStateWireFrame11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidNoCull11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidBackCullCCW11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidBackCullCW11{};

		ID3D11BlendState*			m_BlendStateTransparent{};
		ID3D11BlendState*			m_BlendStateOpaque{};
		
		ID3D11SamplerState*			m_SamplerStateMinMagMipLinearWrap{};
		ID3D11SamplerState*			m_SamplerStateMinMagMipPointWrap{};

		D3D11_VIEWPORT				m_DefaultViewPort{};
	};
};