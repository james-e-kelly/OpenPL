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


#include "AkEnvironmentIndex.h"

void FAkEnvironmentOctreeSemantics::SetElementId(AK_OCTREE_TYPE<FAkEnvironmentOctreeElement, FAkEnvironmentOctreeSemantics>& OctreeOwner, const FAkEnvironmentOctreeElement& Element, AK_OCTREE_ELEMENT_ID Id)
{
	static_cast<UAkEnvironmentOctree&>(OctreeOwner).ObjectToOctreeId.Add(Element.Component->GetUniqueID(), Id);
}

void FAkEnvironmentIndex::Add(USceneComponent* in_EnvironmentToAdd)
{
	UWorld* CurrentWorld = in_EnvironmentToAdd->GetWorld();
	auto& Octree = Map.FindOrAdd(CurrentWorld);
	
	if (Octree == nullptr)
	{
		Octree = MakeUnique<UAkEnvironmentOctree>();
	}

	if (Octree != nullptr)
	{
		FAkEnvironmentOctreeElement element(in_EnvironmentToAdd);
		Octree->AddElement(element);
	}
}

void FAkEnvironmentIndex::Remove(USceneComponent* in_EnvironmentToRemove)
{
	UWorld* CurrentWorld = in_EnvironmentToRemove->GetWorld();
	auto* Octree = Map.Find(CurrentWorld);

	if (Octree != nullptr)
	{
		auto id = (*Octree)->ObjectToOctreeId.Find(in_EnvironmentToRemove->GetUniqueID());
		if (id != nullptr && (*Octree)->IsValidElementId(*id))
		{
			(*Octree)->RemoveElement(*id);
		}

		(*Octree)->ObjectToOctreeId.Remove(in_EnvironmentToRemove->GetUniqueID());
	}
}

void FAkEnvironmentIndex::Update(USceneComponent* in_Environment)
{
	Remove(in_Environment);
	Add(in_Environment);
}

void FAkEnvironmentIndex::Clear(const UWorld* World)
{
	Map.Remove(World);
}

bool FAkEnvironmentIndex::IsEmpty(const UWorld* World)
{
	auto* Octree = Map.Find(World);

	return Octree == nullptr;
}
