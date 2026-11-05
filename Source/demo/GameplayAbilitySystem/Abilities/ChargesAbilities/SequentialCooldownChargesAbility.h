// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChargesAbilityBase.h"
#include "SequentialCooldownChargesAbility.generated.h"

/**
 * Sequential recharge model.
 *
 * Charges are spent one by one without starting any timer; the cooldown starts when the
 * last charge is gone (RefreshOnSuccessfulApplication on the GE means it starts from the
 * last use) and restores every charge at once when it ends.
 *
 * There is no extra logic here on purpose - the model is defined by the charge GE you
 * assign on the ability (a BP child of USequentialCooldownChargeEffect).
 */
UCLASS()
class DEMO_API USequentialCooldownChargesAbility : public UChargesAbilityBase
{
	GENERATED_BODY()

public:
	USequentialCooldownChargesAbility();

	/** True while every charge is spent (global cooldown running). */
	UFUNCTION(BlueprintPure, Category = "Charges")
	bool IsInGlobalCooldown() const { return GetSpentCharges() >= GetMaxCharges(); }

	/** Seconds until all charges come back; 0 when not in global cooldown. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	float GetGlobalCooldownRemaining() const { return IsInGlobalCooldown() ? GetChargeCooldownRemaining() : 0.f; }
};