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
		LineStrip,
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
		MinMagMipLinearWrapBias2,
		MinMagMipPointWrap,
		Anisotropic,
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

	enum class EGeometryShader
	{
		None,
		GSNormal,
	};

	enum class EPixelShader
	{
		Invalid,
		PSBase,
		PSRaw,
		PSSkyMap,
		PSIntantText,
	};
	
	enum class EAllowedDisplayMode : uint32_t
	{
		// 4:3 ratio
		w640h480,
		w800h600,
		w1024h768,

		// 16:9 ratio
		w1280h720,
		w1366h768,
		w1600h900,
		w1920h1080,
	};
	static const SSize2 KAllowedDisplayModes[] = {
		// 4:3 ratio
		SSize2(640, 480), SSize2(800, 600), SSize2(1024, 768),

		// 16:9 ratio
		SSize2(1280, 720), SSize2(1366, 768), SSize2(1600, 900), SSize2(1920, 1080)
	};
	static const uint32_t KAllowedDisplayModeCount{ sizeof(KAllowedDisplayModes) / sizeof(SSize2) };
	using DisplayModeIndex = uint32_t;

	static auto ConvertEAllowedDisplayModeToModeSize(EAllowedDisplayMode Mode)->SSize2
	{
		return KAllowedDisplayModes[static_cast<DisplayModeIndex>(Mode)];
	}

	static auto ConvertModeSizeToEAllowedDisplayMode(const SSize2& ModeSize)->EAllowedDisplayMode
	{
		DisplayModeIndex index{};
		for (const auto& iter : KAllowedDisplayModes)
		{
			if (iter == ModeSize) { return static_cast<EAllowedDisplayMode>(index); }
			++index;
		}
		return EAllowedDisplayMode::w640h480;
	}

	class JWDX
	{
	public:
		JWDX() = default;
		~JWDX() = default;

		void Create(HWND hWnd, const SSize2& WindowSize, EAllowedDisplayMode InitialMode, STRING Directory, const SClearColor& ClearColor) noexcept;
		void Destroy() noexcept;

		// Factory functions
		void CreateDynamicVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateStaticVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		void CreateIndexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept;
		
		// Only 32-bit color is available (R8G8B8A8 UNORM)
		void SetCurrentDisplayMode(EAllowedDisplayMode Mode) noexcept;
		void SetToFullScreenDisplayMode() noexcept;
		void SetToWindowedDisplayMode() noexcept;
	private:
		void SetDisplayMode(EAllowedDisplayMode Mode) noexcept;

	public:
		
		inline void UpdateDynamicResource(ID3D11Resource* pResource, const void* pData, size_t Size) noexcept;

		// Rasterizer state
		auto GetRasterizerState() const noexcept { return m_CurrentRasterizerState; }
		auto GetPreviousRasterizerState() const noexcept { return m_PreviousRasterizerState; }
		void SetRasterizerState(ERasterizerState State) noexcept;
		void SwitchRasterizerState() noexcept;

		void SetPrimitiveTopology(EPrimitiveTopology Topology) noexcept;
		void SetDepthStencilState(EDepthStencilState State) noexcept;
		void SetBlendState(EBlendState State) noexcept;
		void SetPSSamplerState(ESamplerState State) noexcept;

		void SetVS(EVertexShader VS) noexcept;
		void SetGS(EGeometryShader GS) noexcept;
		void SetPS(EPixelShader PS) noexcept;

		// Update VS constant buffers
		void UpdateVSCBSpace(const SVSCBSpace& Data) noexcept;
		void UpdateVSCBFlags(const SVSCBFlags& Data) noexcept;
		void UpdateVSCBCPUAnimationData(const SVSCBCPUAnimationData& Data) noexcept;
		void UpdateVSCBGPUAnimationData(const SVSCBGPUAnimationData& Data) noexcept;

		// Update PS constant buffers
		void UpdatePSCBFlags(const SPSCBFlags& Data) noexcept;
		void UpdatePSCBLights(const SPSCBLights& Data) noexcept;
		void UpdatePSCBCamera(const XMVECTOR& CameraPosition) noexcept;

		void BeginDrawing() noexcept;
		void EndDrawing() noexcept;

		auto GetSwapChain() const noexcept { return m_SwapChain; }
		auto GetDevice() const noexcept { return m_Device11; }
		auto GetDeviceContext() const noexcept { return m_DeviceContext11; }
		auto GetWindowedDisplayMode() const noexcept { return m_WindowedDisplayMode; }
		auto GetFullScreenDisplayMode() const noexcept { return m_FullScreenDisplayMode; }
		const auto& GetAvailableDisplayModes() const noexcept { return m_AvailableDisplayModes; }
		const auto& GetUniversalOrthoProjMatrix() const noexcept { return m_UniversalOrthoProjMat; }

	private:
		void CreateAllDX(HWND hWnd) noexcept;
		void DestroyAllDX() noexcept;

		// Called in Create()
		void CreateDeviceAndSwapChain(HWND hWnd) noexcept;

		void UpdateAvailableDisplayModes() noexcept;
		void UpdateDefaultViewport() noexcept;
		void UpdateUniversalOrthoProjMatrix() noexcept;

		// VS Shader & input layout creation
		void CreateVSBase() noexcept;
		void CreateVSRaw() noexcept;
		void CreateVSSkyMap() noexcept;
		void CreateVSInstantText() noexcept;
		void CreateAndSetVSCBs() noexcept;

		// GS Shader creation
		void CreateGSNormal() noexcept;

		// PS Shader creation
		void CreatePSBase() noexcept;
		void CreatePSRaw() noexcept;
		void CreatePSSkyMap() noexcept;
		void CreatePSInstantText() noexcept;
		void CreateAndSetPSCBs() noexcept;


		// Views
		void CreateRenderTargetView() noexcept;
		void CreateDepthStencilView() noexcept;
		void SetViews() noexcept;
		void DestroyViews() noexcept;
		

		// States
		void CreateDepthStencilStates() noexcept;
		void CreateRasterizerStates() noexcept;
		void CreateBlendStates() noexcept;
		void CreateSamplerStates() noexcept;

	private:
		bool			m_IsCreated{ false };

		STRING			m_BaseDirectory;
		const SSize2*	m_pWindowSize{};
		FLOAT			m_ClearColor[4]{};

		IDXGISwapChain*			m_SwapChain{};
		ID3D11Device*			m_Device11{};
		ID3D11DeviceContext*	m_DeviceContext11{};

		// Display mode & projection matrix
		VECTOR<SSize2>			m_AvailableDisplayModes{};
		XMMATRIX				m_UniversalOrthoProjMat{};
		EAllowedDisplayMode		m_FullScreenDisplayMode{};
		EAllowedDisplayMode		m_WindowedDisplayMode{};

		// Shaders and input layouts
		ID3D11InputLayout*		m_VSBaseInputLayout{};
		ID3D10Blob*				m_VSBaseBuffer{};
		ID3D11VertexShader*		m_VSBase{};
		ID3D10Blob*				m_VSRawBuffer{};
		ID3D11VertexShader*		m_VSRaw{};
		ID3D10Blob*				m_VSSkyMapBuffer{};
		ID3D11VertexShader*		m_VSSkyMap{};
		ID3D11InputLayout*		m_VSInstantTextInputLayout{};
		ID3D10Blob*				m_VSInstantTextBlob{};
		ID3D11VertexShader*		m_VSInstantText{};
		ID3D10Blob*				m_GSNormalBuffer{};
		ID3D11GeometryShader*	m_GSNormal{};
		ID3D10Blob*				m_PSBaseBuffer{};
		ID3D11PixelShader*		m_PSBase{};
		ID3D10Blob*				m_PSRawBuffer{};
		ID3D11PixelShader*		m_PSRaw{};
		ID3D10Blob*				m_PSSkyMapBuffer{};
		ID3D11PixelShader*		m_PSSkyMap{};
		ID3D10Blob*				m_PSInstantTextBlob{};
		ID3D11PixelShader*		m_PSInstantText{};
		EVertexShader			m_CurrentVS{ EVertexShader::Invalid };
		EGeometryShader			m_CurrentGS{ EGeometryShader::None };
		EPixelShader			m_CurrentPS{ EPixelShader::Invalid };

		// Shader constant buffers
		ID3D11Buffer*			m_VSCBSpace{};
		ID3D11Buffer*			m_VSCBFlags{};
		ID3D11Buffer*			m_VSCBCPUAnimationData{};
		ID3D11Buffer*			m_VSCBGPUAnimationData{};
		ID3D11Buffer*			m_PSCBFlags{};
		ID3D11Buffer*			m_PSCBLights{};
		ID3D11Buffer*			m_PSCBCamera{};
		SPSCBCamera				m_PSCBCameraData{};

		ID3D11RenderTargetView*		m_RenderTargetView{};
		ID3D11Texture2D*			m_RenderTargetTexture{};
		ID3D11DepthStencilView*		m_DepthStencilView{};
		D3D11_VIEWPORT				m_DefaultViewPort{};

		//
		// States
		//
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
		ID3D11SamplerState*			m_SamplerStateMinMagMipLinearWrapBias2{};
		ID3D11SamplerState*			m_SamplerStateMinMagMipPointWrap{};
		ID3D11SamplerState*			m_SamplerStateAnisotropic{};

		EPrimitiveTopology			m_CurrentPrimitiveTopology{ EPrimitiveTopology::Invalid };
	};
};