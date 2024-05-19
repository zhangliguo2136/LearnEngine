#pragma once
#include <string>
#include <winerror.h>

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::string& filename, int lineNumber);
	std::string ToString()const;

public:
	HRESULT ErrorCode = S_OK;
	std::string FunctionName;
	std::string Filename;
	int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)													\
{																			\
    HRESULT hr__ = (x);														\
    if(FAILED(hr__)) { throw DxException(hr__, L#x, __FILE__, __LINE__); }	\
}
#endif