// Fill out your copyright notice in the Description page of Project Settings.

#include "ChargesAbilityBase.h"

#include "AbilitySystemComponent.h"

UChargesAbilityBase::UChargesAbilityBase()
{
	// Charges live in a GE, but the queries below still need a valid instance per actor.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Cooldown is handled by the charge GE, never by the ability cooldown system.
	CooldownGameplayEffectClass = nullptr;
}

TArray<FActiveGameplayEffectHandle> UChargesAbilityBase::GetChargeEffectHandles() const
{
	TArray<FActiveGameplayEffectHandle> Handles;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || ChargeEffectTags.IsEmpty())
	{
		return Handles;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllEffectTags(ChargeEffectTags);
	return ASC->GetActiveEffects(Query);
}

int32 UChargesAbilityBase::GetSpentCharges() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return 0;
	}

	int32 Spent = 0;
	for (const FActiveGameplayEffectHandle& Handle : GetChargeEffectHandles())
	{
		if (const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle))
		{
			Spent += ActiveEffect->Spec.GetStackCount();
		}
	}
	return Spent;
}

int32 UChargesAbilityBase::GetAvailableCharges() const
{
	return FMath::Clamp(GetMaxCharges() - GetSpentCharges(), 0, GetMaxCharges());
}

float UChargesAbilityBase::GetChargeCooldownRemaining() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return 0.f;
	}

	const UWorld* World = ASC->GetWorld();
	if (!World)
	{
		return 0.f;
	}

	const float Now = World->GetTimeSeconds();

	float Shortest = 0.f;
	bool bFound = false;
	for (const FActiveGameplayEffectHandle& Handle : GetChargeEffectHandles())
	{
		if (const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle))
		{
			const float Remaining = ActiveEffect->GetTimeRemaining(Now);
			if (Remaining > 0.f && (!bFound || Remaining < Shortest))
			{
				Shortest = Remaining;
				bFound = true;
			}
		}
	}

	return bFound ? Shortest : 0.f;
}

const FChargeCooldownLevelRow* UChargesAbilityBase::FindLevelRow() const
{
	if (!CooldownByLevelTable)
	{
		return nullptr;
	}

	const int32 Level = FMath::Max(1, GetAbilityLevel());
	const FString RowName = CooldownRowPrefix.IsEmpty()
		? FString::FromInt(Level)
		: FString::Printf(TEXT("%s_%d"), *CooldownRowPrefix, Level);

	return CooldownByLevelTable->FindRow<FChargeCooldownLevelRow>(FName(*RowName), TEXT("ChargesAbility"), false);
}

int32 UChargesAbilityBase::GetMaxCharges() const
{
	// Level table first, so a talent level can raise the number of charges.
	int32 Result = FMath::Max(1, MaxCharges);
	if (const FChargeCooldownLevelRow* Row = FindLevelRow())
	{
		if (Row->MaxCharges > 0)
		{
			Result = Row->MaxCharges;
		}
	}

	// The GE StackLimitCount is the ceiling, so it never has to be edited per level.
	if (ChargeSpentEffectClass)
	{
		if (const UGameplayEffect* EffectCDO = ChargeSpentEffectClass->GetDefaultObject<UGameplayEffect>())
		{
			if (EffectCDO->StackLimitCount > 0)
			{
				Result = FMath::Min(Result, EffectCDO->StackLimitCount);
			}
		}
	}

	return FMath::Max(1, Result);
}

float UChargesAbilityBase::GetCooldownForCurrentLevel() const
{
	const FChargeCooldownLevelRow* Row = FindLevelRow();
	return (Row && Row->Cooldown > 0.f) ? Row->Cooldown : 0.f;
}

void UChargesAbilityBase::SetAbilityLevel(int32 NewLevel)
{
	const int32 PreviousMax = GetMaxCharges();
	Super::SetAbilityLevel(NewLevel);

	const int32 NewMax = GetMaxCharges();
	if (bRefillChargesOnLevelUp && NewMax > PreviousMax)
	{
		RefillCharges();
	}

	LastKnownMaxCharges = NewMax;
}

void UChargesAbilityBase::RefillIfChargesGrew()
{
	const int32 Current = GetMaxCharges();
	if (bRefillChargesOnLevelUp && LastKnownMaxCharges >= 0 && Current > LastKnownMaxCharges)
	{
		RefillCharges();
	}
	LastKnownMaxCharges = Current;
}

void UChargesAbilityBase::RefillCharges()
{
	if (!K2_HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Removing the charge GE wipes its stacks, which means every charge is available again.
	const TArray<FActiveGameplayEffectHandle> Handles = GetChargeEffectHandles();
	if (Handles.Num() == 0)
	{
		return;
	}

	for (const FActiveGameplayEffectHandle& Handle : Handles)
	{
		ASC->RemoveActiveGameplayEffect(Handle, -1);
	}

	BroadcastChargesChanged();
}

bool UChargesAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Blocked tags / missing ASC are still handled by the parent class.
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Every charge spent means the ability is recharging as a whole.
	return HasAvailableCharge();
}

void UChargesAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Safety net: a level up may have raised the charge count through another path.
	RefillIfChargesGrew();

	// Commit cost (and cooldown, but there is none) first: a failure must not eat a charge.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Authority owns the charge stacks; clients get them through GAS replication.
	if (K2_HasAuthority())
	{
		ApplyChargeStack();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UChargesAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	BroadcastChargesChanged();
}

bool UChargesAbilityBase::ApplyChargeStack()
{
	if (!ChargeSpentEffectClass)
	{
		return false;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChargeSpentEffectClass, GetAbilityLevel());
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return false;
	}

	// The per-level table (when present) overrides the duration baked into the GE.
	const float Cooldown = GetCooldownForCurrentLevel();
	if (Cooldown > 0.f)
	{
		SpecHandle.Data->SetDuration(Cooldown, true);
	}

	ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
		GetCurrentActivationInfoRef(), SpecHandle);

	BroadcastChargesChanged();
	return true;
}
