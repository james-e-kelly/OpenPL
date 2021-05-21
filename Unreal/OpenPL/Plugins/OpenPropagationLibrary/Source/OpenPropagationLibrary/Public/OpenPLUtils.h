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
    PLVector Result (UnrealVector.Y / 100, UnrealVector.Z / 100, UnrealVector.X / 100);
    return Result;
}

inline PLVector4 ConvertUnrealVectorToPL4(FVector UnrealVector)
{
    PLVector4 Result { UnrealVector.Y, UnrealVector.Z, UnrealVector.X, 1 };
    return Result;
}

#endif /* OpenPLUtils_h */
