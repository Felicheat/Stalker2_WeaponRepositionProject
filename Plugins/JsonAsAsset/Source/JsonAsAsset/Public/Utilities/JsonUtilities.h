// Copyright JAA Contributors 2024-2025

#pragma once

struct FExportData {
	FExportData(const FName Type, const FName Outer, const TSharedPtr<FJsonObject>& Json) {
		this->Type = Type;
		this->Outer = Outer;
		this->Json = Json.Get();
	}

	FExportData(const FString& Type, const FString& Outer, const TSharedPtr<FJsonObject>& Json) {
		this->Type = FName(Type);
		this->Outer = FName(Outer);
		this->Json = Json.Get();
	}

	FExportData(const FString& Type, const FString& Outer, FJsonObject* Json) {
		this->Type = FName(Type);
		this->Outer = FName(Outer);
		this->Json = Json;
	}

	FName Type;
	FName Outer;
	FJsonObject* Json;
};