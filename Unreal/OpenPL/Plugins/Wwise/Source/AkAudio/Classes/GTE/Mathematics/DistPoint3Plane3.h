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

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Plane3<Real>>
    {
    public:
        struct Result
        {
            Real distance, signedDistance;
            Vector3<Real> planeClosestPoint;
        };

        Result operator()(Vector3<Real> const& point, Plane3<Real> const& plane)
        {
            Result result;
            result.signedDistance = Dot(plane.normal, point) - plane.constant;
            result.distance = std::fabs(result.signedDistance);
            result.planeClosestPoint = point - result.signedDistance * plane.normal;
            return result;
        }
    };
}
