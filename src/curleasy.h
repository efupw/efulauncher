#include <string>
#include <memory>
#include <stdexcept>
#include <curl/curl.h>

size_t writefunction(const char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string *s = static_cast<std::string *>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

class CurlGlobalInit
{
    public:
        explicit CurlGlobalInit()
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        ~CurlGlobalInit()
        {
            curl_global_cleanup();
        }
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
        explicit CurlEasy(const std::string &url):
            m_pcurl(std::make_shared<CURL *>(curl_easy_init())),
            m_used(false)
        {
            if (!*m_pcurl)
            {
                throw CurlEasyException("curl handle not properly initialized.");
            }

            if (url.empty())
            {
                throw CurlEasyException("Empty URL.");
            }

            CURLcode c = curl_easy_setopt(*m_pcurl, CURLOPT_URL, url.c_str());
            if (c != CURLE_OK)
            {
                throw CurlEasyException(c);
            }
        }

        ~CurlEasy()
        {
            if (*m_pcurl)
            {
                curl_easy_cleanup(*m_pcurl);
            }
        }

        void perform()
        {
            if (m_used)
            {
                throw CurlEasyException("Cannot reuse curl handles.");
            }

            CURLcode c = curl_easy_perform(*m_pcurl);
            if (c != CURLE_OK)
            {
                throw CurlEasyException(c);
            }
            else
            {
                m_used = true;
            }
        }

        void write_to(const std::string &dest)
        {
            CURLcode c = curl_easy_setopt(*m_pcurl,
                    CURLOPT_WRITEFUNCTION, &writefunction);

            if (c != CURLE_OK)
            {
                throw CurlEasyException(c);
            }

            c = curl_easy_setopt(*m_pcurl,
                    CURLOPT_WRITEDATA, &dest);

            if (c != CURLE_OK)
            {
                throw CurlEasyException(c);
            }
        }

    private:
        std::shared_ptr<CURL *> m_pcurl;
        bool m_used;
};

