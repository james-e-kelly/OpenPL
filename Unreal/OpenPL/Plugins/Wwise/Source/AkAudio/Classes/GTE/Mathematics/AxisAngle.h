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

#include <Mathematics/Vector.h>

namespace gte
{
    // Axis-angle representation for N = 3 or N = 4.  When N = 4, the axis
    // must be a vector of the form (x,y,z,0) [affine representation of the
    // 3-tuple direction].

    template <int N, typename Real>
    class AxisAngle
    {
    public:
        AxisAngle()
            :
            axis(Vector<N, Real>::Zero()),
            angle((Real)0)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        AxisAngle(Vector<N, Real> const& inAxis, Real inAngle)
            :
            axis(inAxis),
            angle(inAngle)
        {
            static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        }

        Vector<N, Real> axis;
        Real angle;
    };
}
