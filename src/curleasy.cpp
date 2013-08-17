#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

#include <iostream>
#include <iomanip>
#include <sstream>

struct CurlEasy::progress_info
{
    double lastruntime;
    std::string sdltotal;
    CURL *curl;
};

namespace
{
    size_t writefunction(const char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string *s = static_cast<std::string *>(userdata);
        s->append(ptr, size * nmemb);
        return size * nmemb;
    }

    const std::string sizes[] = { "B", "kB", "MB", "GB" };

    const std::string sizetos(double size)
    {
        size_t div(0);
        double dl(size);

        while (dl >= 1024 && div < 4)
        {
            ++div;
            dl /= 1024;
        }

        std::stringstream ss;
        ss.setf(std::ios::fixed);
        ss << std::setw(6) << std::setprecision(2)
            << dl << " " << sizes[div];
        return ss.str();
    }

    int progressfunction(void *p,
            double dltotal, double dlnow,
            double ultotal, double ulnow)
    {
        CurlEasy::progress_info *progress
            = static_cast<CurlEasy::progress_info *>(p);

        double speed(0.0);
        double curtime(0.0);
        curl_easy_getinfo(progress->curl,
                CURLINFO_SPEED_DOWNLOAD, &speed);
        curl_easy_getinfo(progress->curl,
                CURLINFO_TOTAL_TIME, &curtime);

        if (progress->sdltotal.empty() && dltotal)
        {
            progress->sdltotal = sizetos(dltotal);
        }

        if (curtime - progress->lastruntime >= 0.5
                || dlnow == dltotal)
        {
            progress->lastruntime = curtime;

            std::stringstream ss;
            ss.setf(std::ios::fixed);
            ss << sizetos(dlnow)
                << " / ";
            if (dltotal)
            {
                ss << progress->sdltotal
                << " ("
                << std::setprecision(2)
                << dlnow / dltotal * 100
                << "%) @ " << sizetos(speed)
                << "/s, ";
            }
            else
            {
                ss << "unknown";
            }
            ss << std::setprecision(0) << std::setw(2)
                << curtime
                << "s\r";

            std::cout << ss.str();
            std::cout.flush();
        }

        return 0;
    }
}

CurlGlobalInit::CurlGlobalInit()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

CurlGlobalInit::~CurlGlobalInit()
{
    curl_global_cleanup();
}

CurlEasy::CurlEasy(const std::string &url):
    m_pcurl(std::make_shared<CURL *>(curl_easy_init())),
    m_used(false),
    m_progress(nullptr)
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

CurlEasy::~CurlEasy()
{
    if (*m_pcurl)
    {
        curl_easy_cleanup(*m_pcurl);
    }
}

void CurlEasy::perform()
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
        std::cout << "\n";
        m_used = true;
    }
}

void CurlEasy::write_to(const std::string &dest)
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

void CurlEasy::progressbar(bool val)
{
    if (!val)
    {
        curl_easy_setopt(*m_pcurl, CURLOPT_NOPROGRESS, 1L);
        return;
    }

    m_progress = std::shared_ptr<progress_info>(new progress_info);
    m_progress->lastruntime = 0.0;
    m_progress->sdltotal = "";
    m_progress->curl = *m_pcurl;

    curl_easy_setopt(*m_pcurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(*m_pcurl, CURLOPT_PROGRESSFUNCTION, &progressfunction);
    curl_easy_setopt(*m_pcurl, CURLOPT_PROGRESSDATA, m_progress.get());
}
