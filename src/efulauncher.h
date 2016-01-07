#ifndef EFU_EFULAUNCHER_H
#define EFU_EFULAUNCHER_H

#include <string>

class EfuLauncher
{
    public:
        explicit EfuLauncher(const std::string &path,
                const std::string &update_check,
                const std::string &version);
        
        EfuLauncher(const EfuLauncher&) = default;
        EfuLauncher& operator=(const EfuLauncher&) = delete;

        bool has_update();
        bool get_update();
        void stat_targets();
        
    private:
        const std::string path() const { return m_path; }

        const std::string m_path;
        const std::string m_update_check;
        std::string m_update_path;
        std::string m_version;
        bool m_has_update;
};
#endif //EFU_EFULAUNCHER_H
