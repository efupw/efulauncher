#ifndef EFU_EFULAUNCHER_H
#include "efulauncher.h"
#endif

#include <iostream>
#include <fstream>
#include <algorithm>

#ifndef EFU_TARGET_H
#include "target.h"
#endif

#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

namespace
{
    const std::string listing("http://nwn.efupw.com/rootdir/index.dat");

    int replace_all(std::string &haystack, const std::string &needle,
            const std::string &val);

    int replace_all(std::string &haystack, const std::string &needle,
            const std::string &val)
    {
        size_t count = 0, pos = 0;
        while ((pos = haystack.find(needle, pos)) != std::string::npos)
        {
            haystack.replace(pos, needle.size(), val);
            // Move past the new value in case needle is a substring of val.
            pos += val.size();
            ++count;
        }

        return count;
    }
}

namespace Options
{
    bool version(const std::string &val)
    {
#ifdef _WIN32
        return val == "win-version";
#else
        return val == "lin-version";
#endif
    }
    bool update_path(const std::string &val)
    {
        return val == "update path";
    }
};

EfuLauncher::EfuLauncher(const std::string &path,
            const std::string &update_check,
            const std::string &version):
    m_path(path),
    m_update_check(update_check),
    m_update_path(),
    m_version(version),
    m_has_update(false)
{
}

bool EfuLauncher::has_update()
{
    if (m_has_update)
    {
        return m_has_update;
    }

    std::string fetch;
    CurlEasy curl(m_update_check.c_str());
    curl.write_to(fetch);
    try
    {
        curl.perform();
    }
    catch (CurlEasyException &e)
    {
        std::cerr
            << "Update check failed. Could not establish secure connection to "
            << m_update_check
            << ": "
            << e.what() << std::endl;
    }

    std::vector<std::string> lines(split(fetch, '\n'));
    fetch.clear();
    for (const auto& line : lines)
    {
        auto keyvals(split(line, '='));
        if (keyvals.size() != 2)
        {
            std::cerr << "Malformed option: " << line <<
                ", aborting launcher update check." << std::endl;
            return m_has_update = false;
        }
        if (Options::version(keyvals[0]))
        {
            const std::string version_test(keyvals[1]);
            m_has_update = version_test != m_version;
            if (m_has_update)
            {
                m_version = version_test;
            }
        }
        else if (Options::update_path(keyvals[0]))
        {
            m_update_path = keyvals[1];
        }

    }
    return m_has_update;
}

bool EfuLauncher::get_update()
{
    if (!m_has_update || m_update_path.empty())
    {
        return m_has_update = false;
    }
    // Token replacement performed here because we're guaranteed to have all
    // values.
    replace_all(m_update_path, "{version}", m_version);
    replace_all(m_update_path, "{platform}",
#ifdef _WIN32
    "win"
#else
    "lin"
#endif
    );

    size_t pos = 0;
    if ((pos = m_update_path.rfind("/")) == std::string::npos)
    {
        throw std::runtime_error("Couldn't find '/' in update path to"\
                " determine file name.");
    }

    // m_update_path.substr(pos) begins with "/".
    const std::string dlpath("." + m_update_path.substr(pos));
    std::string fetch;
    CurlEasy curl(m_update_path);
    curl.write_to(fetch);
    curl.progressbar(true);
    try
    {
        curl.perform();
    }
    catch (CurlEasyException &e)
    {
        std::cerr
            << "Download failed. Could not establish secure connection to "
            << m_update_path
            << ": "
            << e.what() << std::endl;

        return false;
    }

    std::ofstream ofs(dlpath, std::ios::binary);
    if (ofs)
    {
        ofs << fetch;
        ofs.close();
        m_has_update = false;
    }
    else
    {
        throw std::runtime_error("Couldn't write to " + dlpath);
    }

    return !m_has_update;
}

void EfuLauncher::stat_targets()
{
    std::string fetch;
    CurlEasy curl(listing);
    curl.write_to(fetch);
    curl.perform();

    auto lines(split(fetch, '\n'));
    std::vector<Target> new_targets, old_targets;
    for (const auto& line : lines)
    {
        auto data(split(line, '@'));
        Target t(path(), data[0], data[data.size() - 1]);
        auto status = t.status();

        if (status == Target::Status::Nonexistent)
        {
            new_targets.push_back(std::move(t));
        }
        else if (status == Target::Status::Outdated)
        {
            old_targets.push_back(std::move(t));
        }
    }
    if (new_targets.size())
    {
        std::cout << "New targets: " << new_targets.size() << std::endl;
        for (const auto &t : new_targets)
        {
            std::cout << "- " << t.name() << std::endl;
        }
    }
    else
    {
        std::cout << "No new targets." << std::endl;
    }

    if (old_targets.size())
    {
        std::cout << "Outdated targets: " << old_targets.size() << std::endl;
        for (const auto &t : old_targets)
        {
            std::cout << "- " << t.name() << std::endl;
        }
    }
    else
    {
        std::cout << "No targets out of date." << std::endl;
    }

#ifndef DEBUG
    for (auto &t : new_targets)
    {
        t.fetch();
    }
    for (auto &t : old_targets)
    {
        t.fetch();
    }
#endif
}
