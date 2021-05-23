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

#include <Mathematics/Math.h>

namespace gte
{
    // Find-intersection queries.

    template <typename Real, typename Type0, typename Type1>
    class FIQuery
    {
    public:
        struct Result
        {
            // A FIQuery-base class B must define a B::Result struct with
            // member 'bool intersect'.  A FIQuery-derived class D must also
            // derive a D::Result from B:Result but may have no members.  The
            // member 'intersect' is 'true' iff the primitives intersect.  The
            // operator() is const for conceptual constness, but derived
            // classes can use internal data to support the queries and tag
            // that data with the mutable modifier.
        };

        Result operator()(Type0 const& primitive0, Type1 const& primitive1) const;
    };
}
