#ifndef EFU_EFULAUNCHER_H
#include "efulauncher.h"
#endif

#include <iostream>
#include <algorithm>

#ifndef EFU_TARGET_H
#include "target.h"
#endif

#ifndef EFU_CURLEASY_H
#include "curleasy.h"
#endif

namespace
{
    const std::string version("0.1.0");
    const std::string listing("http://nwn.efupw.com/rootdir/index.dat");
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

EfuLauncher::EfuLauncher(const std::string &path ,
            const std::string &update_check):
    m_path(path),
    m_update_check(update_check),
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

bool EfuLauncher::get_update()
{
    if (!m_has_update || m_update_path.empty())
    {
        return m_has_update = false;
    }
    return !(m_has_update = false);
}

void EfuLauncher::stat_targets()
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
        Target t(path(), data[0], data[data.size() - 1]);
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