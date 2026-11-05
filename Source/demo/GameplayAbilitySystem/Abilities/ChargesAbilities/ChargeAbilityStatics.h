// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChargeAbilityStatics.generated.h"

class UGameplayAbility;

/**
 * Small helpers so UI blueprints can show cooldowns without casting abilities.
 *
 * Both functions take a plain UGameplayAbility (what GetGameplayAbilityFromSpecHandle
 * returns) and internally check whether it is a charge ability, so widgets do not need
 * a Cast node for every slot.
 */
UCLASS()
class DEMO_API UChargeAbilityStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Remaining cooldown of any ability:
	 *  - charge ability: seconds until the next charge comes back
	 *  - normal ability: the engine cooldown remaining (GetCooldownTimeRemaining)
	 * Returns 0 when nothing is cooling down.
	 */
	UFUNCTION(BlueprintPure, Category = "Charges")
	static float GetCooldownRemaining(const UGameplayAbility* Ability);

	/** True when the ability uses charges; OutAvailable/OutMax are only valid in that case. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	static bool GetChargeInfo(const UGameplayAbility* Ability, int32& OutAvailable, int32& OutMax);

	/**
	 * Remaining cooldown as a 0..1 fraction (1 = just started, 0 = ready).
	 * Used by the vertical cooldown sweep in the HUD; returns 0 when nothing is cooling.
	 */
	UFUNCTION(BlueprintPure, Category = "Charges")
	static float GetCooldownFraction(const UGameplayAbility* Ability);
};
