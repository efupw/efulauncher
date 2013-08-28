#ifndef EFU_WINERRORSTRING_H
#define EFU_WINERRORSTRING_H

#include <windows.h>
#include <string>

class WinErrorString
{
public:
    explicit WinErrorString():
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
        
    const DWORD code() const { return m_code; }
    const std::string str() const { return m_what; }
    
    ~WinErrorString()
    {
        if (m_error_text)
        {
            ::LocalFree(m_error_text);
            m_error_text = nullptr;
        }
    }
    
private:
    DWORD m_code;
    LPTSTR m_error_text;
    std::string m_what;
};

#endif //EFU_WINERRORSTRING_H