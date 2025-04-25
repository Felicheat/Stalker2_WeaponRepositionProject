// Copyright JAA Contributors 2024-2025

#pragma once

#include "Importers/Constructor/Importer.h"
#include "UObject/StructOnScope.h"

class IDataTableImporter : public IImporter {
public:
	using FTableRowMap = TMap<FName, TSharedPtr<class FStructOnScope>>;

	IDataTableImporter(const FString& FileName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg):
		IImporter(FileName, FilePath, JsonObject, Package, OutermostPkg) {
	}

	virtual bool Import() override;
};
