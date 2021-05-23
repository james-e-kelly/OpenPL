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

#include <string>

namespace gte
{
    struct VertexAttribute
    {
        VertexAttribute(std::string inSemantic = "", void* inSource = nullptr, size_t inStride = 0)
            :
            semantic(inSemantic),
            source(inSource),
            stride(inStride)
        {
        }

        // The 'semantic' string allows you to query for a specific vertex
        // attribute and use the 'source' and 'stride' to access the data
        // of the attribute.  For example, you might use the semantics
        // "position" (px,py,pz), "normal" (nx,ny,nz), "tcoord" (texture
        // coordinates (u,v)), "dpdu" (derivative of position with respect
        // to u), or "dpdv" (derivative of position with respect to v) for
        // mesh vertices.
        //
        // The source pointer must be 4-byte aligned.  The stride must be
        // positive and a multiple of 4.  The pointer alignment constraint is
        // guaranteed on 32-bit and 64-bit architectures.  The stride constraint
        // is reasonable given that (usually) geometric attributes are usually
        // arrays of 'float' or 'double'.

        std::string semantic;
        void* source;
        size_t stride;
    };
}
