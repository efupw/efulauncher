#ifndef EFU_CURLEASY_H
#define EFU_CURLEASY_H

#include <string>
#include <memory>
#include <stdexcept>
#include <curl/curl.h>
#undef max

class CurlGlobalInit
{
    public:
        explicit CurlGlobalInit();
        ~CurlGlobalInit();
};

class CurlEasyException : public std::runtime_error
{
    public:
        explicit CurlEasyException(const std::string &msg):
            std::runtime_error(msg) {}

        explicit CurlEasyException(CURLcode c):
            std::runtime_error(curl_easy_strerror(c)) {}
};

class CurlEasy
{
    public:
        explicit CurlEasy(const std::string &url);

        ~CurlEasy();

        void perform();
        void write_to(const std::string &dest);

        void progressbar(bool val);

        struct progress_info;
    private:
        std::shared_ptr<CURL *> m_pcurl;
        bool m_used;
        std::shared_ptr<progress_info> m_progress;
};
#endif //EFU_CURLEASY_H
