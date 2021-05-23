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

#include <Mathematics/DistLine3Triangle3.h>
#include <Mathematics/DistPointTriangle.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Segment3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real segmentParameter, triangleParameter[3];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Segment3<Real> const& segment, Triangle3<Real> const& triangle)
        {
            Result result;

            Vector3<Real> segCenter, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segCenter, segDirection, segExtent);

            Line3<Real> line(segCenter, segDirection);
            DCPQuery<Real, Line3<Real>, Triangle3<Real>> ltQuery;
            auto ltResult = ltQuery(line, triangle);

            if (ltResult.lineParameter >= -segExtent)
            {
                if (ltResult.lineParameter <= segExtent)
                {
                    result.distance = ltResult.distance;
                    result.sqrDistance = ltResult.sqrDistance;
                    result.segmentParameter = ltResult.lineParameter;
                    result.triangleParameter[0] = ltResult.triangleParameter[0];
                    result.triangleParameter[1] = ltResult.triangleParameter[1];
                    result.triangleParameter[2] = ltResult.triangleParameter[2];
                    result.closestPoint[0] = ltResult.closestPoint[0];
                    result.closestPoint[1] = ltResult.closestPoint[1];
                }
                else
                {
                    DCPQuery<Real, Vector3<Real>, Triangle3<Real>> ptQuery;
                    Vector3<Real> point = segCenter + segExtent * segDirection;
                    auto ptResult = ptQuery(point, triangle);
                    result.sqrDistance = ptResult.sqrDistance;
                    result.distance = ptResult.distance;
                    result.segmentParameter = segExtent;
                    result.closestPoint[0] = point;
                    result.closestPoint[1] = ptResult.closest;
                }
            }
            else
            {
                DCPQuery<Real, Vector3<Real>, Triangle3<Real>> ptQuery;
                Vector3<Real> point = segCenter - segExtent * segDirection;
                auto ptResult = ptQuery(point, triangle);
                result.sqrDistance = ptResult.sqrDistance;
                result.distance = ptResult.distance;
                result.segmentParameter = segExtent;
                result.closestPoint[0] = point;
                result.closestPoint[1] = ptResult.closest;
            }
            return result;
        }
    };
}
