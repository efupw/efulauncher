#ifndef EFU_TARGET_H
#define EFU_TARGET_H

#include <vector>
#include <string>

const std::string patch_dir("http://nwn.efupw.com/rootdir/patch/");

typedef std::vector<std::string> strvec;

strvec &split(const std::string &s, char delim, strvec &elems);
strvec split(const std::string &s, char delim);

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
        
        explicit Target(const std::string &dlpath,
            const std::string &name, const std::string &checksum);

        const std::string name() const { return m_name; }
        const std::string checksum() const { return m_checksum; }
        void fetch() const;
        Status status() const;

    private:
        std::string m_dlpath;
        std::string m_name;
        std::string m_checksum;

        const std::string dlpath() const { return m_dlpath; }
        void do_fetch() const;
};

#endif //EFU_TARGET_H