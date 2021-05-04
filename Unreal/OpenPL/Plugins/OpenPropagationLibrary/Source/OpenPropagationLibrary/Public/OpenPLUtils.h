//
//  OpenPLUtils.h
//  OpenPL
//
//  Created by James Kelly on 03/05/2021.
//  Copyright © 2021 Epic Games, Inc. All rights reserved.
//

#ifndef OpenPLUtils_h
#define OpenPLUtils_h

#include "OpenPL.hpp"

inline PLVector ConvertUnrealVectorToPL(FVector UnrealVector)
{
    PLVector Result (UnrealVector.X, UnrealVector.Y, UnrealVector.Z);
    return Result;
}

inline PLVector4 ConvertUnrealVectorToPL4(FVector UnrealVector)
{
    PLVector4 Result { UnrealVector.X, UnrealVector.Y, UnrealVector.Z, 1 };
    return Result;
}

#endif /* OpenPLUtils_h */
