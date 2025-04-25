﻿// Copyright JAA Contributors 2024-2025

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Animation/BlendSpace.h"

/* Access the samples variable */
class CBlendSpaceDerived : public UBlendSpace {
public:
	void CreateNewSample(UAnimSequence* AnimationSequence, const FVector& SampleValue);
};

class IBlendSpaceImporter : public IImporter {
public:
	IBlendSpaceImporter(const FString& FileName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg):
		IImporter(FileName, FilePath, JsonObject, Package, OutermostPkg) {
	}

	virtual bool Import() override;
};