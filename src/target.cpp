#ifndef EFU_TARGET_H
#include "target.h"
#endif

#include <array>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

#ifdef _WIN32

#ifndef EFU_WINERRORSTRING_H
#include "win_error_string.h"
#endif

#else

#include <sys/stat.h>
#include <system_error>

#endif

namespace
{
    void make_dir(const std::string &path);

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
            if (s.size())
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
}

strvec &split(const std::string &s, char delim, strvec &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

strvec split(const std::string &s, char delim)
{
    strvec elems;
    return split(s, delim, elems);
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

Target::Target(const std::string &dlpath,
    const std::string &name, const std::string &checksum):
    m_dlpath(dlpath),
    m_name(name.find_first_of('/') == std::string::npos
        ? name : name, name.find_first_of('/') + 1, name.size() - 1),
    m_checksum(checksum)
{
}

void Target::fetch() const
{
    std::cout << "Statting target " << name() << "...";
    std::fstream fs(dlpath() + name(), std::ios_base::in);
    if (!fs.good())
    {
        fs.close();
        std::cout << " doesn't exist, creating new." << std::endl;
        make_dir(dlpath() + name());
        fs.open(dlpath() + name(), std::ios_base::out);
        if (!fs.good())
        {
            fs.close();
            std::cout << "Failed to create file: " << dlpath() + name() << std::endl;
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

Target::Status Target::status() const
{
    std::ifstream is(dlpath() + name());
    if (!is.good())
    {
        return Status::Nonexistent;
    }
    is.close();

    auto calcsum(file_checksum(dlpath() + name()));
    if (calcsum == checksum())
    {
        return Status::Current;
    }
    else
    {
        return Status::Outdated;
    }
}

void Target::do_fetch() const
{
    std::ofstream ofs(dlpath() + name(), std::ios::binary);
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
        std::cout << "Couldn't write to " << dlpath() + name() << std::endl;
    }
}
