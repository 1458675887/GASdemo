// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CombatAttributeSet.generated.h"


/**
 * 
 */
UCLASS()
class DEMO_API UCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UCombatAttributeSet();

	//护甲值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, Armor);
	//最大护甲值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_MaxArmor)
	FGameplayAttributeData MaxArmor;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, MaxArmor);

	//力量值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, Strength);
	//最大力量值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStrength)
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, MaxStrength);

protected:
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Armor, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxArmor(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxArmor, OldValue);
	}

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Strength, OldValue);
	}
	UFUNCTION()
	void OnRep_MaxStrength(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxStrength, OldValue);
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
};
