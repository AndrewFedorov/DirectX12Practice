#include "AppException.h"
#include <string>
#include <stdlib.h>

AppException::AppException(std::string msg): std::exception(msg.c_str()) {}

wchar_t const* AppException::GetErrorMsgWide() const
{
    char const* msg = what();
    const size_t size = strlen(msg) + 1;
    wchar_t* wMsg = new wchar_t[size];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wMsg, size, msg, _TRUNCATE);
    return wMsg;
}