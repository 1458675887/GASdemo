// Fill out your copyright notice in the Description page of Project Settings.

#include "ChargeAbilityStatics.h"

#include "Abilities/GameplayAbility.h"
#include "ChargesAbilityBase.h"
#include "GameplayEffect.h"

namespace
{
	/** Static duration of a GameplayEffect at the given level (0 when it is not a simple magnitude). */
	float GetEffectDuration(const UGameplayEffect* Effect, float Level)
	{
		if (!Effect)
		{
			return 0.f;
		}

		float Magnitude = 0.f;
		return Effect->DurationMagnitude.GetStaticMagnitudeIfPossible(Level, Magnitude) ? Magnitude : 0.f;
	}
}

float UChargeAbilityStatics::GetCooldownRemaining(const UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return 0.f;
	}

	if (const UChargesAbilityBase* ChargeAbility = Cast<UChargesAbilityBase>(Ability))
	{
		return ChargeAbility->GetChargeCooldownRemaining();
	}

	const float EngineCooldown = Ability->GetCooldownTimeRemaining();
	return FMath::Max(0.f, EngineCooldown);
}

bool UChargeAbilityStatics::GetChargeInfo(const UGameplayAbility* Ability, int32& OutAvailable, int32& OutMax)
{
	OutAvailable = 0;
	OutMax = 0;

	if (const UChargesAbilityBase* ChargeAbility = Cast<UChargesAbilityBase>(Ability))
	{
		OutAvailable = ChargeAbility->GetAvailableCharges();
		OutMax = ChargeAbility->GetMaxCharges();
		return true;
	}

	return false;
}

float UChargeAbilityStatics::GetCooldownFraction(const UGameplayAbility* Ability)
{
	const float Remaining = GetCooldownRemaining(Ability);
	if (Remaining <= 0.f)
	{
		return 0.f;
	}

	float Total = 0.f;

	if (const UChargesAbilityBase* ChargeAbility = Cast<UChargesAbilityBase>(Ability))
	{
		// Charge abilities: the level table wins, otherwise the charge GE duration.
		Total = ChargeAbility->GetCooldownForCurrentLevel();
		if (Total <= 0.f && ChargeAbility->ChargeSpentEffectClass)
		{
			Total = GetEffectDuration(ChargeAbility->ChargeSpentEffectClass->GetDefaultObject<UGameplayEffect>(),
				static_cast<float>(ChargeAbility->GetAbilityLevel()));
		}
	}
	else
	{
		// Normal abilities: the ability cooldown GE duration.
		Total = GetEffectDuration(Ability->GetCooldownGameplayEffect(), static_cast<float>(Ability->GetAbilityLevel()));
	}

	return (Total > 0.f) ? FMath::Clamp(Remaining / Total, 0.f, 1.f) : 0.f;
}
