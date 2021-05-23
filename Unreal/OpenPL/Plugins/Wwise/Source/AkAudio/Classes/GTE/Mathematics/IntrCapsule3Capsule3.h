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

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistSegmentSegment.h>
#include <Mathematics/Capsule.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Capsule3<Real>, Capsule3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Capsule3<Real> const& capsule0, Capsule3<Real> const& capsule1)
        {
            Result result;
            DCPQuery<Real, Segment3<Real>, Segment3<Real>> ssQuery;
            auto ssResult = ssQuery(capsule0.segment, capsule1.segment);
            Real rSum = capsule0.radius + capsule1.radius;
            result.intersect = (ssResult.distance <= rSum);
            return result;
        }
    };
}
