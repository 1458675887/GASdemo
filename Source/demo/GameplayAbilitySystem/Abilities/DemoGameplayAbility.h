// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DemoGameplayAbility.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAbilityInputID :uint8
{
	None UMETA(DisplayName = "None"), //0
	PrimaryAbility UMETA(DisplayName = "PrimaryAbility"), //1
	SecondaryAbility UMETA(DisplayName = "SecondaryAbility"), //2
	DefensiveAbility UMETA(DisplayName = "DefensiveAbility"), //3
	MovementAbility UMETA(DisplayName = "MovementAbility"), //4
};


UCLASS()
class DEMO_API UDemoGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDemoGameplayAbility();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShouldShowInAbilitiesBar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	bool AutoActivateWhenGranted = false;

	//变量类型是枚举EAbilityInputID，默认为None（没有输入ID）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SetAbilityLevel(int32 NewLevel);
	
private:
	//是否有玩家控制器
	UFUNCTION(BlueprintCallable,Category="Helpers")
	bool HasPC()const;
};
