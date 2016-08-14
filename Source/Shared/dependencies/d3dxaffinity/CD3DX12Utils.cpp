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

#include <locale>
#include <codecvt>
#include <string>
#include <comdef.h>

D3DX12AF_API void WriteHRESULTError(HRESULT const hr)
{
    _com_error err(hr, nullptr);
    DebugLog("HRESULT Failure: %d 0x%08X %s\n", hr, hr, err.ErrorMessage());
}
