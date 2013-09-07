#ifndef EFU_SIMPLEREADHKLMKEY_H
#define EFU_SIMPLEREADHKLMKEY_H

#include <windows.h>
#include <string>

class SimpleReadHKLMKey
{
public:
    explicit SimpleReadHKLMKey(const std::string &location, const std::string &name):
        m_hkey(nullptr),
        m_val()
    {
        LONG ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            location.c_str(),
            0, KEY_READ | KEY_WOW64_32KEY, &m_hkey);
        if (ret == ERROR_SUCCESS)
        {
            CHAR szBuffer[128];
            DWORD dwBufferSize = sizeof(szBuffer);
            ret = ::RegQueryValueEx(m_hkey,
                name.c_str(),
                0, nullptr,
                reinterpret_cast<LPBYTE>(szBuffer),
                &dwBufferSize);
            
            // RegQueryValueEx() does not guarantee the string is
            // null-terminated.
            szBuffer[dwBufferSize - 1] = '\0';
            
            if (ret == ERROR_SUCCESS)
            {
                m_val = szBuffer;
            }
        }
    }
    
    const std::string str() const { return m_val; }
    
    ~SimpleReadHKLMKey()
    {
        if (m_hkey)
        {
            ::RegCloseKey(m_hkey);
        }
    }
    
    bool good() const { return !m_val.empty(); }
    
private:
    HKEY m_hkey;
    std::string m_val;
};

#endif //EFU_SIMPLEREADHKLMKEY_H