#include <windows.h>

#include <string>
#include <vector>
#include <iostream>

LONG ReadRegKey(const std::wstring& keyPath, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue);

int main(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
		args.push_back(argv[i]);

	if (args.size() >= 2 && args[1] == "--windows-sdk")
	{
		std::wstring windowsSDKPath;
		ReadRegKey(L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots", L"KitsRoot10", windowsSDKPath, L"");
		std::wcout << windowsSDKPath;
	}
}

LONG ReadRegKey(const std::wstring& keyPath, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
{
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey);
	bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
	bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
	std::wstring strValueOfBinDir;
	std::wstring strKeyDefaultValue;

	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}

	RegCloseKey(hKey);

	return nError;
}