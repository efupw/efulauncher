#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

#ifndef HEADER_SSL_H
#include "openssl/ssl.h"
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

struct CurlEasy::progress_info
{
    double lastruntime;
    double dltotal;
    std::string sdltotal;
    CURL *curl;
};

namespace
{
    size_t writefunction(const char *ptr, size_t size, size_t nmemb, void *userdata);
    
    const std::string sizetos(double size);

    int progressfunction(void *p,
            double dltotal, double dlnow,
            double ultotal, double ulnow);

    /**
     * Read an SSL certificate from memory. The certificate is hardcoded.
     * Because the first and third arguments are not used they have been left
     * unnamed.
     */
    CURLcode sslctxfunction(CURL *, void *sslctx, void *);
    
    void curlcheck(CURLcode c);

    size_t writefunction(const char * const ptr, const size_t size, const size_t nmemb, void * const userdata)
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

    int progressfunction(void *p, double dltotal, double dlnow, double, double)
    {
        CurlEasy::progress_info *progress
            = static_cast<CurlEasy::progress_info *>(p);

        double speed(0.0);
        double curtime(0.0);
        curl_easy_getinfo(progress->curl,
                CURLINFO_SPEED_DOWNLOAD, &speed);
        curl_easy_getinfo(progress->curl,
                CURLINFO_TOTAL_TIME, &curtime);

        if (dltotal && progress->dltotal != dltotal)
        {
            progress->sdltotal = sizetos(dltotal);
            progress->dltotal = dltotal;
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
    
    // PEM format certificates taken from
    // http://curl.haxx.se/docs/caextract.html
    std::string certs[] = {
        "DigiCert High Assurance EV Root CA\n"\
        "==================================\n"\
        "-----BEGIN CERTIFICATE-----\n"\
        "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBsMQswCQYDVQQG\n"\
        "EwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3d3cuZGlnaWNlcnQuY29tMSsw\n"\
        "KQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5jZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAw\n"\
        "MFoXDTMxMTExMDAwMDAwMFowbDELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZ\n"\
        "MBcGA1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFu\n"\
        "Y2UgRVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm+9S75S0t\n"\
        "Mqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTWPNt0OKRKzE0lgvdKpVMS\n"\
        "OO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEMxChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3\n"\
        "MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFBIk5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQ\n"\
        "NAQTXKFx01p8VdteZOE3hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUe\n"\
        "h10aUAsgEsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQFMAMB\n"\
        "Af8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaAFLE+w2kD+L9HAdSY\n"\
        "JhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3NecnzyIZgYIVyHbIUf4KmeqvxgydkAQ\n"\
        "V8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6zeM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFp\n"\
        "myPInngiK3BD41VHMWEZ71jFhS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkK\n"\
        "mNEVX58Svnw2Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"\
        "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep+OkuE6N36B9K\n"\
        "-----END CERTIFICATE-----\n",
        "Verisign Class 3 Public Primary Certification Authority - G2\n"\
        "============================================================\n"\
        "-----BEGIN CERTIFICATE-----\n"\
        "MIIDAjCCAmsCEH3Z/gfPqB63EHln+6eJNMYwDQYJKoZIhvcNAQEFBQAwgcExCzAJBgNVBAYTAlVT\n"\
        "MRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xhc3MgMyBQdWJsaWMgUHJpbWFy\n"\
        "eSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcyMTowOAYDVQQLEzEoYykgMTk5OCBWZXJpU2ln\n"\
        "biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVz\n"\
        "dCBOZXR3b3JrMB4XDTk4MDUxODAwMDAwMFoXDTI4MDgwMTIzNTk1OVowgcExCzAJBgNVBAYTAlVT\n"\
        "MRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xhc3MgMyBQdWJsaWMgUHJpbWFy\n"\
        "eSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcyMTowOAYDVQQLEzEoYykgMTk5OCBWZXJpU2ln\n"\
        "biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVz\n"\
        "dCBOZXR3b3JrMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDMXtERXVxp0KvTuWpMmR9ZmDCO\n"\
        "FoUgRm1HP9SFIIThbbP4pO0M8RcPO/mn+SXXwc+EY/J8Y8+iR/LGWzOOZEAEaMGAuWQcRXfH2G71\n"\
        "lSk8UOg013gfqLptQ5GVj0VXXn7F+8qkBOvqlzdUMG+7AUcyM83cV5tkaWH4mx0ciU9cZwIDAQAB\n"\
        "MA0GCSqGSIb3DQEBBQUAA4GBAFFNzb5cy5gZnBWyATl4Lk0PZ3BwmcYQWpSkU01UbSuvDV1Ai2TT\n"\
        "1+7eVmGSX6bEHRBhNtMsJzzoKQm5EWR0zLVznxxIqbxhAe7iF6YM40AIOw7n60RzKprxaZLvcRTD\n"\
        "Oaxxp5EJb+RxBrO6WVcmeQD2+A2iMzAo1KpYoJ2daZH9\n"\
        "-----END CERTIFICATE-----\n"
    };

    CURLcode sslctxfunction(CURL * const, void * const sslctx, void * const)
    {
        X509_STORE *store = SSL_CTX_get_cert_store(static_cast<const SSL_CTX *>(sslctx));
        for (auto beg = std::begin(certs); beg != std::end(certs); ++beg)
        {
            BIO *bio = BIO_new_mem_buf(const_cast<char *>(beg->c_str()),
                beg->length());

            // Read the PEM formatted certificate from memory into an X509
            // struct that SSL can use.
            X509 *cert = PEM_read_bio_X509(bio, nullptr, 0, nullptr);
            if (!cert || X509_STORE_add_cert(store, cert) == 0)
            {
                return CURLE_SSL_CERTPROBLEM;
            }
        }
        
        return CURLE_OK ;
    }
    
    inline void curlcheck(const CURLcode c)
    {
        if (c != CURLE_OK)
        {
            throw CurlEasyException(c);
        }
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

    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_URL, url.c_str()));

    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_FOLLOWLOCATION, 1L));

    // For hosts, 2 means verify, 0 means don't, and 1 is ignored!
    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_SSL_VERIFYHOST, 2L));

    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_SSL_VERIFYPEER, 1L));
    
    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_SSL_CTX_FUNCTION,
        &sslctxfunction));
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

    curlcheck(curl_easy_perform(*m_pcurl));
    std::cout << "\n";
    m_used = true;
}

void CurlEasy::write_to(const std::string &dest)
{
    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_WRITEFUNCTION,
        &writefunction));

    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_WRITEDATA, &dest));
}

void CurlEasy::progressbar(bool val)
{
    if (!val)
    {
        curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_NOPROGRESS, 1L));
        return;
    }

    m_progress = std::shared_ptr<progress_info>(new progress_info);
    m_progress->lastruntime = 0.0;
    m_progress->dltotal = 0.0;
    m_progress->sdltotal = "";
    m_progress->curl = *m_pcurl;

    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_NOPROGRESS, 0L));
    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_PROGRESSFUNCTION,
        &progressfunction));
    curlcheck(curl_easy_setopt(*m_pcurl, CURLOPT_PROGRESSDATA,
        m_progress.get()));
}
