#ifndef EFU_SIMPLEREADHKLMKEY_H
#include "simple_read_hklm_key.h"
#endif

SimpleReadHKLMKey::SimpleReadHKLMKey(const std::string &location,
    const std::string &name):
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

SimpleReadHKLMKey::~SimpleReadHKLMKey()
{
    if (m_hkey)
    {
        ::RegCloseKey(m_hkey);
    }
}
