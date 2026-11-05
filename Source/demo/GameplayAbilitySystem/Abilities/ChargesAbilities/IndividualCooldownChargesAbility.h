// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChargesAbilityBase.h"
#include "IndividualCooldownChargesAbility.generated.h"

/**
 * Individual recharge model.
 *
 * Every used charge recharges on its own timer: charges come back in the order they
 * were used (first used, first restored).
 *
 * There is no extra logic here on purpose - the model is defined by the charge GE you
 * assign on the ability (a BP child of UIndividualCooldownChargeEffect). This class only
 * marks the semantic and exposes a model-specific getter.
 */
UCLASS()
class DEMO_API UIndividualCooldownChargesAbility : public UChargesAbilityBase
{
	GENERATED_BODY()

public:
	UIndividualCooldownChargesAbility();

	/** Charges that are currently recharging. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	int32 GetRechargingCharges() const { return GetSpentCharges(); }
};