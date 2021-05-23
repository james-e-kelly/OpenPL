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

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>

// The queries consider the box to be a solid.
//
// The aligned-aligned queries use simple min-max comparisions.  The
// interesection of aligned boxes is an aligned box, possibly degenerate,
// where min[d] == max[d] for at least one dimension d.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, AlignedBox3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(AlignedBox3<Real> const& box0, AlignedBox3<Real> const& box1)
        {
            Result result;
            for (int i = 0; i < 3; i++)
            {
                if (box0.max[i] < box1.min[i] || box0.min[i] > box1.max[i])
                {
                    result.intersect = false;
                    return result;
                }
            }
            result.intersect = true;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, AlignedBox3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            AlignedBox3<Real> box;
        };

        Result operator()(AlignedBox3<Real> const& box0, AlignedBox3<Real> const& box1)
        {
            Result result;
            for (int i = 0; i < 3; i++)
            {
                if (box0.max[i] < box1.min[i] || box0.min[i] > box1.max[i])
                {
                    result.intersect = false;
                    return result;
                }
            }

            for (int i = 0; i < 3; i++)
            {
                if (box0.max[i] <= box1.max[i])
                {
                    result.box.max[i] = box0.max[i];
                }
                else
                {
                    result.box.max[i] = box1.max[i];
                }

                if (box0.min[i] <= box1.min[i])
                {
                    result.box.min[i] = box1.min[i];
                }
                else
                {
                    result.box.min[i] = box0.min[i];
                }
            }
            result.intersect = true;
            return result;
        }
    };
}
