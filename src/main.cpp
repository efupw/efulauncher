#include <limits>
#include <algorithm>
#include <memory>
//#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <system_error>
#include <iostream>
#include <fstream>

/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
*/

#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

#ifdef _WIN32

#ifndef EFU_SIMPLEREADHKLMKEY_H
#include "simple_read_hklm_key.h"
#endif

#endif

#ifndef EFU_TARGET_H
#include "target.h"
#endif

const std::string version("0.1.0");
const std::string listing("http://nwn.efupw.com/rootdir/index.dat");

namespace Options
{
    bool version(const std::string &val)
    {
        return val == "version";
    }
    bool update_path(const std::string &val)
    {
        return val == "update path";
    }
};

bool confirm()
{
    char c;
    do
    {
        std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        c = static_cast<char>(tolower(c));
    } while (c != 'y' && c != 'n');
    return c == 'y';
}

class EfuLauncher
{
    public:
    // TODO: assignment, copy operator
        explicit EfuLauncher(const std::string path,
                const std::string update_check):
            m_path(path),
            m_update_check(update_check),
            m_has_update(false)
    {}

        bool has_update()
        {
            if (m_has_update)
            {
                return m_has_update;
            }

            std::string fetch;
            CurlEasy curl(m_update_check.c_str());
            curl.write_to(fetch);
            //TODO
            try
            {
                curl.perform();
            }
            catch (CurlEasyException &e)
            {
                std::cout << e.what() << std::endl;
            }

            std::vector<std::string> lines(split(fetch, '\n'));
            fetch.clear();
            for (auto beg = std::begin(lines), end = std::end(lines);
                    beg != end; ++beg)
            {
                auto keyvals(split(*beg, '='));
                if (keyvals.size() != 2)
                {
                    std::cerr << "Malformed option: " + *beg +
                        ", aborting launcher update check." << std::endl;
                    return m_has_update = false;
                }
                if (Options::version(keyvals[0]))
                {
                    const std::string version_test(keyvals[1]);
                    m_has_update = version_test != version;
                }
                else if (Options::update_path(keyvals[0]))
                {
                    m_update_path = keyvals[1];
                }

            }
            return m_has_update;
        }

        bool get_update()
        {
            if (!m_has_update || m_update_path.empty())
            {
                return m_has_update = false;
            }
            return !(m_has_update = false);
        }

        void stat_targets()
        {
            std::string fetch;
            CurlEasy curl(listing);
            curl.write_to(fetch);
            curl.perform();

            auto lines(split(fetch, '\n'));
            std::vector<Target> new_targets, old_targets;
            for (auto beg = std::begin(lines), end = std::end(lines);
                    beg != end; ++beg)
            {
                auto data(split(*beg, '@'));
                Target t(data[0], data[data.size() - 1]);
                auto status = t.status();
// TODO
#ifdef CPP11_ENUM_CLASS
                if (status == Target::Status::Nonexistent)
#else
                if (status == Target::Nonexistent)
#endif
                {
                    new_targets.push_back(std::move(t));
                }
// TODO
#ifdef CPP11_ENUM_CLASS
                else if (status == Target::Status::Outdated)
#else
                else if (status == Target::Outdated)
#endif
                {
                    old_targets.push_back(std::move(t));
                }
            }
            if (new_targets.size())
            {
                std::cout << "New targets: " << new_targets.size() << std::endl;
#ifdef CPP11_FOR_EACH
                for (auto &t : new_targets)
#else
                std::for_each(new_targets.cbegin(), new_targets.cend(),
                    [](const Target &t)
#endif
                {
                    std::cout << "- " << t.name() << std::endl;
                }
#ifndef CPP11_FOR_EACH
                );
#endif
            }
            else
            {
                std::cout << "No new targets." << std::endl;
            }

            if (old_targets.size())
            {
                std::cout << "Outdated targets: " << old_targets.size() << std::endl;
#ifdef CPP11_FOR_EACH
                for (auto &t : old_targets)
#else
                std::for_each(old_targets.cbegin(), old_targets.cend(),
                    [](const Target &t)
#endif
                {
                    std::cout << "- " << t.name() << std::endl;
                }
#ifndef CPP11_FOR_EACH
                );
#endif
            }
            else
            {
                std::cout << "No targets out of date." << std::endl;
            }

#ifndef DEBUG
#ifdef CPP11_FOR_EACH
            for (auto &t : new_targets)
#else
            std::for_each(new_targets.cbegin(), new_targets.cend(),
                [](const Target &t)
#endif
            {
                t.fetch();
            }
#ifndef CPP11_FOR_EACH
            );

            std::for_each(old_targets.cbegin(), old_targets.cend(),
                [](const Target &t)
#else
            for (auto &t : old_targets)
#endif
            {
                t.fetch();
            }
#ifndef CPP11_FOR_EACH
            );
#endif
#endif
        }
        
    private:
        const std::string path() const { return m_path; }

        const std::string m_path;
        const std::string m_update_check;
        std::string m_update_path;
        bool m_has_update;
};

int main(int, char *argv[])
{
    CurlGlobalInit curl_global;

#ifdef _WIN32
    std::string nwn_bin("nwmain.exe");
    std::string nwn_root_dir("./");
    std::ifstream nwn(nwn_root_dir + nwn_bin, std::ios::binary);
    if (!nwn)
    {
        std::cout << "Current launcher directory not detected as NWN root"\
        " directory.";
        nwn_root_dir = "C:/NeverwinterNights/NWN/";
        std::cout << "\nTrying " << nwn_root_dir << "... ";
        nwn.open(nwn_root_dir + nwn_bin, std::ios::binary);

        if (!nwn)
        {
            std::cout << "not found.\nSearching registry... ";
            SimpleReadHKLMKey reg("SOFTWARE\\BioWare\\NWN\\Neverwinter",
                "Location");
            if (reg.good())
            {
                nwn_root_dir = reg.str();
                std::cout << "key found.";
                auto path_sep = nwn_root_dir.at(nwn_root_dir.size() - 1);
                if (path_sep != '/' && path_sep != '\\')
                {
                    nwn_root_dir.append("/");
                }
                std::cout << "\nTrying " << nwn_root_dir << "... ";
                nwn.open(nwn_root_dir + nwn_bin, std::ios::binary);
                if (!nwn)
                {
                    std::cout << "not found.";
                }
            }
            else
            {
                std::cout << "no key found.";
            }
        }
    }
    if (nwn)
    {
        std::cout << nwn_bin << " found." << std::endl;
    }
    else
    {
        std::cout << "\n\nNWN root directory not found, known"\
            " options exhausted."\
            "\nThe launcher will not be able to download files to the correct"\
            " location or"\
            "\nlaunch Neverwinter Nights, however, the launcher"\
            " may still download files to"\
            "\nthe current directory and you can"\
            " move them manually afterwards. To avoid this"\
            "\nin the future"\
            " either move the launcher to the NWN root directory containing"\
            "\nnwmain.exe or pass the -nwn=C:/NeverwinterNights/NWN flag to"\
            " the launcher"\
            "\nexecutable, substituting the correct path, quoted"\
            " if it contains spaces:"\
            "\n\t-nwn=\"X:/Games/Neverwinter Nights/NWN\"."\
            "\n/ and \\ are interchangeable."\
            "\nWould you like to download files anyway (y/n)?" << std::endl;
        if (!confirm())
        {
            std::cout << "Exiting EfU Launcher. Goodbye!" << std::endl;
            return 0;
        }
    }
#endif

    EfuLauncher l(argv[0],
            "https://raw.github.com/commonquail/efulauncher/"\
            "master/versioncheck");

    if (l.has_update())
    {
        std::cout << "A new version of the launcher is available."\
            " Would you like to download it (y/n)?" << std::endl;
        bool download(confirm());
        if (!download)
        {
            std::cout << "It is strongly recommended to always use"\
                " the latest launcher. Would you like to download it (y/n)?"
                << std::endl;
            download = confirm();
        }
        if (download)
        {
            // Download.
            std::cout << "Downloading new launcher..." << std::endl;
            if (l.get_update())
            {
                std::cout << "Done. Please extract and run the new launcher." << std::endl;
            }
            return 0;
        }
    }

    try
    {
        l.stat_targets();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
