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

#ifdef _WIN32

#ifndef EFU_SIMPLEREADHKLMKEY_H
#include "simple_read_hklm_key.h"
#endif

#endif

#ifndef EFU_TARGET_H
#include "efulauncher.h"
#endif

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
