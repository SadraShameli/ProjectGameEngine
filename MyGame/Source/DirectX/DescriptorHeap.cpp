#include "CommonHeaders.h"

#include "DescriptorHeap.h"
#include "CommandList.h"
#include "DirectXImpl.h"

#include "../Debugs/DebugHelpers.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace MyGame
{
	std::mutex DescriptorAllocator::sm_AllocationMutex;
	std::vector<ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_DescriptorHeapPool;

	void DescriptorAllocator::DestroyAll(void) { sm_DescriptorHeapPool.clear(); }

	ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
	{
		std::lock_guard<std::mutex> LockGuard(sm_AllocationMutex);

		D3D12_DESCRIPTOR_HEAP_DESC Desc;
		Desc.Type = Type;
		Desc.NumDescriptors = sm_NumDescriptorsPerHeap;
		Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ComPtr<ID3D12DescriptorHeap> pHeap;
		ThrowIfFailed(DirectXImpl::m_device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
		sm_DescriptorHeapPool.emplace_back(pHeap);
		return pHeap.Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t Count)
	{
		if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < Count)
		{
			m_CurrentHeap = RequestNewHeap(m_Type);
			m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
			m_RemainingFreeHandles = sm_NumDescriptorsPerHeap;

			if (m_DescriptorSize == 0)
				m_DescriptorSize = DirectXImpl::m_device->GetDescriptorHandleIncrementSize(m_Type);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
		m_CurrentHandle.ptr += Count * m_DescriptorSize;
		m_RemainingFreeHandles -= Count;
		return ret;
	}

	void DescriptorHeap::Create(const std::wstring_view& Name, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount)
	{
		m_HeapDesc.Type = Type;
		m_HeapDesc.NumDescriptors = MaxCount;
		m_HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(DirectXImpl::m_device->CreateDescriptorHeap(&m_HeapDesc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())));

#ifdef MYGAME_DEBUG
		m_Heap->SetName(Name.data());
#endif

		m_DescriptorSize = DirectXImpl::m_device->GetDescriptorHandleIncrementSize(m_HeapDesc.Type);
		m_NumFreeDescriptors = m_HeapDesc.NumDescriptors;
		m_FirstHandle = DescriptorHandle(m_Heap->GetCPUDescriptorHandleForHeapStart(), m_Heap->GetGPUDescriptorHandleForHeapStart());
		m_NextFreeHandle = m_FirstHandle;
	}

	DescriptorHandle DescriptorHeap::Alloc(uint32_t Count)
	{
		MYGAME_ASSERT(HasAvailableSpace(Count), "Descriptor Heap out of space.  Increase heap size.");
		DescriptorHandle ret = m_NextFreeHandle;
		m_NextFreeHandle += Count * m_DescriptorSize;
		m_NumFreeDescriptors -= Count;
		return ret;
	}

	bool DescriptorHeap::ValidateHandle(const DescriptorHandle& DHandle) const
	{
		if (DHandle.GetCpuPtr() < m_FirstHandle.GetCpuPtr() || DHandle.GetCpuPtr() >= m_FirstHandle.GetCpuPtr() + m_HeapDesc.NumDescriptors * m_DescriptorSize)
			return false;

		if (DHandle.GetGpuPtr() - m_FirstHandle.GetGpuPtr() != DHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr())
			return false;
		return true;
	}
}