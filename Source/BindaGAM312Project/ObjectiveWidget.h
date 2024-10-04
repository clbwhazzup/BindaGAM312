// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveWidget.generated.h"

/**
 * 
 */
UCLASS()
class BINDAGAM312PROJECT_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	// Declaration of functions implemented in the Blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void UpdatematObj(float matsCollected);
	
	UFUNCTION(BlueprintImplementableEvent)
		void UpdatebuildObj(float objectsBuilt);
};
