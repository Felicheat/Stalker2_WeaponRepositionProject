// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Stalker2Globals.h"
#include "WeatherPresetDataAsset.generated.h"


UCLASS()
class STALKER2_API UWeatherPresetDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FWeatherState WeatherState;
};
