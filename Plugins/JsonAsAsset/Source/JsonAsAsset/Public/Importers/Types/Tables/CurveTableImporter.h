// Copyright JAA Contributors 2024-2025

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Engine/CurveTable.h"

class CCurveTableDerived : public UCurveTable {
public:
	void AddRow(FName Name, FRealCurve* Curve);
	void ChangeTableMode(ECurveTableMode Mode);
};

class ICurveTableImporter : public IImporter {
public:
	ICurveTableImporter(const FString& FileName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg):
		IImporter(FileName, FilePath, JsonObject, Package, OutermostPkg) {
	}

	virtual bool Import() override;
};
