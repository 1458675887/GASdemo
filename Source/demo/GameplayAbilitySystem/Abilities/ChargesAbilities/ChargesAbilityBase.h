// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "../DemoGameplayAbility.h"
#include "ChargesAbilityBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChargesChanged);

/**
 * One row of the per-level recharge table (row name = talent level, e.g. "1", "2").
 * When a row exists for the current ability level it overrides the GE duration.
 */
USTRUCT(BlueprintType)
struct FChargeCooldownLevelRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Recharge time for this talent level, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	float Cooldown = 3.f;

	/** Charges granted at this talent level. 0 = keep the ability default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	int32 MaxCharges = 0;
};

/**
 * Base class for charge-based abilities.
 *
 * Charges are not stored here: one stack of ChargeSpentEffectClass means "one charge
 * that is currently unavailable", so GAS owns replication, expiry and stack counting.
 * The ability only has to
 *   - block activation while every charge is spent (CanActivateAbility),
 *   - commit cost and add one stack, atomically, when it activates (ActivateAbility).
 *
 * The charge is applied only after CommitAbility() succeeded, so blueprints never need
 * a refund step: they just implement what the ability does.
 *
 * The model (individual vs sequential recharge) lives entirely in the GE: use a BP child
 * of UIndividualCooldownChargeEffect or USequentialCooldownChargeEffect.
 */
UCLASS(Abstract)
class DEMO_API UChargesAbilityBase : public UDemoGameplayAbility
{
	GENERATED_BODY()

public:
	UChargesAbilityBase();

	/** Charge GE (BP child of one of the ChargeEffect base classes). One stack = one spent charge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges")
	TSubclassOf<UGameplayEffect> ChargeSpentEffectClass;

	/** Asset tags used to find the active charge GE (must be set on the GE). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges")
	FGameplayTagContainer ChargeEffectTags;

	/**
	 * Default number of charges, used when the level table has no MaxCharges for the
	 * current level. The effective value is clamped to the charge GE StackLimitCount,
	 * which therefore acts as the ceiling (set it to the max charges at max level).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges", meta = (ClampMin = "1"))
	int32 MaxCharges = 2;

	/** Optional per-level recharge table. Row name = level, or "{CooldownRowPrefix}_{Level}". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges|Table")
	TObjectPtr<UDataTable> CooldownByLevelTable = nullptr;

	/** Optional row prefix so several abilities can share one table ("Dash" -> Dash_1, Dash_2...). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges|Table")
	FString CooldownRowPrefix;

	/** Fired when charges may have changed (activation, end, or a charge came back). */
	UPROPERTY(BlueprintAssignable, Category = "Charges")
	FOnChargesChanged OnChargesChanged;

	/** Charges usable right now. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	int32 GetAvailableCharges() const;

	/**
	 * Maximum charges at the current ability level: the level table value, else MaxCharges,
	 * clamped to the charge GE StackLimitCount (that GE value is the ceiling, so it never
	 * has to be edited when a talent raises the charge count).
	 */
	UFUNCTION(BlueprintPure, Category = "Charges")
	int32 GetMaxCharges() const;

	/** Charges currently recharging (stack count of the charge GE). */
	UFUNCTION(BlueprintPure, Category = "Charges")
	int32 GetSpentCharges() const;

	/** Seconds until the next charge comes back; 0 when full. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	float GetChargeCooldownRemaining() const;

	/** True when at least one charge is usable. */
	UFUNCTION(BlueprintPure, Category = "Charges")
	bool HasAvailableCharge() const { return GetAvailableCharges() > 0; }

	/** Recharge time for the current ability level; 0 means "use the GE duration". */
	UFUNCTION(BlueprintPure, Category = "Charges")
	float GetCooldownForCurrentLevel() const;

	/**
	 * Authority only: clears every spent charge so all charges are available again.
	 * Use it when a talent level raises MaxCharges and the new charges should be usable
	 * immediately instead of waiting for the next recharge.
	 */
	UFUNCTION(BlueprintCallable, Category = "Charges")
	void RefillCharges();

	/**
	 * When true, raising the ability level above the level the charges were last seen with
	 * refills every charge automatically (no blueprint wiring needed).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charges")
	bool bRefillChargesOnLevelUp = true;

	/** Kept so the level driven charge count is refreshed automatically (see above). */
	virtual void SetAbilityLevel(int32 NewLevel) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Adds one "spent charge" stack. Authority only. */
	bool ApplyChargeStack();

	/** Active charge GE handles. */
	TArray<FActiveGameplayEffectHandle> GetChargeEffectHandles() const;

	/** Row of CooldownByLevelTable for the current ability level (may be null). */
	const struct FChargeCooldownLevelRow* FindLevelRow() const;

	void BroadcastChargesChanged() { OnChargesChanged.Broadcast(); }

private:
	/** Max charges seen at the last activation/level change, for the auto refill. */
	int32 LastKnownMaxCharges = -1;

	/** Refills when the level driven charge count grew since the last check. */
	void RefillIfChargesGrew();
};
