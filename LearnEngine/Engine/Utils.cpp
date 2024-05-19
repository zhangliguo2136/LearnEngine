#include "Utils.h"
#include <comdef.h>
#include "FormatConvert.h"

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::string& filename, int lineNumber)
{
	ErrorCode = hr;
	FunctionName = TFormatConvert::WStrToStr(functionName);
	Filename = filename;
	LineNumber = lineNumber;
}

std::string DxException::ToString() const
{
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + " failed in " + Filename + "; line " + std::to_string(LineNumber) + "; error: " + TFormatConvert::WStrToStr(msg);
}
