#include <algorithm>
#include <array>
#include <iomanip>
#include <curl/curl.h>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <system_error>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

const std::string listing("http://nwn.efupw.com/rootdir/index.dat");
const std::string patch_dir("http://nwn.efupw.com/rootdir/patch/");
const std::string update_check("");

size_t writefunction(const char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string *s = static_cast<std::string *>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

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

class Target
{
    public:
        enum class Status {
            Nonexistent,
            Outdated,
            Current
        };
        explicit Target(const std::string &name, const std::string &checksum):
            m_name(name.find_first_of('/') == std::string::npos ?
                    name
                    : name, name.find_first_of('/') + 1, name.size() - 1), m_checksum(checksum)
    {}
        std::string name() const { return m_name; }
        std::string checksum() const { return m_checksum; }
        void fetch()
        {
            std::cout << "pretending to fetch " << name() << std::endl;
            std::string path;
            std::ofstream ofs(name());
            if (!ofs.good())
            {
                auto elems = split(name(), '/');
                for (size_t i = 0, k = elems.size(); i < k; ++i)
                    //for (auto s : elems)
                {
                    const std::string &s(elems[i]);
                    if (s.size() && s != ".")
                    {
                        path.append((i > 0 ? "/" : "") + s);
                        if (k > 1 && i < k - 1)
                        {
                            auto status = mkdir(path.c_str(), S_IRWXU);
                            if (status == -1)
                            {
                                std::error_code err(errno, std::generic_category());
                                if (err == std::errc::file_exists)
                                {
                                    std::cout << "error making dir: "
                                        << err.message() << std::endl;
                                }
                            }
                        }
                        std::cout << "the path: " << path << std::endl;
                    }
                }
                ofs.open(path);
                if (ofs.good())
                {
                    std::cout << "opened path " << path << std::endl;
                }
            }
            if (ofs.good())
            {
                const std::string md5("38eaad974c15e5f3119469f17e8e97a9");
                //if (md5 != checksum())
                {
                    std::cout << "Downloading new " << name() << std::endl;
                    std::string s;
                    std::shared_ptr<CURL *> phandle =
                        std::make_shared<CURL *>(curl_easy_init());
                    std::string url(patch_dir + name());
                    curl_easy_setopt(*phandle, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(*phandle, CURLOPT_WRITEFUNCTION, &writefunction);
                    curl_easy_setopt(*phandle, CURLOPT_WRITEDATA, &s);
                    //curl_easy_setopt(*phandle, CURLOPT_NOPROGRESS, 0);
                    curl_easy_perform(*phandle);
                    curl_easy_cleanup(*phandle);
                    phandle.reset();
                    ofs << s;
                    ofs.close();
                }
                //else
                {
                    std::cout << name() << " already up to date." << std::endl;
                }
            }
        }
        Status status()
        {
            return Status::Nonexistent;
        }

    private:
        std::string m_name;
        std::string m_checksum;
};

std::ostream& operator<<(std::ostream &os, const Target &t)
{
    return os << "name: " << t.name() << ", checksum: " << t.checksum();
}

class CurlGlobalInit
{
    public:
        CurlGlobalInit()
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        ~CurlGlobalInit()
        {
            curl_global_cleanup();
        }
};

bool file_checksum_test(const std::string &path,
        const std::string &checksum)
{
#define md_md5
#ifdef md_md5
    auto md = EVP_md5();
    const int md_len = MD5_DIGEST_LENGTH;
#else
    auto md = EVP_sha1();
    const int md_len = SHA_DIGEST_LENGTH;
#endif
    std::array<unsigned char, md_len> result;
    EVP_MD_CTX *mdctx = nullptr;
    std::ifstream is(path, std::ifstream::binary);

    const int length = 8192;
    std::array<unsigned char, length> buffer;
    auto buf = reinterpret_cast<char *>(buffer.data());
    mdctx = EVP_MD_CTX_create();
    int status = EVP_DigestInit_ex(mdctx, md, nullptr);
    while (status && is)
    {
        is.read(buf, length);
        status = EVP_DigestUpdate(mdctx, buffer.data(), is.gcount());
    }
    status = EVP_DigestFinal_ex(mdctx, result.data(), nullptr);
    EVP_MD_CTX_destroy(mdctx);
    std::stringstream calcsum;
    calcsum << std::setfill('0');
    
    std::for_each(std::begin(result), std::end(result),
            [&calcsum](unsigned char c)
            {
            calcsum << std::hex << std::setw(2)
            << static_cast<unsigned int>(c);
            });
    std::cout << calcsum.str() << std::endl;
    return calcsum.str() == checksum;
}

int main(int argc, char *argv[])
{
    CurlGlobalInit curl_global;
    std::string fetch;
    std::shared_ptr<CURL *> phandle = std::make_shared<CURL *>(curl_easy_init());
    curl_easy_setopt(*phandle, CURLOPT_URL, listing.c_str());
    curl_easy_setopt(*phandle, CURLOPT_WRITEFUNCTION, &writefunction);
    curl_easy_setopt(*phandle, CURLOPT_WRITEDATA, &fetch);
    //curl_easy_setopt(*phandle, CURLOPT_NOPROGRESS, 0);
    curl_easy_perform(*phandle);
    curl_easy_cleanup(*phandle);
    phandle.reset();
    //std::cout << fetch << std::endl;
    std::vector<std::string> v(split(fetch, '\n'));
    //split(fetch, '\n', v);
    //std::cout << v[1] << std::endl;
    std::vector<Target> new_targets, old_targets;
    for (auto beg = std::begin(v), end = std::end(v);
            beg != end; ++beg)
    {
        auto w(split(*beg, '@'));
        /*
        for (const auto & sss : w)
        {
            std::cout << sss << ", " << std::endl;
        }
        */
        Target t(w[0], w[w.size() - 1]);
        auto status = t.status();
        if (status == Target::Status::Nonexistent)
        {
            new_targets.push_back(std::move(t));
        }
        else if (status == Target::Status::Outdated)
        {
            old_targets.push_back(std::move(t));
        }
        //std::cout << *beg << std::endl;
    }
    std::stringstream ss(fetch);
    std::string item;
    //std::vector<std::pair<std::string, std::string>> new_targets;
    while (std::getline(ss, item))
    {
        //new_targets.push_back(item);
    }
    if (new_targets.size())
    {
        std::cout << "New targets: " << new_targets.size() << std::endl;
        //for (auto &t : new_targets)
        {
            //std::cout << t << std::endl;
            //t.fetch();
        }
        std::cout << new_targets[4] << std::endl;
        new_targets[1].fetch();
        new_targets[4].fetch();
        /*
        for (auto s : split(new_targets[4].name(), '/'))
        {
            if (s.length())
            {
                std::cout << s << ", ";
            }
        }
        */
    }
    else
    {
        std::cout << "No new targets." << std::endl;
    }
    if (old_targets.size())
    {
        std::cout << "Outdated targets: " << old_targets.size() << std::endl;
        for (auto &t : old_targets)
        {
            std::cout << t << std::endl;
        }
    }
    else
    {
        std::cout << "No targets out of date." << std::endl;
    }
    const std::string path = "bin/override/spells.2da";
    const std::string checksum("38eaad974c15e5f3119469f17e8e97a9");
    std::cout << "file checksum test: "
        << path << " = " << checksum
        << " : " << file_checksum_test(path, checksum)
        << std::endl;

    std::cout << std::endl;
    return 0;
}
