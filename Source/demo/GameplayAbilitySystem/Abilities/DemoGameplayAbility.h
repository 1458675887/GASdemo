// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DemoGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API UDemoGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDemoGameplayAbility();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShouldShowInAbilitiesBar = false;
private:
	//是否有玩家控制器
	UFUNCTION(BlueprintCallable,Category="Helpers")
	bool HasPC()const;
};
