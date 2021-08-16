#pragma once

#include <exception>
#include <string>

class AppException : public std::exception
{
public:
    AppException(std::string msg);

    wchar_t const* GetErrorMsgWide() const;
};