// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ChargesGameplayEffectBase.generated.h"

/**
 * Base GameplayEffect for the "individual cooldown" charge model.
 *
 * One stack == one charge that is currently recharging, so the stack count is
 * "charges used". GAS keeps a single timer per active effect, which is expressed
 * here as first-in-first-out: the oldest stack expires first and ClearSingleStack
 * restarts the timer for the remaining stacks (sequential charge recovery).
 *
 * Configure per ability with a BP child of this class: set StackLimitCount
 * (= the ability's MaxCharges) and an asset tag (e.g. Charge.Dash) the ability queries.
 */
UCLASS(Abstract, Blueprintable)
class DEMO_API UIndividualCooldownChargeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UIndividualCooldownChargeEffect();
};

/**
 * Base GameplayEffect for the "sequential cooldown" charge model.
 *
 * Every use adds a stack and refreshes the timer, so the cooldown runs from the LAST
 * use; when it expires ClearEntireStack removes every stack at once (all charges back).
 *
 * Configure per ability with a BP child of this class: set StackLimitCount
 * (= the ability's MaxCharges) and an asset tag (e.g. Charge.Dash).
 */
UCLASS(Abstract, Blueprintable)
class DEMO_API USequentialCooldownChargeEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USequentialCooldownChargeEffect();
};
