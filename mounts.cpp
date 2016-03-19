#include "mounts.h"

#include <algorithm>

using namespace std;

Mounts::Mounts(const string &absoluteBasePath)
    : m_MountPathToMountpoint()
{
    mount( "/", absoluteBasePath );
}

string Mounts::absolutePath(const string &mountPath) const
{
    // aaaa/
    // bbbb/

    // aa/Test

    // cccc/Test


    auto it = std::upper_bound(m_MountPathToMountpoint.begin(), m_MountPathToMountpoint.end(), mountPath + '/', [](const auto& path, auto pair)
    {
        return path < pair.first;
    });
    if (it == m_MountPathToMountpoint.begin())
    {
        return {};
    }
    --it;
    auto candidate = it->first;
    if (candidate.length() == mountPath.length() && mountPath == candidate)
    {
        return it->second.absolutePath;
    }
    if (candidate.length() < mountPath.length() && mountPath[candidate.length()] == '/' && 0 == mountPath.compare(0, candidate.length(), candidate))
    {
        return it->second.absolutePath + string(mountPath.begin()+candidate.length(), mountPath.end());
    }
    return {};
}

vector<string> Mounts::allMountPathes(const string &absolutePath) const
{
    vector<string> results;

    for(auto mapping : m_MountPathToMountpoint)
    {
        auto ap = mapping.second.absolutePath;

        if((ap.length() == absolutePath.length()) && (absolutePath == ap))
        {
           results.push_back(mapping.first);
        }
        else if((ap.length() < absolutePath.length()) &&
                (absolutePath[ap.length()] == '\\') &&
                (0 == absolutePath.compare(0, ap.length(), ap)))
        {
            results.push_back(mapping.first + string(absolutePath.begin()+ ap.length(), absolutePath.end()));
        }
    }

    return results;
}

vector<pair<string, string> > Mounts::getMountPointsBelow(const string &mountPath) const
{
    vector<pair<MountPath, AbsolutePath>> result;

    auto it = std::upper_bound(m_MountPathToMountpoint.begin(), m_MountPathToMountpoint.end(), mountPath + '/', [](const auto& path, auto pair)
    {
        return path < pair.first;
    });

    if (it == m_MountPathToMountpoint.begin())
        return result;

    for(--it; it != m_MountPathToMountpoint.end(); ++it) {
        if( it->first.compare(0, mountPath.length(), mountPath) ) {
            break;
        }
        result.push_back({ it->first, it->second.absolutePath });
    }

    return result;
}

void Mounts::mount(const MountPath &mountPath, const AbsolutePath &absolutePath)
{
    m_MountPathToMountpoint[ mountPath ] = { absolutePath, {} };
}

void Mounts::unmountAbsolute(const AbsolutePath &absolutePath)
{
    auto keylist = allMountPathes(absolutePath);

    for (auto key : keylist)
    {
        unmountBelow(key);
    }
}

void Mounts::unmountBelow(const MountPath &mountPath)
{
    auto points = getMountPointsBelow(mountPath);

    for (auto p : points)
    {
        m_MountPathToMountpoint.erase(p.first);
    }
}
