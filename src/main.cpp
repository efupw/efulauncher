#include <limits>
#include <iostream>
#include <fstream>
#include <vector>

#include <algorithm>
#ifdef _WIN32

#ifndef EFU_SIMPLEREADHKLMKEY_H
#include "simple_read_hklm_key.h"
#endif

#ifndef EFU_WINERRORSTRING_H
#include "win_error_string.h"
#endif

#endif

#ifndef EFU_TARGET_H
#include "target.h"
#endif

#ifndef EFU_EFULAUNCHER_H
#include "efulauncher.h"
#endif

#ifndef EFU_CURLEASY_H
#include "curleasy.h"
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

#ifdef _WIN32
const std::string version("1.1.0");
#else
const std::string version("1.1.0");
#endif

int main(int argc, char *argv[])
{
    CurlGlobalInit curl_global;
    bool arg_errors = false;

#ifdef _WIN32
    const std::string nwn_bin("nwmain.exe");
#else
    const std::string nwn_bin("nwmain");
#endif
    std::string nwn_root_dir("./");

    std::string cmd_line(" +connect nwn.efupw.com:5121");
    
    const std::vector<std::string> args(argv + 1, argv + argc);

    std::cout << "Processing command line arguments." << std::endl;

#ifdef CPP11_FOR_EACH
    for (const auto arg : args)
#else
    std::for_each(args.cbegin(), args.cend(),
            [&cmd_line, &nwn_root_dir, &arg_errors]
            (const std::string &arg)
#endif
    {
        if (arg.find("-dmpass") == 0)
        {
            auto cmd(split(arg, '='));
            if (cmd.size() == 2)
            {
                cmd_line.append(" -dmc +password " + cmd.at(1));
            }
            else
            {
                std::cout << "-dmpass specified but no value given. Use\n"\
                    "-dmpass=mypassword" <<std::endl;
                arg_errors = true;
            }
        }
        else if (arg.find("-nwn") == 0)
        {
            auto cmd(split(arg, '='));
            if (cmd.size() == 2)
            {
                nwn_root_dir = cmd.at(1);
            }
            else
            {
                std::cout << "-nwn specified but no value given. Use\n"\
                    "-nwn=\"path\\to\\NWN\\directory\\\"" <<std::endl;
                arg_errors = true;
            }
        }
        else
        {
            std::cout << "Ignoring unrecognized argument: " << arg
                << std::endl;
            arg_errors = true;
        }
    }
#ifndef CPP11_FOR_EACH
    );
#endif
    
    if (arg_errors)
    {
        std::cout << "Argument errors. Press a key to continue." << std::endl;
        std::cin.get();
    }

#ifdef _WIN32
    std::ifstream nwn(nwn_root_dir + nwn_bin, std::ios::binary);
    
    if (!nwn && nwn_root_dir != "./")
    {
        std::cout << nwn_root_dir << " not detected as NWN root directory."\
            "\nTrying current launcher directory...\n";
        nwn_root_dir = "./";
        nwn.open(nwn_root_dir + nwn_bin, std::ios::binary);
    }

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
        std::cout << nwn_root_dir + nwn_bin << " found." << std::endl;
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
        nwn_root_dir = "./";
    }
#endif

    EfuLauncher l(nwn_root_dir,
            "https://raw.github.com/commonquail/efulauncher/"\
            "master/versioncheck",
            version);

    if (l.has_update())
    {
        std::cout << "A new version of the launcher is available. Please"\
            " download it at https://github.com/commonquail/efulauncher/"\
            "releases. Press any key to exit." << std::endl;
        std::cin.get();
        return 0;
        /*
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
        */
    }

    try
    {
        l.stat_targets();
        std::cout << "Done." << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

#ifdef _WIN32
    if (nwn)
    {
        std::cout << "Launching " << nwn_root_dir + nwn_bin << "..." << std::endl;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ::ZeroMemory(&pi, sizeof(pi));
        ::ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        
        auto nwn_path(nwn_root_dir + nwn_bin);
        cmd_line = nwn_path + cmd_line;

        BOOL success = ::CreateProcess(
            const_cast<char *>(nwn_path.c_str()),
            const_cast<char *>(cmd_line.c_str()),
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            const_cast<char *>(nwn_root_dir.c_str()),           // Starting directory 
            &si, &pi);
        if (success)
        {
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
        }
        else
        {
            WinErrorString we;
            std::cout << we.str() << std::endl;
        }
    }
#endif

    return 0;
}
