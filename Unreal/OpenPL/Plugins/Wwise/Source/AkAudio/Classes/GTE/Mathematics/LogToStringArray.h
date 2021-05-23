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

#include <Mathematics/Logger.h>
#include <vector>

namespace gte
{
    class LogToStringArray : public Logger::Listener
    {
    public:
        LogToStringArray(std::string const& name, int flags)
            :
            Logger::Listener(flags),
            mName(name)
        {
        }

        inline std::string const& GetName() const
        {
            return mName;
        }

        inline std::vector<std::string> const& GetMessages() const
        {
            return mMessages;
        }

        inline std::vector<std::string>& GetMessages()
        {
            return mMessages;
        }

    private:
        virtual void Report(std::string const& message)
        {
            mMessages.push_back(message);
        }

        std::string mName;
        std::vector<std::string> mMessages;
    };
}
