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
#include <Mathematics/DistPoint3Frustum3.h>
#include <Mathematics/Hypersphere.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Sphere3<Real>, Frustum3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Sphere3<Real> const& sphere, Frustum3<Real> const& frustum)
        {
            Result result;
            DCPQuery<Real, Vector3<Real>, Frustum3<Real>> vfQuery;
            Real distance = vfQuery(sphere.center, frustum).distance;
            result.intersect = (distance <= sphere.radius);
            return result;
        }
    };
}
