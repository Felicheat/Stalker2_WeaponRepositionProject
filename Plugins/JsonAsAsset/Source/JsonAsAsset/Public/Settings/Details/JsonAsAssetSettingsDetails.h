// Copyright JAA Contributors 2024-2025

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UJsonAsAssetSettings;

// Details For JsonAsAssetSettings
class FJsonAsAssetSettingsDetails final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	// Customization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	void EditConfiguration(TWeakObjectPtr<UJsonAsAssetSettings> Settings, IDetailLayoutBuilder& DetailBuilder);
	void EditEncryption(TWeakObjectPtr<UJsonAsAssetSettings> Settings, IDetailLayoutBuilder& DetailBuilder);
};