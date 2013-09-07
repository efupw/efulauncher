#include <stdexcept>
#include <limits>
#include <algorithm>
#include <array>
#include <memory>
#include <iomanip>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
//#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <system_error>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
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

#ifndef EFU_WINERRORSTRING_H
#include "win_error_string.h"
#endif

#ifndef EFU_SIMPLEREADHKLMKEY_H
#include "simple_read_hklm_key.h"
#endif

#endif

const std::string version("0.1.0");
const std::string listing("http://nwn.efupw.com/rootdir/index.dat");
const std::string patch_dir("http://nwn.efupw.com/rootdir/patch/");

const std::string file_checksum(const std::string &path);

std::vector<std::string> &split(const std::string &s, char delim,
        std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

void make_dir(const std::string &path)
{
#ifdef _WIN32
    unsigned file_start = path.find_last_of("/\\");
    if (file_start == std::string::npos)
    {
        return;
    }
    if (!CreateDirectory(path.substr(0, file_start).c_str(), nullptr))
    {
        WinErrorString wes;
        if (wes.code() != ERROR_ALREADY_EXISTS)
        {
            throw std::ios_base::failure("Failed creating "
                + path + ": " + wes.str());
        }
    }
#else
    auto elems = split(path, '/');
    std::string descend;
    for (size_t i = 0, k = elems.size() - 1; i < k; ++i)
    {
        const std::string &s(elems[i]);
        if (s.size() && s != ".")
        {
            descend.append((i > 0 ? "/" : "") + s);
            auto status = mkdir(descend.c_str(), S_IRWXU);
            if (status == -1)
            {
                std::error_code err(errno, std::generic_category());
                if (err != std::errc::file_exists)
                {
                    std::cout << "error making dir: "
                        << descend << ": "
                        << err.message() << std::endl;
                }
            }
        }
    }
#endif
}

// TODO
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
        explicit Target(const std::string &name, const std::string &checksum):
            m_name(name.find_first_of('/') == std::string::npos ? name :
                    name, name.find_first_of('/') + 1, name.size() - 1),
            m_checksum(checksum)
            {}
        std::string name() const { return m_name; }
        const std::string checksum() const { return m_checksum; }

        void fetch() const
        {
            std::cout << "Statting target " << name() << "...";
            std::fstream fs(name(), std::ios_base::in);
            if (!fs.good())
            {
                fs.close();
                std::cout << " doesn't exist, creating new." << std::endl;
                make_dir(name());
                fs.open(name(), std::ios_base::out);
                if (!fs.good())
                {
                    fs.close();
                    std::cout << "Failed to create file: " << name() << std::endl;
                }
                else
                {
                    fs.close();
                    do_fetch();
                    return;
                }
            }
            if (fs.good())
            {
                fs.close();
                if (status() == Status::Current)
                {
                    std::cout << " already up to date." << std::endl;
                }
                else
                {
                    std::cout << " outdated, downloading new." << std::endl;
                    do_fetch();
                }
            }
        }
        
        Status status() const
        {
            std::ifstream is(name());
            if (!is.good())
            {
                return Status::Nonexistent;
            }
            is.close();

            auto calcsum(file_checksum(name()));
            if (calcsum == checksum())
            {
                return Status::Current;
            }
            else
            {
                return Status::Outdated;
            }
        }

    private:
        void do_fetch() const
        {
            std::ofstream ofs(name(), std::ios::binary);
            if (ofs.good())
            {
                std::string s;
                std::string url(patch_dir + name());
                CurlEasy curl(url);
                curl.write_to(s);
                curl.progressbar(true);
                curl.perform();

                ofs << s;
                ofs.close();
                std::cout << "Finished downloading " << name() << std::endl;
            }
            else
            {
                std::cout << "Couldn't write to " << name() << std::endl;
            }
        }

        std::string m_name;
        std::string m_checksum;
};

std::ostream& operator<<(std::ostream &os, const Target &t)
{
    return os << "name: " << t.name() << ", checksum: " << t.checksum();
}

const std::string file_checksum(const std::string &path)
{
#ifdef md_md5
    auto md = EVP_md5();
    const int md_len = MD5_DIGEST_LENGTH;
#else
    auto md = EVP_sha1();
    const int md_len = SHA_DIGEST_LENGTH;
#endif
    std::array<unsigned char, md_len> result;
    EVP_MD_CTX *mdctx = nullptr;
    std::ifstream is(path, std::ios::binary);
    if (!is.good())
    {
        std::cout << "Couldn't open file " << path
           << " for checksumming." << std::endl;
        return std::string();
    }

    const int length = 8 * 1024;
    std::array<unsigned char, length> buffer;
    auto buf = reinterpret_cast<char *>(buffer.data());
    mdctx = EVP_MD_CTX_create();
    int status = EVP_DigestInit_ex(mdctx, md, nullptr);
    while (status && is)
    {
        is.read(buf, length);
        status = EVP_DigestUpdate(mdctx, buffer.data(),
            static_cast<size_t>(is.gcount()));
    }
    status = EVP_DigestFinal_ex(mdctx, result.data(), nullptr);
    EVP_MD_CTX_destroy(mdctx);
    std::stringstream calcsum;
    calcsum << std::setfill('0');

#ifdef CPP11_FOR_EACH
    for (const unsigned char c : result)
#else
    std::for_each(std::begin(result), std::end(result),
            [&calcsum](const unsigned char c)
#endif
    {
        calcsum << std::hex << std::setw(2)
            << static_cast<unsigned int>(c);
    }
#ifndef CPP11_FOR_EACH
    );
#endif

    return calcsum.str();
}

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
