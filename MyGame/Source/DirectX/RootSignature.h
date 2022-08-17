#pragma once

#include "DirectXImpl.h"

namespace MyGame
{
	class RootParameter
	{
		friend class RootSignature;

	public:
		RootParameter() { m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF; }
		~RootParameter() { Clear(); }

		void Clear()
		{
			if (m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				delete[] m_RootParam.DescriptorTable.pDescriptorRanges;
			m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
		}

		void InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
		{
			m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			m_RootParam.ShaderVisibility = Visibility;
			m_RootParam.Constants.Num32BitValues = NumDwords;
			m_RootParam.Constants.ShaderRegister = Register;
			m_RootParam.Constants.RegisterSpace = Space;
		}

		void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
		{
			m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			m_RootParam.ShaderVisibility = Visibility;
			m_RootParam.Descriptor.ShaderRegister = Register;
			m_RootParam.Descriptor.RegisterSpace = Space;
		}

		void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
		{
			m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			m_RootParam.ShaderVisibility = Visibility;
			m_RootParam.Descriptor.ShaderRegister = Register;
			m_RootParam.Descriptor.RegisterSpace = Space;
		}

		void InitAsBufferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
		{
			m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			m_RootParam.ShaderVisibility = Visibility;
			m_RootParam.Descriptor.ShaderRegister = Register;
			m_RootParam.Descriptor.RegisterSpace = Space;
		}

		void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
		{
			InitAsDescriptorTable(1, Visibility);
			SetTableRange(0, Type, Register, Count, Space);
		}

		void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			m_RootParam.ShaderVisibility = Visibility;
			m_RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
			m_RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
		}

		void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space = 0)
		{
			D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
			range->RangeType = Type;
			range->NumDescriptors = Count;
			range->BaseShaderRegister = Register;
			range->RegisterSpace = Space;
			range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

		const D3D12_ROOT_PARAMETER& operator() () const { return m_RootParam; }

	protected:
		D3D12_ROOT_PARAMETER m_RootParam;
	};


	class RootSignature : public DirectXImpl
	{
	public:
		RootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0) : m_Finalized(FALSE), m_NumParameters(NumRootParams) { Reset(NumRootParams, NumStaticSamplers); }

		void Reset(UINT NumRootParams, UINT NumStaticSamplers = 0)
		{
			if (NumRootParams > 0)
				m_ParamArray.reset(new RootParameter[NumRootParams]);
			else
				m_ParamArray = nullptr;
			m_NumParameters = NumRootParams;

			if (NumStaticSamplers > 0)
				m_SamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
			else
				m_SamplerArray = nullptr;
			m_NumSamplers = NumStaticSamplers;
			m_NumInitializedStaticSamplers = 0;
		}

		RootParameter& operator[] (UINT EntryIndex)
		{
			DirectX::ThrowIfFailed(EntryIndex < m_NumParameters);
			return m_ParamArray.get()[EntryIndex];
		}

		const RootParameter& operator[] (UINT EntryIndex) const
		{
			DirectX::ThrowIfFailed(EntryIndex < m_NumParameters);
			return m_ParamArray.get()[EntryIndex];
		}

		void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc, D3D12_SHADER_VISIBILITY Visibility);
		void Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);
		ID3D12RootSignature* GetSignature() const { return m_Signature; }

	private:
		bool m_Finalized;
		UINT m_NumParameters;
		UINT m_NumSamplers;
		UINT m_NumInitializedStaticSamplers;
		uint32_t m_DescriptorTableBitMap;
		uint32_t m_SamplerTableBitMap;
		uint32_t m_DescriptorTableSize[16];
		std::unique_ptr<RootParameter[]> m_ParamArray;
		std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> m_SamplerArray;
		ID3D12RootSignature* m_Signature;
	};
}