#include "JWDX.h"
#include "JWWin32Window.h"

using namespace JWEngine;

JWDX::~JWDX()
{
	uint64_t reference_count{};

	// Release the COM objects we created.
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_SamplerStateMinMagMipPointWrap);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_SamplerStateMinMagMipLinearWrap);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_BlendStateOpaque);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_BlendStateTransparent);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_RasterizerStateSolidBackCullCW11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_RasterizerStateSolidBackCullCCW11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_RasterizerStateSolidNoCull11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_RasterizerStateWireFrame11);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_RenderTargetView11);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_DepthStencilStateZDisabled11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_DepthStencilStateZEnabled11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_DepthStencilView11);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSCBCamera);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSCBLights);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSCBFlags);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSCBGPUAnimation);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSCBCPUAnimation);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSCBFlags);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSCBSpace);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSSkyMap);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSSkyMapBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSRaw);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSRawBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSBase);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_PSBaseBuffer);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSSkyMap);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSSkyMapBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSRaw);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSRawBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSAnim);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSAnimBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSBase);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_VSBaseBuffer);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_IAInputLayoutAnim);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_IAInputLayoutBase);

	JW_RELEASE_CHECK_REFERENCE_COUNT(m_DeviceContext11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_Device11);
	JW_RELEASE_CHECK_REFERENCE_COUNT(m_SwapChain);

	// Reference count check!
	assert(reference_count == 0);
}

void JWDX::Create(const JWWin32Window& Window, STRING Directory, const SClearColor& ClearColor) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);
	
	// Set base directory
	m_BaseDirectory = Directory;

	// Set window size
	m_WindowSize.Width = Window.GetWidth();
	m_WindowSize.Height = Window.GetHeight();

	// Set clear color
	m_ClearColor[0] = ClearColor.R;
	m_ClearColor[1] = ClearColor.G;
	m_ClearColor[2] = ClearColor.B;
	m_ClearColor[3] = 1.0f;

	// Create device and swap chain
	CreateDeviceAndSwapChain(Window.GethWnd());

	// Create VS shaders, input layout and constant buffers
	CreateVSBase();
	CreateVSAnim();
	CreateVSRaw();
	CreateVSSkyMap();
	CreateVSCBs();

	// Create PS shaders and constant buffers
	CreatePSBase();
	CreatePSRaw();
	CreatePSSkyMap();
	CreatePSCBs();

	// Set default shaders
	SetVS(EVertexShader::VSBase);
	SetPS(EPixelShader::PSBase);

	// Create depth-stencil view
	CreateDepthStencilView();

	// Create depth-stencil states
	CreateDepthStencilStates();

	// Create render target view
	CreateRenderTargetView();

	// Create rasterizer states
	CreateRasterizerStates();

	// Create sampler states
	CreateSamplerStates();

	// Create blend states
	CreateBlendStates();

	// Create viewport
	CreateDefaultViewport();

	m_IsValid = true;
}

PRIVATE void JWDX::CreateDeviceAndSwapChain(HWND hWnd) noexcept
{
	// Describe the screen buffer
	DXGI_MODE_DESC buffer_description{};
	buffer_description.Width = m_WindowSize.Width;
	buffer_description.Height = m_WindowSize.Height;
	buffer_description.RefreshRate.Numerator = 60;
	buffer_description.RefreshRate.Denominator = 1;
	buffer_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	buffer_description.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	buffer_description.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Describe the SwapChain
	DXGI_SWAP_CHAIN_DESC swap_chain_description{};
	swap_chain_description.BufferDesc = buffer_description;
	swap_chain_description.SampleDesc.Count = 1;
	swap_chain_description.SampleDesc.Quality = 0;
	swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_description.BufferCount = 1;
	swap_chain_description.OutputWindow = hWnd;
	swap_chain_description.Windowed = TRUE;
	swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Create the Device and the SwapChain
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION, &swap_chain_description, &m_SwapChain,
		&m_Device11, nullptr, &m_DeviceContext11);
}

PRIVATE void JWDX::CreateVSBase() noexcept
{
	// Compile shader from file
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\VSBase.hlsl";

	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_VSBaseBuffer, nullptr);

	// Create shader
	m_Device11->CreateVertexShader(m_VSBaseBuffer->GetBufferPointer(), m_VSBaseBuffer->GetBufferSize(), nullptr, &m_VSBase);

	// Create input layout
	m_Device11->CreateInputLayout(KInputElementDescriptionBase, ARRAYSIZE(KInputElementDescriptionBase),
		m_VSBaseBuffer->GetBufferPointer(), m_VSBaseBuffer->GetBufferSize(), &m_IAInputLayoutBase);
}

PRIVATE void JWDX::CreateVSAnim() noexcept
{
	// Compile shader from file
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\VSAnim.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_VSAnimBuffer, nullptr);

	// Create shader
	m_Device11->CreateVertexShader(m_VSAnimBuffer->GetBufferPointer(), m_VSAnimBuffer->GetBufferSize(), nullptr, &m_VSAnim);

	// Create input layout
	m_Device11->CreateInputLayout(KInputElementDescriptionAnim, ARRAYSIZE(KInputElementDescriptionAnim),
		m_VSAnimBuffer->GetBufferPointer(), m_VSAnimBuffer->GetBufferSize(), &m_IAInputLayoutAnim);
}

PRIVATE void JWDX::CreateVSRaw() noexcept
{
	// Compile shader from file
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\VSRaw.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_VSRawBuffer, nullptr);

	// Create shader
	m_Device11->CreateVertexShader(m_VSRawBuffer->GetBufferPointer(), m_VSRawBuffer->GetBufferSize(), nullptr, &m_VSRaw);
}

PRIVATE void JWDX::CreateVSSkyMap() noexcept
{
	// Compile shader from file (SkyMap)
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\VSSkyMap.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_VSSkyMapBuffer, nullptr);

	// Create shader (SkyMap)
	m_Device11->CreateVertexShader(m_VSSkyMapBuffer->GetBufferPointer(), m_VSSkyMapBuffer->GetBufferSize(), nullptr, &m_VSSkyMap);
}

PRIVATE void JWDX::CreatePSBase() noexcept
{
	// Compile shader from file
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\PSBase.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_PSBaseBuffer, nullptr);

	// Create shader
	m_Device11->CreatePixelShader(m_PSBaseBuffer->GetBufferPointer(), m_PSBaseBuffer->GetBufferSize(), nullptr, &m_PSBase);
}

PRIVATE void JWDX::CreatePSRaw() noexcept
{
	// Compile shader from file
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\PSRaw.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_PSRawBuffer, nullptr);

	// Create shader
	m_Device11->CreatePixelShader(m_PSRawBuffer->GetBufferPointer(), m_PSRawBuffer->GetBufferSize(), nullptr, &m_PSRaw);
}

PRIVATE void JWDX::CreatePSSkyMap() noexcept
{
	// Compile shader from file (SkyMap)
	WSTRING shader_file_name;
	shader_file_name = StringToWstring(m_BaseDirectory) + L"Shaders\\PSSkyMap.hlsl";
	D3DCompileFromFile(shader_file_name.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_PSSkyMapBuffer, nullptr);

	// Create shader (SkyMap)
	m_Device11->CreatePixelShader(m_PSSkyMapBuffer->GetBufferPointer(), m_PSSkyMapBuffer->GetBufferSize(), nullptr, &m_PSSkyMap);
}

PRIVATE void JWDX::CreateVSCBs() noexcept
{
	// Create buffer to send to constant buffer in HLSL
	D3D11_BUFFER_DESC constant_buffer_description{};
	constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_description.ByteWidth = sizeof(SVSCBSpace);
	constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_description.CPUAccessFlags = 0;
	constant_buffer_description.MiscFlags = 0;
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_VSCBSpace);

	constant_buffer_description.ByteWidth = sizeof(SVSCBFlags);
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_VSCBFlags);

	constant_buffer_description.ByteWidth = sizeof(SVSCBCPUAnimation);
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_VSCBCPUAnimation);

	constant_buffer_description.ByteWidth = sizeof(SVSCBGPUAnimation);
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_VSCBGPUAnimation);
}

PRIVATE void JWDX::CreatePSCBs() noexcept
{
	// Create buffer to send to constant buffer in HLSL
	D3D11_BUFFER_DESC constant_buffer_description{};
	constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_description.ByteWidth = sizeof(SPSCBFlags);
	constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_description.CPUAccessFlags = 0;
	constant_buffer_description.MiscFlags = 0;
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_PSCBFlags);

	constant_buffer_description.ByteWidth = sizeof(SPSCBLights);
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_PSCBLights);

	constant_buffer_description.ByteWidth = sizeof(SPSCBCamera);
	m_Device11->CreateBuffer(&constant_buffer_description, nullptr, &m_PSCBCamera);
}

PRIVATE void JWDX::CreateDepthStencilView() noexcept
{
	// Describe depth-stencil buffer
	D3D11_TEXTURE2D_DESC depth_stencil_texture_descrption{};
	depth_stencil_texture_descrption.Width = m_WindowSize.Width;
	depth_stencil_texture_descrption.Height = m_WindowSize.Height;
	depth_stencil_texture_descrption.MipLevels = 1;
	depth_stencil_texture_descrption.ArraySize = 1;
	depth_stencil_texture_descrption.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_texture_descrption.SampleDesc.Count = 1;
	depth_stencil_texture_descrption.SampleDesc.Quality = 0;
	depth_stencil_texture_descrption.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_texture_descrption.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_texture_descrption.CPUAccessFlags = 0;
	depth_stencil_texture_descrption.MiscFlags = 0;
	
	// Create buffer for depth-stencil view
	ID3D11Texture2D* depth_stencil_buffer{};
	m_Device11->CreateTexture2D(&depth_stencil_texture_descrption, nullptr, &depth_stencil_buffer);

	// Create the depth-stencil View
	m_Device11->CreateDepthStencilView(depth_stencil_buffer, nullptr, &m_DepthStencilView11);
	
	JW_RELEASE(depth_stencil_buffer);
}

PRIVATE void JWDX::CreateDepthStencilStates() noexcept
{
	D3D11_DEPTH_STENCIL_DESC depth_stencil_description{};

	depth_stencil_description.DepthEnable = TRUE;
	depth_stencil_description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depth_stencil_description.StencilEnable = FALSE;
	depth_stencil_description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depth_stencil_description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depth_stencil_description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	depth_stencil_description.BackFace = depth_stencil_description.FrontFace;

	m_Device11->CreateDepthStencilState(&depth_stencil_description, &m_DepthStencilStateZEnabled11);


	depth_stencil_description.DepthEnable = FALSE;
	depth_stencil_description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Read-only

	m_Device11->CreateDepthStencilState(&depth_stencil_description, &m_DepthStencilStateZDisabled11);
}

PRIVATE void JWDX::CreateRenderTargetView() noexcept
{
	// Create buffer for render target view
	ID3D11Texture2D* back_buffer{};
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);

	// Create render target view
	m_Device11->CreateRenderTargetView(back_buffer, nullptr, &m_RenderTargetView11);
	
	// Set render target view & depth-stencil view
	m_DeviceContext11->OMSetRenderTargets(1, &m_RenderTargetView11, m_DepthStencilView11);

	JW_RELEASE(back_buffer);
}

PRIVATE void JWDX::CreateRasterizerStates() noexcept
{
	D3D11_RASTERIZER_DESC rasterizer_description{};
	rasterizer_description.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_description.CullMode = D3D11_CULL_NONE;

	m_Device11->CreateRasterizerState(&rasterizer_description, &m_RasterizerStateWireFrame11);

	rasterizer_description.FillMode = D3D11_FILL_SOLID;
	rasterizer_description.CullMode = D3D11_CULL_NONE;

	m_Device11->CreateRasterizerState(&rasterizer_description, &m_RasterizerStateSolidNoCull11);

	rasterizer_description.FillMode = D3D11_FILL_SOLID;
	rasterizer_description.CullMode = D3D11_CULL_BACK;
	rasterizer_description.FrontCounterClockwise = true;

	m_Device11->CreateRasterizerState(&rasterizer_description, &m_RasterizerStateSolidBackCullCCW11);

	rasterizer_description.FrontCounterClockwise = false;
	m_Device11->CreateRasterizerState(&rasterizer_description, &m_RasterizerStateSolidBackCullCW11);
}

PRIVATE void JWDX::CreateBlendStates() noexcept
{
	D3D11_BLEND_DESC blend_description{};
	blend_description.RenderTarget[0].BlendEnable = true;

	blend_description.RenderTarget[0].SrcBlend = blend_description.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blend_description.RenderTarget[0].DestBlend = blend_description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blend_description.RenderTarget[0].BlendOp = blend_description.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blend_description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_Device11->CreateBlendState(&blend_description, &m_BlendStateTransparent);

	blend_description.RenderTarget[0].BlendEnable = false;

	m_Device11->CreateBlendState(&blend_description, &m_BlendStateOpaque);
}

PRIVATE void JWDX::CreateSamplerStates() noexcept
{
	D3D11_SAMPLER_DESC sampler_description{};
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_description.MinLOD = 0;
	sampler_description.MaxLOD = D3D11_FLOAT32_MAX;

	m_Device11->CreateSamplerState(&sampler_description, &m_SamplerStateMinMagMipLinearWrap);

	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_description.MinLOD = 0;
	sampler_description.MaxLOD = D3D11_FLOAT32_MAX;

	m_Device11->CreateSamplerState(&sampler_description, &m_SamplerStateMinMagMipPointWrap);
}

PRIVATE void JWDX::CreateDefaultViewport() noexcept
{
	// Setup the viewport
	m_DefaultViewPort.TopLeftX = 0;
	m_DefaultViewPort.TopLeftY = 0;
	m_DefaultViewPort.Width = static_cast<FLOAT>(m_WindowSize.Width);
	m_DefaultViewPort.Height = static_cast<FLOAT>(m_WindowSize.Height);
	m_DefaultViewPort.MinDepth = 0.0f; // IMPORTANT!
	m_DefaultViewPort.MaxDepth = 1.0f; // IMPORTANT!

	// Set the viewport
	m_DeviceContext11->RSSetViewports(1, &m_DefaultViewPort);
}

void JWDX::CreateDynamicVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_description.ByteWidth = ByteSize;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = pData;

	m_Device11->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, ppBuffer);
}

void JWDX::CreateStaticVertexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept
{
	D3D11_BUFFER_DESC vertex_buffer_description{};
	vertex_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_description.ByteWidth = ByteSize;
	vertex_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_description.CPUAccessFlags = 0;
	vertex_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data{};
	vertex_buffer_data.pSysMem = pData;

	m_Device11->CreateBuffer(&vertex_buffer_description, &vertex_buffer_data, ppBuffer);
}

void JWDX::CreateIndexBuffer(UINT ByteSize, const void* pData, ID3D11Buffer** ppBuffer) noexcept
{
	D3D11_BUFFER_DESC index_buffer_description{};
	index_buffer_description.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_description.ByteWidth = ByteSize;
	index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_description.CPUAccessFlags = 0;
	index_buffer_description.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA index_buffer_data{};
	index_buffer_data.pSysMem = pData;

	m_Device11->CreateBuffer(&index_buffer_description, &index_buffer_data, ppBuffer);
}

void JWDX::SetRasterizerState(ERasterizerState State) noexcept
{
	switch (State)
	{
	case JWEngine::ERasterizerState::WireFrame:
		m_DeviceContext11->RSSetState(m_RasterizerStateWireFrame11);
		break;
	case JWEngine::ERasterizerState::SolidNoCull:
		m_DeviceContext11->RSSetState(m_RasterizerStateSolidNoCull11);
		break;
	case JWEngine::ERasterizerState::SolidBackCullCCW:
		m_DeviceContext11->RSSetState(m_RasterizerStateSolidBackCullCCW11);
		break;
	case JWEngine::ERasterizerState::SolidBackCullCW:
		m_DeviceContext11->RSSetState(m_RasterizerStateSolidBackCullCW11);
		break;
	default:
		break;
	}
}

void JWDX::SetBlendState(EBlendState State) noexcept
{
	switch (State)
	{
	case JWEngine::EBlendState::Transprent:
		m_DeviceContext11->OMSetBlendState(m_BlendStateTransparent, 0, 0xFFFFFFFF);
		break;
	case JWEngine::EBlendState::Opaque:
		m_DeviceContext11->OMSetBlendState(m_BlendStateOpaque, 0, 0xFFFFFFFF);
		break;
	default:
		break;
	}
}

void JWDX::SetPSSamplerState(ESamplerState State) noexcept
{
	switch (State)
	{
	case JWEngine::ESamplerState::MinMagMipLinearWrap:
		m_DeviceContext11->PSSetSamplers(0, 1, &m_SamplerStateMinMagMipLinearWrap);
		break;
	case JWEngine::ESamplerState::MinMagMipPointWrap:
		m_DeviceContext11->PSSetSamplers(0, 1, &m_SamplerStateMinMagMipPointWrap);
		break;
	default:
		break;
	}
}

void JWDX::SetDepthStencilState(EDepthStencilState State) noexcept
{
	switch (State)
	{
	case JWEngine::EDepthStencilState::ZEnabled:
		m_DeviceContext11->OMSetDepthStencilState(m_DepthStencilStateZEnabled11, 0);
		break;
	case JWEngine::EDepthStencilState::ZDisabled:
		m_DeviceContext11->OMSetDepthStencilState(m_DepthStencilStateZDisabled11, 0);
		break;
	default:
		break;
	}
}

void JWDX::SetVS(EVertexShader VS) noexcept
{
	switch (VS)
	{
	case JWEngine::EVertexShader::VSBase:
		m_DeviceContext11->IASetInputLayout(m_IAInputLayoutBase);
		m_DeviceContext11->VSSetShader(m_VSBase, nullptr, 0);
		break;
	case JWEngine::EVertexShader::VSAnim:
		m_DeviceContext11->IASetInputLayout(m_IAInputLayoutAnim);
		m_DeviceContext11->VSSetShader(m_VSAnim, nullptr, 0);
		break;
	case JWEngine::EVertexShader::VSRaw:
		m_DeviceContext11->IASetInputLayout(nullptr);
		m_DeviceContext11->VSSetShader(m_VSRaw, nullptr, 0);
		break;
	case JWEngine::EVertexShader::VSSkyMap:
		m_DeviceContext11->IASetInputLayout(m_IAInputLayoutBase);
		m_DeviceContext11->VSSetShader(m_VSSkyMap, nullptr, 0);
		break;
	default:
		break;
	}
}

void JWDX::SetPS(EPixelShader PS) noexcept
{
	switch (PS)
	{
	case JWEngine::EPixelShader::PSBase:
		m_DeviceContext11->PSSetShader(m_PSBase, nullptr, 0);
		break;
	case JWEngine::EPixelShader::PSRaw:
		m_DeviceContext11->PSSetShader(m_PSRaw, nullptr, 0);
		break;
	case JWEngine::EPixelShader::PSSkyMap:
		m_DeviceContext11->PSSetShader(m_PSSkyMap, nullptr, 0);
		break;
	default:
		break;
	}
}

void JWDX::UpdateVSCBSpace(const SVSCBSpace& Data) noexcept
{
	m_DeviceContext11->UpdateSubresource(m_VSCBSpace, 0, nullptr, &Data, 0, 0);
	m_DeviceContext11->VSSetConstantBuffers(0, 1, &m_VSCBSpace);
}

void JWDX::UpdateVSCBFlags(const SVSCBFlags& Data) noexcept
{
	m_DeviceContext11->UpdateSubresource(m_VSCBFlags, 0, nullptr, &Data, 0, 0);
	m_DeviceContext11->VSSetConstantBuffers(1, 1, &m_VSCBFlags);
}

void JWDX::UpdateVSCBCPUAnimation(const SVSCBCPUAnimation& Data) noexcept
{
	m_DeviceContext11->UpdateSubresource(m_VSCBCPUAnimation, 0, nullptr, &Data, 0, 0);
	m_DeviceContext11->VSSetConstantBuffers(2, 1, &m_VSCBCPUAnimation);
}

void JWDX::UpdateVSCBGPUAnimation(const SVSCBGPUAnimation& Data) noexcept
{
	m_DeviceContext11->UpdateSubresource(m_VSCBGPUAnimation, 0, nullptr, &Data, 0, 0);
	m_DeviceContext11->VSSetConstantBuffers(3, 1, &m_VSCBGPUAnimation);
}

void JWDX::UpdatePSCBFlags(bool HasTexture, bool UseLighting) noexcept
{
	if (HasTexture)
	{
		m_PSCBFlagsData.HasTexture = TRUE;
	}
	else
	{
		m_PSCBFlagsData.HasTexture = FALSE;
	}

	if (UseLighting)
	{
		m_PSCBFlagsData.UseLighting = TRUE;
	}
	else
	{
		m_PSCBFlagsData.UseLighting = FALSE;
	}

	m_DeviceContext11->UpdateSubresource(m_PSCBFlags, 0, nullptr, &m_PSCBFlagsData, 0, 0);
	m_DeviceContext11->PSSetConstantBuffers(0, 1, &m_PSCBFlags);
}

void JWDX::UpdatePSCBLights(const SPSCBLights& Data) noexcept
{
	m_DeviceContext11->UpdateSubresource(m_PSCBLights, 0, nullptr, &Data, 0, 0);
	m_DeviceContext11->PSSetConstantBuffers(1, 1, &m_PSCBLights);
}

void JWDX::UpdatePSCBCamera(const XMFLOAT4& CameraPosition) noexcept
{
	m_PSCBCameraData.CameraPosition = CameraPosition;

	m_DeviceContext11->UpdateSubresource(m_PSCBCamera, 0, nullptr, &m_PSCBCameraData, 0, 0);
	m_DeviceContext11->PSSetConstantBuffers(2, 1, &m_PSCBCamera);
}

void JWDX::BeginDrawing() noexcept
{
	// Clear render target view
	m_DeviceContext11->ClearRenderTargetView(m_RenderTargetView11, m_ClearColor);

	// Clear depth-stencil view
	m_DeviceContext11->ClearDepthStencilView(m_DepthStencilView11, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void JWDX::EndDrawing() noexcept
{
	// Present back buffer to screen
	m_SwapChain->Present(0, 0);
}

auto JWDX::GetDevice() const noexcept->ID3D11Device*
{
	return m_Device11;
}

auto JWDX::GetDeviceContext() const noexcept->ID3D11DeviceContext*
{
	return m_DeviceContext11;
}

auto JWDX::GetWindowSize() const noexcept->SSizeInt
{
	return m_WindowSize;
}