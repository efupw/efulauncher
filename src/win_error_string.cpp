#ifndef EFU_WINERRORSTRING_H
#include "win_error_string.h"
#endif

WinErrorString::WinErrorString():
    m_code(::GetLastError()),
    m_error_text(nullptr),
    m_what()
{
    DWORD buf_len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        m_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&m_error_text),
        0,
        nullptr);
    
    if (buf_len)
    {
        m_what = std::string(m_error_text, m_error_text + buf_len);
    }
}

WinErrorString::~WinErrorString()
{
    if (m_error_text)
    {
        ::LocalFree(m_error_text);
        m_error_text = nullptr;
    }
}
