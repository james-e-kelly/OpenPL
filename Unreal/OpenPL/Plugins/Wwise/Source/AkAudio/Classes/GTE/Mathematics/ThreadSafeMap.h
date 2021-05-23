/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <map>
#include <mutex>
#include <vector>

namespace gte
{
    template <typename Key, typename Value>
    class ThreadSafeMap
    {
    public:
        // Construction and destruction.
        ThreadSafeMap() = default;
        virtual ~ThreadSafeMap() = default;

        // All the operations are thread-safe.
        bool HasElements() const
        {
            bool hasElements;
            mMutex.lock();
            {
                hasElements = (mMap.size() > 0);
            }
            mMutex.unlock();
            return hasElements;
        }

        bool Exists(Key key) const
        {
            bool exists;
            mMutex.lock();
            {
                exists = (mMap.find(key) != mMap.end());
            }
            mMutex.unlock();
            return exists;
        }

        void Insert(Key key, Value value)
        {
            mMutex.lock();
            {
                mMap[key] = value;
            }
            mMutex.unlock();
        }

        bool Remove(Key key, Value& value)
        {
            bool exists;
            mMutex.lock();
            {
                auto iter = mMap.find(key);
                if (iter != mMap.end())
                {
                    value = iter->second;
                    mMap.erase(iter);
                    exists = true;
                }
                else
                {
                    exists = false;
                }
            }
            mMutex.unlock();
            return exists;
        }

        void RemoveAll()
        {
            mMutex.lock();
            {
                mMap.clear();
            }
            mMutex.unlock();
        }

        bool Get(Key key, Value& value) const
        {
            bool exists;
            mMutex.lock();
            {
                auto iter = mMap.find(key);
                if (iter != mMap.end())
                {
                    value = iter->second;
                    exists = true;
                }
                else
                {
                    exists = false;
                }
            }
            mMutex.unlock();
            return exists;
        }

        void GatherAll(std::vector<Value>& values) const
        {
            mMutex.lock();
            {
                if (mMap.size() > 0)
                {
                    values.resize(mMap.size());
                    auto viter = values.begin();
                    for (auto const& m : mMap)
                    {
                        *viter++ = m.second;
                    }
                }
                else
                {
                    values.clear();
                }
            }
            mMutex.unlock();
        }

    protected:
        std::map<Key, Value> mMap;
        mutable std::mutex mMutex;
    };
}
