#ifndef EFU_TARGET_H
#define EFU_TARGET_H

#include <vector>
#include <string>

const std::string patch_dir("http://nwn.efupw.com/rootdir/patch/");

std::vector<std::string> &split(const std::string &s, char delim,
        std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

const std::string file_checksum(const std::string &path);

#ifdef CPP11_ENUM_CLASS
#define ENUM_CLASS enum class
#else
#define ENUM_CLASS enum
#endif
class Target
{
    public:
        ENUM_CLASS Status {
            Nonexistent,
            Outdated,
            Current
        };
        
        explicit Target(const std::string &name, const std::string &checksum);

        const std::string name() const { return m_name; }
        const std::string checksum() const { return m_checksum; }
        void fetch() const;
        Status status() const;

    private:
        std::string m_name;
        std::string m_checksum;

        void do_fetch() const;
};

#endif //EFU_TARGET_H