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

#include <Mathematics/FeatureKey.h>

// An ordered edge has (V[0], V[1]) = (v0, v1).  An unordered edge has
// (V[0], V[1]) = (min(V[0],V[1]), max(V[0],V[1])).

namespace gte
{
    template <bool Ordered>
    class EdgeKey : public FeatureKey<2, Ordered>
    {
    public:
        // Initialize to invalid indices.
        EdgeKey()
        {
            this->V = { -1, -1 };
        }

        // This constructor is specialized based on Ordered.
        explicit EdgeKey(int v0, int v1)
        {
            Initialize(v0, v1);
        }

    private:
        template <bool Dummy = Ordered>
        typename std::enable_if<Dummy, void>::type
        Initialize(int v0, int v1)
        {
            this->V[0] = v0;
            this->V[1] = v1;
        }

        template <bool Dummy = Ordered>
        typename std::enable_if<!Dummy, void>::type
        Initialize(int v0, int v1)
        {
            if (v0 < v1)
            {
                // v0 is minimum
                this->V[0] = v0;
                this->V[1] = v1;
            }
            else
            {
                // v1 is minimum
                this->V[0] = v1;
                this->V[1] = v0;
            }
        }
    };
}
