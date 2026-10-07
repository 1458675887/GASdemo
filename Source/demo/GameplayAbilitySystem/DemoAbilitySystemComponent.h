// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DemoAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API UDemoAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
protected:
	//存上次可激活能力的数组
	TArray<FGameplayAbilitySpec>LastActivetableAbilities;
public:
	UDemoAbilitySystemComponent();
protected:
	void OnRep_ActivateAbilities()override;
};
