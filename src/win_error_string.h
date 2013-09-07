#ifndef EFU_WINERRORSTRING_H
#define EFU_WINERRORSTRING_H

#include <windows.h>
#include <string>

class WinErrorString
{
public:
    explicit WinErrorString();
        
    const DWORD code() const { return m_code; }
    const std::string str() const { return m_what; }
    
    ~WinErrorString();

private:
    DWORD m_code;
    LPTSTR m_error_text;
    std::string m_what;
};

#endif //EFU_WINERRORSTRING_H