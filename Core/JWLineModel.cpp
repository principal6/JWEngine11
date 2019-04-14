#include "JWLineModel.h"
#include "../Core/JWDX.h"
#include "JWCamera.h"

using namespace JWEngine;

void JWLineModel::Create(JWDX& DX) noexcept
{
	assert(!m_IsCreated);

	// Set JWDX pointer.
	m_pDX = &DX;

	m_IsCreated = true;
}

void JWLineModel::Destroy() noexcept
{
	JW_RELEASE(m_VertexBuffer);
	JW_RELEASE(m_IndexBuffer);
}

void JWLineModel::Make3DGrid(float XSize, float ZSize, float GridInterval) noexcept
{
	if (m_RenderType == ERenderType::Invalid)
	{
		m_RenderType = ERenderType::Model_Line3D;
	}

	assert(m_RenderType == ERenderType::Model_Line3D);

	XSize = max(XSize, 0);
	ZSize = max(ZSize, 0);

	int grid_count_x = static_cast<int>(XSize / GridInterval) + 1;
	int grid_count_z = static_cast<int>(ZSize / GridInterval) + 1;

	int total_grid_count{};
	float position_x{}, position_z{};
	for (int i{}; i < grid_count_z; ++i)
	{
		position_z = -ZSize / 2.0f + static_cast<float>(i) * GridInterval;

		if (position_z)
		{
			m_VertexData.vVertices.push_back(SVertexNonRiggedModel(-XSize / 2.0f, 0, position_z));
			m_VertexData.vVertices.push_back(SVertexNonRiggedModel(+XSize / 2.0f, 0, position_z));

			m_IndexData.vIndices.push_back(SIndexLine(total_grid_count * 2, total_grid_count * 2 + 1));

			++total_grid_count;
		}
	}
	for (int i{}; i < grid_count_x; ++i)
	{
		position_x = -XSize / 2.0f + static_cast<float>(i) * GridInterval;

		if (position_x)
		{
			m_VertexData.vVertices.push_back(SVertexNonRiggedModel(position_x, 0, -ZSize / 2.0f));
			m_VertexData.vVertices.push_back(SVertexNonRiggedModel(position_x, 0, +ZSize / 2.0f));

			m_IndexData.vIndices.push_back(SIndexLine(total_grid_count * 2, total_grid_count * 2 + 1));

			++total_grid_count;
		}
	}

	// X Axis
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(-KAxisLength / 2.0f, 0, 0), KXAxisColor));
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(+KAxisLength / 2.0f, 0, 0), KXAxisColor));
	m_IndexData.vIndices.push_back(SIndexLine(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Y Axis
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(0, -KAxisLength / 2.0f, 0), KYAxisColor));
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(0, +KAxisLength / 2.0f, 0), KYAxisColor));
	m_IndexData.vIndices.push_back(SIndexLine(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Z Axis
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(0, 0, -KAxisLength / 2.0f), KZAxisColor));
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(XMFLOAT3(0, 0, +KAxisLength / 2.0f), KZAxisColor));
	m_IndexData.vIndices.push_back(SIndexLine(total_grid_count * 2, total_grid_count * 2 + 1));
	++total_grid_count;

	// Craete vertex & index buffer.
	AddEnd();
}

auto JWLineModel::AddLine3D(XMFLOAT3 StartPosition, XMFLOAT3 EndPosition, XMFLOAT4 Color) noexcept->JWLineModel*
{
	if (m_RenderType == ERenderType::Invalid)
	{
		m_RenderType = ERenderType::Model_Line3D;
	}
	
	assert(m_RenderType == ERenderType::Model_Line3D);
	
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(StartPosition, Color));
	m_VertexData.vVertices.push_back(SVertexNonRiggedModel(EndPosition, Color));
	m_IndexData.vIndices.push_back(SIndexLine(m_VertexData.GetCount() - 2, m_VertexData.GetCount() - 1));

	return this;
}

auto JWLineModel::AddLine2D(XMFLOAT2 StartPosition, XMFLOAT2 Length, XMFLOAT4 Color) noexcept->JWLineModel*
{
	if (m_RenderType == ERenderType::Invalid)
	{
		m_RenderType = ERenderType::Model_Line2D;
	}

	assert(m_RenderType == ERenderType::Model_Line2D);
	
	float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
	float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

	XMFLOAT3 position_a = XMFLOAT3(-window_width / 2 + StartPosition.x, window_height / 2 - StartPosition.y, 0);
	XMFLOAT3 position_b = position_a;
	position_b.x += Length.x;
	position_b.y += Length.y;
	
	m_VertexData.vVertices.emplace_back(position_a, Color);
	m_VertexData.vVertices.emplace_back(position_b, Color);
	m_IndexData.vIndices.emplace_back(m_VertexData.GetCount() - 2, m_VertexData.GetCount() - 1);

	return this;
}

void JWLineModel::AddEnd() noexcept
{
	// Create vertex buffer
	m_pDX->CreateDynamicVertexBuffer(m_VertexData.GetByteSize(), m_VertexData.GetPtrData(), &m_VertexBuffer);

	// Create index buffer
	m_pDX->CreateIndexBuffer(m_IndexData.GetByteSize(), m_IndexData.GetPtrData(), &m_IndexBuffer);
}

auto JWLineModel::SetLine3DPosition(size_t Line3DIndex, XMFLOAT3 StartPosition, XMFLOAT3 EndPosition) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line3D)
		{
			Line3DIndex = min(Line3DIndex, m_VertexData.GetCount() / 2 - 1);

			m_VertexData.vVertices[Line3DIndex * 2].Position = StartPosition;

			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position = EndPosition;
		}
	}

	return this;
}

auto JWLineModel::SetLine3DPosition(size_t Line3DIndex, XMFLOAT3 StartPosition, XMFLOAT3 EndPosition, XMFLOAT4 Color) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line3D)
		{
			Line3DIndex = min(Line3DIndex, m_VertexData.GetCount() / 2 - 1);

			m_VertexData.vVertices[Line3DIndex * 2].Position = StartPosition;
			m_VertexData.vVertices[Line3DIndex * 2].ColorDiffuse = Color;

			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position = EndPosition;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].ColorDiffuse = Color;
		}
	}

	return this;
}

auto JWLineModel::SetLine3DOriginDirection(size_t Line3DIndex, XMVECTOR Origin, XMVECTOR Direction) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line3D)
		{
			constexpr float line_length = 100.0f;

			Line3DIndex = min(Line3DIndex, m_VertexData.GetCount() / 2 - 1);

			m_VertexData.vVertices[Line3DIndex * 2].Position.x = XMVectorGetX(Origin);
			m_VertexData.vVertices[Line3DIndex * 2].Position.y = XMVectorGetY(Origin);
			m_VertexData.vVertices[Line3DIndex * 2].Position.z = XMVectorGetZ(Origin);

			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.x =
				m_VertexData.vVertices[Line3DIndex * 2].Position.x + XMVectorGetX(Direction) * line_length;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.y =
				m_VertexData.vVertices[Line3DIndex * 2].Position.y + XMVectorGetY(Direction) * line_length;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.z =
				m_VertexData.vVertices[Line3DIndex * 2].Position.z + XMVectorGetZ(Direction) * line_length;
		}
	}

	return this;
}

auto JWLineModel::SetLine3DOriginDirection(size_t Line3DIndex, XMVECTOR Origin, XMVECTOR Direction, XMFLOAT4 Color) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line3D)
		{
			constexpr float line_length = 100.0f;

			Line3DIndex = min(Line3DIndex, m_VertexData.GetCount() / 2 - 1);

			m_VertexData.vVertices[Line3DIndex * 2].Position.x = XMVectorGetX(Origin);
			m_VertexData.vVertices[Line3DIndex * 2].Position.y = XMVectorGetY(Origin);
			m_VertexData.vVertices[Line3DIndex * 2].Position.z = XMVectorGetZ(Origin);
			m_VertexData.vVertices[Line3DIndex * 2].ColorDiffuse = Color;

			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.x =
				m_VertexData.vVertices[Line3DIndex * 2].Position.x + XMVectorGetX(Direction) * line_length;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.y = 
				m_VertexData.vVertices[Line3DIndex * 2].Position.y + XMVectorGetY(Direction) * line_length;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].Position.z = 
				m_VertexData.vVertices[Line3DIndex * 2].Position.z + XMVectorGetZ(Direction) * line_length;
			m_VertexData.vVertices[Line3DIndex * 2 + 1].ColorDiffuse = Color;
		}
	}

	return this;
}

auto JWLineModel::SetLine2D(size_t Line2DIndex, XMFLOAT2 StartPosition, XMFLOAT2 Length) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line2D)
		{
			Line2DIndex = min(Line2DIndex, m_VertexData.GetCount() / 2 - 1);

			float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
			float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

			XMFLOAT3 position_a = XMFLOAT3(-window_width / 2 + StartPosition.x, window_height / 2 - StartPosition.y, 0);
			XMFLOAT3 position_b = position_a;
			position_b.x += Length.x;
			position_b.y += Length.y;

			m_VertexData.vVertices[Line2DIndex * 2].Position = position_a;

			m_VertexData.vVertices[Line2DIndex * 2 + 1].Position = position_b;
		}
	}

	return this;
}

auto JWLineModel::SetLine2D(size_t Line2DIndex, XMFLOAT2 StartPosition, XMFLOAT2 Length, XMFLOAT4 Color) noexcept->JWLineModel*
{
	if (m_VertexData.GetCount())
	{
		if (m_RenderType == ERenderType::Model_Line2D)
		{
			Line2DIndex = min(Line2DIndex, m_VertexData.GetCount() / 2 - 1);

			float window_width = static_cast<float>(m_pDX->GetWindowSize().Width);
			float window_height = static_cast<float>(m_pDX->GetWindowSize().Height);

			XMFLOAT3 position_a = XMFLOAT3(-window_width / 2 + StartPosition.x, window_height / 2 - StartPosition.y, 0);
			XMFLOAT3 position_b = position_a;
			position_b.x += Length.x;
			position_b.y += Length.y;

			m_VertexData.vVertices[Line2DIndex * 2].Position = position_a;
			m_VertexData.vVertices[Line2DIndex * 2].ColorDiffuse = Color;

			m_VertexData.vVertices[Line2DIndex * 2 + 1].Position = position_b;
			m_VertexData.vVertices[Line2DIndex * 2 + 1].ColorDiffuse = Color;
		}
	}

	return this;
}

void JWLineModel::UpdateLines() noexcept
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	if (SUCCEEDED(m_pDX->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource)))
	{
		memcpy(mapped_subresource.pData, m_VertexData.GetPtrData(), m_VertexData.GetByteSize());

		m_pDX->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}
}