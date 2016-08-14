//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "d3dx12affinity.h"
#include "CD3DX12Utils.h"

HRESULT STDMETHODCALLTYPE CD3DX12AffinityObject::GetPrivateData(
	REFGUID guid,
	UINT* pDataSize,
	void* pData,
	UINT AffinityMask)
{
	HRESULT hr = S_OK;
	AffinityMask &= GetNodeMask();
	
	for (int AffinityIndex = 0; AffinityIndex < D3DX12_MAX_ACTIVE_NODES; AffinityIndex++)
	{
		if ((AffinityMask & AffinityIndexToMask(AffinityIndex)) == 0 || mObjects[AffinityIndex] == nullptr) // skip if affinity index is not in range or object is null
			continue;

		if (IsD3D())
		{
			hr |= ((ID3D12Object*)mObjects[AffinityIndex])->GetPrivateData(guid, pDataSize, pData);
		}
		else
		{
			hr |= ((IDXGIObject*)mObjects[AffinityIndex])->GetPrivateData(guid, pDataSize, pData);
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityObject::SetPrivateData(
	REFGUID guid,
	UINT DataSize,
	const void* pData,
	UINT AffinityMask)
{
	HRESULT hr = S_OK;
	AffinityMask &= GetNodeMask();
	

	for (int AffinityIndex = 0; AffinityIndex < D3DX12_MAX_ACTIVE_NODES; AffinityIndex++)
	{
		if ((AffinityMask & AffinityIndexToMask(AffinityIndex)) == 0 || mObjects[AffinityIndex] == nullptr) // skip if affinity index is not in range or object is null
			continue;

		if (IsD3D())
		{
			hr |= ((ID3D12Object*)mObjects[AffinityIndex])->SetPrivateData(guid, DataSize, pData);
		}
		else
		{
			hr |= ((IDXGIObject*)mObjects[AffinityIndex])->SetPrivateData(guid, DataSize, pData);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityObject::SetPrivateDataInterface(
	REFGUID guid,
	const IUnknown* pData,
	UINT AffinityMask)
{
	HRESULT hr = S_OK;
	AffinityMask &= GetNodeMask();

	for (int AffinityIndex = 0; AffinityIndex < D3DX12_MAX_ACTIVE_NODES; AffinityIndex++)
	{
		if ((AffinityMask & AffinityIndexToMask(AffinityIndex)) == 0 || mObjects[AffinityIndex] == nullptr) // skip if affinity index is not in range or object is null
			continue;

		if (IsD3D())
		{
			hr |= ((ID3D12Object*)mObjects[AffinityIndex])->SetPrivateDataInterface(guid, pData);
		}
		else
		{
			hr |= ((IDXGIObject*)mObjects[AffinityIndex])->SetPrivateDataInterface(guid, pData);
		}
	}

	return hr;

}

HRESULT STDMETHODCALLTYPE CD3DX12AffinityObject::SetName(
	LPCWSTR Name,
	UINT AffinityMask)
{
	HRESULT hr = S_OK;
	AffinityMask &= GetNodeMask();

	for (int AffinityIndex = 0; AffinityIndex < D3DX12_MAX_ACTIVE_NODES; AffinityIndex++)
	{
		if ((AffinityMask & AffinityIndexToMask(AffinityIndex) ) == 0 || mObjects[AffinityIndex] == nullptr) // skip if affinity index is not in range
			continue;

		if (IsD3D())
		{
			hr |= ((ID3D12Object*)mObjects[AffinityIndex])->SetName(Name);
		}
	}
#ifdef DEBUG_OBJECT_NAME
	mObjectDebugName = Name;
#endif
	return hr;
}

void STDMETHODCALLTYPE CD3DX12AffinityObject::SetAffinity(
	UINT AffinityMask)
{
	mAffinityMask = AffinityMask & GetNodeMask();

}

UINT CD3DX12AffinityObject::AffinityIndexToMask(UINT const Index)
{
	return 1 << Index;
}

CD3DX12AffinityObject::CD3DX12AffinityObject(CD3DX12AffinityDevice* device, IUnknown** objects, UINT Count)
{
	for (UINT i = 0; i < D3DX12_MAX_ACTIVE_NODES; i++)
	{
		if (i < Count)
		{
			mObjects[i] = objects[i];
		}
		else
		{
			mObjects[i] = nullptr;
		}
	}
#ifdef DEBUG_OBJECT_NAME
	mObjectTypeName = L"Object";
#endif
	mReferenceCount = 1;
	mParentDevice = device;
	SetAffinity(mParentDevice->GetNodeMask());
}

CD3DX12AffinityObject::~CD3DX12AffinityObject()
{
	for (size_t i = 0; i < D3DX12_MAX_ACTIVE_NODES; ++i)
	{
		if (mObjects[i])
		{
			ULONG RefCount = mObjects[i]->Release();
			DebugLog("Releasing handle 0x%p, ref count is now %lu\n", mObjects[i], RefCount);
		}
	}

}

ULONG CD3DX12AffinityObject::AddRef()
{
	InterlockedIncrement(&mReferenceCount);
	return mReferenceCount;
}

ULONG CD3DX12AffinityObject::Release()
{
	ULONG currentRefCount = InterlockedDecrement(&mReferenceCount);
#ifdef DEBUG_OBJECT_NAME
	DebugLog(L"Release() called on an affinity object, reference count now %u [%p] [%s] [%s]\n", mReferenceCount, this, mObjectTypeName.c_str(), mObjectDebugName.c_str());
#endif
	if (mReferenceCount == 0)
	{
		delete this;
	}

	return currentRefCount;
}

CD3DX12AffinityDevice* CD3DX12AffinityObject::GetParentDevice()
{
	return mParentDevice;
}

UINT CD3DX12AffinityObject::GetNodeMask()
{
	return CD3DX12AffinityDevice::g_CachedNodeMask;
}

UINT CD3DX12AffinityObject::GetNodeCount()
{
	return CD3DX12AffinityDevice::g_CachedNodeCount;
}

UINT CD3DX12AffinityObject::GetActiveNodeIndex()
{
	return CD3DX12AffinityDevice::g_ActiveNodeIndex;
}
