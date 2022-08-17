#include "CommonHeaders.h"

#include "GpuBuffer.h"
#include "DirectXImpl.h"
#include "CommandContext.h"

#include "../Utilities/CommonMath.h"
#include "../Debugs/DebugHelpers.h"

using namespace DirectX;

namespace MyGame
{
	void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData)
	{
		Destroy();

		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;

		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;
		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
		ThrowIfFailed(DirectXImpl::D3D12_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
		NAME_D3D12_OBJ_STR(m_pResource, name.data());

		if (initialData)
			CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);
		CreateDerivedViews();
	}

	void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& srcData, uint32_t srcOffset)
	{
		Destroy();

		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;

		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;
		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
		ThrowIfFailed(DirectXImpl::D3D12_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
		NAME_D3D12_OBJ_STR(m_pResource, name.data());

		CommandContext::InitializeBuffer(*this, srcData, srcOffset);
		CreateDerivedViews();
	}

	void GpuBuffer::CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
		const void* initialData)
	{
		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		ThrowIfFailed(DirectXImpl::D3D12_Device->CreatePlacedResource(pBackingHeap, HeapOffset, &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
		NAME_D3D12_OBJ_STR(m_pResource, name.data());

		if (initialData)
			CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);
		CreateDerivedViews();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t Offset, uint32_t Size) const
	{
		MYGAME_ASSERT(Offset + Size <= m_BufferSize);

		Size = Math::AlignUp(Size, 16);
		D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
		CBVDesc.BufferLocation = m_GpuVirtualAddress + (size_t)Offset;
		CBVDesc.SizeInBytes = Size;

		D3D12_CPU_DESCRIPTOR_HANDLE hCBV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateConstantBufferView(&CBVDesc, hCBV);
		return hCBV;
	}

	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
	{
		MYGAME_ASSERT(m_BufferSize != 0);

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		Desc.Flags = m_ResourceFlags;
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_BufferSize;
		return Desc;
	}

	void ByteAddressBuffer::CreateDerivedViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

		if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_SRV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateShaderResourceView(m_pResource, &SRVDesc, m_SRV);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		UAVDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

		if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_UAV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateUnorderedAccessView(m_pResource, nullptr, &UAVDesc, m_UAV);
	}

	void StructuredBuffer::CreateDerivedViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = m_ElementCount;
		SRVDesc.Buffer.StructureByteStride = m_ElementSize;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_SRV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateShaderResourceView(m_pResource, &SRVDesc, m_SRV);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.Buffer.CounterOffsetInBytes = 0;
		UAVDesc.Buffer.NumElements = m_ElementCount;
		UAVDesc.Buffer.StructureByteStride = m_ElementSize;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		m_CounterBuffer.Create(L"StructuredBuffer::Counter", 1, 4);

		if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_UAV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateUnorderedAccessView(m_pResource, m_CounterBuffer.GetResource(), &UAVDesc, m_UAV);
	}

	void TypedBuffer::CreateDerivedViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = m_DataFormat;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = m_ElementCount;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_SRV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateShaderResourceView(m_pResource, &SRVDesc, m_SRV);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = m_DataFormat;
		UAVDesc.Buffer.NumElements = m_ElementCount;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_UAV = DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DirectXImpl::D3D12_Device->CreateUnorderedAccessView(m_pResource, nullptr, &UAVDesc, m_UAV);
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& Context)
	{
		Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		return m_CounterBuffer.GetSRV();
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& Context)
	{
		Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		return m_CounterBuffer.GetUAV();
	}

	void ReadbackBuffer::Create(std::wstring&& name, uint32_t NumElements, uint32_t ElementSize)
	{
		Destroy();

		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;
		m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC ResourceDesc = {};
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResourceDesc.Width = m_BufferSize;
		ResourceDesc.Height = 1;
		ResourceDesc.DepthOrArraySize = 1;
		ResourceDesc.MipLevels = 1;
		ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResourceDesc.SampleDesc.Count = 1;
		ResourceDesc.SampleDesc.Quality = 0;
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(DirectXImpl::D3D12_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
		NAME_D3D12_OBJ_STR(m_pResource, name);
	}

	void* ReadbackBuffer::Map(void)
	{
		void* Memory;
		auto range = CD3DX12_RANGE(0, m_BufferSize);
		m_pResource->Map(0, &range, &Memory);
		return Memory;
	}

	void ReadbackBuffer::Unmap(void) { auto range = CD3DX12_RANGE(0, 0); m_pResource->Unmap(0, &range); }
}