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
#include <Mathematics/DistPoint3Plane3.h>
#include <Mathematics/OrientedBox.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, OrientedBox3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Plane3<Real> const& plane, OrientedBox3<Real> const& box)
        {
            Result result;

            Real radius =
                std::fabs(box.extent[0] * Dot(plane.normal, box.axis[0])) +
                std::fabs(box.extent[1] * Dot(plane.normal, box.axis[1])) +
                std::fabs(box.extent[2] * Dot(plane.normal, box.axis[2]));

            DCPQuery<Real, Vector3<Real>, Plane3<Real>> ppQuery;
            auto ppResult = ppQuery(box.center, plane);
            result.intersect = (ppResult.distance <= radius);
            return result;
        }
    };
}
