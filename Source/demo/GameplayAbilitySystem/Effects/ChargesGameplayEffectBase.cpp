// Fill out your copyright notice in the Description page of Project Settings.

#include "ChargesGameplayEffectBase.h"

UIndividualCooldownChargeEffect::UIndividualCooldownChargeEffect()
{
	// A charge is only unavailable while it recharges, i.e. always duration based.
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// One stack per used charge, all from the same source.
	StackingType = EGameplayEffectStackingType::AggregateBySource;

	// New stacks must not push back the recharge of the older ones.
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::NeverRefresh;

	// Oldest stack expires first, then the timer restarts for the remaining stacks:
	// first used, first restored.
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::RemoveSingleStackAndRefreshDuration;
}

USequentialCooldownChargeEffect::USequentialCooldownChargeEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	StackingType = EGameplayEffectStackingType::AggregateBySource;

	// Every use restarts the single timer, so the cooldown runs from the last use.
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;

	// When the timer ends, every charge is restored at once.
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack;
}
