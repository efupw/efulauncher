#ifndef EFU_SIMPLEREADHKLMKEY_H
#define EFU_SIMPLEREADHKLMKEY_H

#include <windows.h>
#include <string>

class SimpleReadHKLMKey
{
public:
    explicit SimpleReadHKLMKey(const std::string &location,
        const std::string &name);
    
    ~SimpleReadHKLMKey();
    
    const std::string str() const { return m_val; }
    bool good() const { return !m_val.empty(); }
    
private:
    HKEY m_hkey;
    std::string m_val;
};

#endif //EFU_SIMPLEREADHKLMKEY_H