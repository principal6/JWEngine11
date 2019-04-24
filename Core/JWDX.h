#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	// Forward declaration
	class JWWin32Window;
	
	enum class EPrimitiveTopology
	{
		Invalid,
		TriangleList,
		TriangleStrip,
		LineList,
	};

	enum class ERasterizerState
	{
		WireFrame,
		SolidNoCull,
		SolidBackCullCCW,
		SolidBackCullCW,
	};

	enum class EBlendState
	{
		Invalid,
		Transprent,
		Opaque,
	};

	enum class ESamplerState
	{
		Invalid,
		MinMagMipLinearWrap,
		MinMagMipPointWrap,
	};

	enum class EDepthStencilState
	{
		Invalid,
		ZEnabled,
		ZDisabled,
	};

	enum class EVertexShader
	{
		Invalid,
		VSBase,
		VSRaw,
		VSSkyMap,
		VSIntantText,
	};

	enum class EPixelShader
	{
		Invalid,
		PSBase,
		PSRaw,
		PSSkyMap,
		PSIntantText,
	};

	class JWDX
	{
	public:
		JWDX() = default;
		~JWDX() = default;

		void Create(HWND hWnd, XMFLOAT2 WindowSize, STRING Directory, const SClearColor& ClearColor) noexcept;
		void Destroy() noexcept;

		/// Factory functions
		void CreateDynamicVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateStaticVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateIndexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		
		inline void UpdateDynamicResource(ID3D11Resource* pResource, const void* pData, size_t Size) noexcept;

		/// Rasterizer state
		auto GetRasterizerState() const noexcept { return m_CurrentRasterizerState; }
		auto GetPreviousRasterizerState() const noexcept { return m_PreviousRasterizerState; }
		void SetRasterizerState(ERasterizerState State) noexcept;
		void SwitchRasterizerState() noexcept;

		void SetPrimitiveTopology(EPrimitiveTopology Topology) noexcept;
		void SetDepthStencilState(EDepthStencilState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;
		void SetPSSamplerState(ESamplerState State) noexcept;

		void SetVS(EVertexShader VS) noexcept;
		void SetPS(EPixelShader PS) noexcept;

		/// Update VS constant buffers
		void UpdateVSCBSpace(const SVSCBSpace& Data) noexcept;
		void UpdateVSCBFlags(const SVSCBFlags& Data) noexcept;
		void UpdateVSCBCPUAnimationData(const SVSCBCPUAnimationData& Data) noexcept;
		void UpdateVSCBGPUAnimationData(const SVSCBGPUAnimationData& Data) noexcept;

		/// Update PS constant buffers
		void UpdatePSCBFlags(bool HasTexture, bool UseLighting) noexcept;
		void UpdatePSCBLights(const SPSCBLights& Data) noexcept;
		void UpdatePSCBCamera(const XMFLOAT4& CameraPosition) noexcept;

		void BeginDrawing() noexcept;
		void EndDrawing() noexcept;

		auto GetDevice() const noexcept { return m_Device11; }
		auto GetDeviceContext() const noexcept { return m_DeviceContext11; }
		auto GetWindowSize() const noexcept { return m_WindowSize; }

	private:
		// Called in Create()
		void CreateDeviceAndSwapChain(HWND hWnd) noexcept;

		/// VS Shader & input layout creation
		/// Called in Create()
		void CreateVSBase() noexcept;
		void CreateVSRaw() noexcept;
		void CreateVSSkyMap() noexcept;
		void CreateVSInstantText() noexcept;
		void CreateAndSetVSCBs() noexcept;

		/// PS Shader creation
		void CreatePSBase() noexcept;
		void CreatePSRaw() noexcept;
		void CreatePSSkyMap() noexcept;
		void CreatePSInstantText() noexcept;
		void CreateAndSetPSCBs() noexcept;
		
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
		XMFLOAT2	m_WindowSize{};
		FLOAT		m_ClearColor[4]{};

		IDXGISwapChain*			m_SwapChain{};
		ID3D11Device*			m_Device11{};
		ID3D11DeviceContext*	m_DeviceContext11{};

		// Shader and input layout
		ID3D11InputLayout*	m_VSBaseInputLayout{};
		ID3D10Blob*			m_VSBaseBuffer{};
		ID3D11VertexShader*	m_VSBase{};
		ID3D10Blob*			m_VSRawBuffer{};
		ID3D11VertexShader*	m_VSRaw{};
		ID3D10Blob*			m_VSSkyMapBuffer{};
		ID3D11VertexShader*	m_VSSkyMap{};
		ID3D11InputLayout*	m_VSInstantTextInputLayout{};
		ID3D10Blob*			m_VSInstantTextBlob{};
		ID3D11VertexShader*	m_VSInstantText{};
		ID3D10Blob*			m_PSBaseBuffer{};
		ID3D11PixelShader*	m_PSBase{};
		ID3D10Blob*			m_PSRawBuffer{};
		ID3D11PixelShader*	m_PSRaw{};
		ID3D10Blob*			m_PSSkyMapBuffer{};
		ID3D11PixelShader*	m_PSSkyMap{};
		ID3D10Blob*			m_PSInstantTextBlob{};
		ID3D11PixelShader*	m_PSInstantText{};
		EVertexShader		m_CurrentVS{ EVertexShader::Invalid };
		EPixelShader		m_CurrentPS{ EPixelShader::Invalid };

		// Shader constant buffer
		ID3D11Buffer*		m_VSCBSpace{};
		ID3D11Buffer*		m_VSCBFlags{};
		ID3D11Buffer*		m_VSCBCPUAnimationData{};
		ID3D11Buffer*		m_VSCBGPUAnimationData{};
		ID3D11Buffer*		m_PSCBFlags{};
		SPSCBFlags			m_PSCBFlagsData{};
		ID3D11Buffer*		m_PSCBLights{};
		ID3D11Buffer*		m_PSCBCamera{};
		SPSCBCamera			m_PSCBCameraData{};

		ID3D11DepthStencilView*		m_DepthStencilView11{};
		ID3D11RenderTargetView*		m_RenderTargetView11{};
		D3D11_VIEWPORT				m_DefaultViewPort{};

		///
		/// States
		///
		EDepthStencilState			m_CurrentDepthStencilState{ EDepthStencilState::Invalid };
		ID3D11DepthStencilState*	m_DepthStencilStateZEnabled11{};
		ID3D11DepthStencilState*	m_DepthStencilStateZDisabled11{};

		ERasterizerState			m_CurrentRasterizerState{};
		ERasterizerState			m_PreviousRasterizerState{};
		ID3D11RasterizerState*		m_RasterizerStateWireFrame11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidNoCull11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidBackCullCCW11{};
		ID3D11RasterizerState*		m_RasterizerStateSolidBackCullCW11{};

		EBlendState					m_CurerntBlendState{ EBlendState::Invalid };
		ID3D11BlendState*			m_BlendStateTransparent{};
		ID3D11BlendState*			m_BlendStateOpaque{};
		
		ESamplerState				m_CurrentSamplerState{ ESamplerState::Invalid };
		ID3D11SamplerState*			m_SamplerStateMinMagMipLinearWrap{};
		ID3D11SamplerState*			m_SamplerStateMinMagMipPointWrap{};

		EPrimitiveTopology			m_CurrentPrimitiveTopology{ EPrimitiveTopology::Invalid };
	};
};