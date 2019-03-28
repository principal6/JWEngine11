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

		void SetVSBase() noexcept;
		void SetVSAnim() noexcept;
		void SetVSRaw() noexcept;

		void SetPSBase() noexcept;
		void SetPSRaw() noexcept;
		// Called in each JWModel(/JWImage/JWLine)'s Draw()-Update() function
		void SetVSCBStatic(SVSCBStatic& Data) noexcept;
		// Called in each animated JWModel's Animate() function
		void SetVSCBSkinned(SVSCBSkinned& Data) noexcept;
		// Called in each JWModel(/JWImage/JWLine)'s Draw()-Update() function
		void SetPSCBDefaultFlags(bool HasTexture, bool UseLighting) noexcept;
		// Called when Ambient Light is set by JWGame::AddLight()
		void SetPSCBDefaultAmbientLight(XMFLOAT4 AmbientColor) noexcept;
		// Called when Directional Light is set by JWGame::AddLight()
		void SetPSCBDefaultDirectionalLight(XMFLOAT4 DirectionalColor, XMFLOAT3 DirectionalDirection) noexcept;
		// Called once per game loop, which is when the camera's position would probably be changed.
		void SetPSCBDefaultCameraPosition(XMFLOAT4 CameraPosition) noexcept;

		void SetColorVS() noexcept;
		void SetColorPS() noexcept;
		void SetColorVSConstantBufferData(SVSCBColor& Data) noexcept;

		void BeginDrawing(const SClearColor& ClearColor) noexcept;
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
		void CreateVSCBs() noexcept;
		void CreateColorVS() noexcept;
		void CreateColorVSCB() noexcept;

		// PS Shader cretion
		void CreatePSBase() noexcept;
		void CreatePSRaw() noexcept;
		void CreatePSCB() noexcept;
		void CreateColorPS() noexcept;

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

		ID3D10Blob*			m_VSBaseBuffer{};
		ID3D11VertexShader*	m_VSBase{};
		ID3D11InputLayout*	m_VSBaseInputLayout{};
		ID3D10Blob*			m_VSAnimBuffer{};
		ID3D11VertexShader*	m_VSAnim{};
		ID3D11InputLayout*	m_VSAnimInputLayout{};
		ID3D10Blob*			m_VSRawBuffer{};
		ID3D11VertexShader*	m_VSRaw{};
		ID3D10Blob*			m_PSBaseBuffer{};
		ID3D11PixelShader*	m_PSBase{};
		ID3D10Blob*			m_PSRawBuffer{};
		ID3D11PixelShader*	m_PSRaw{};
		ID3D11Buffer*		m_VSCBStatic{};
		SVSCBStatic			m_VSCBStaticData{};
		ID3D11Buffer*		m_VSCBSkinned{};
		SVSCBSkinned		m_VSCBSkinnedData{};
		ID3D11Buffer*		m_PSCBDefault{};
		SPSCBDefault		m_PSCBDefaultData{};

		ID3D10Blob* m_ColorVSBuffer{};
		ID3D11VertexShader* m_ColorVS11{};
		ID3D11InputLayout* m_ColorVSInputLayout11{};
		ID3D10Blob* m_ColorPSBuffer{};
		ID3D11PixelShader* m_ColorPS11{};
		ID3D11Buffer* m_ColorVSCB{};
		SVSCBColor m_ColorVSCBData;

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
		
		ID3D11SamplerState* m_SamplerStateMinMagMipLinearWrap{};
		ID3D11SamplerState* m_SamplerStateMinMagMipPointWrap{};

		D3D11_VIEWPORT m_DefaultViewPort{};
	};
};