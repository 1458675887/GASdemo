// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	Health = 100.f;
	MaxHealth = 100.f;
	Stamina = 100.f;
	MaxStamina = 100.f;
	Damage = 0.f;
	Shield = 0.f;
	MaxShield = 100.f;
}

void UBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*COND_None无条件同步，同步给所有客户端，包括有该对象的客户端和没有的客户端
	REPNOTIFY_Always每次收到数值，不论数值是否变化，都触发OnRep回调*/
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}


void UBasicAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//检查刚刚发生变化的属性
	//如果是生命值变化了
	if (Attribute == GetHealthAttribute())
	{
		//属性钳制
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	//如果是耐力变化了
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}
}

//需要包含头文件GameplayEffectExtension.h
void UBasicAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	//如果是伤害
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		//存一下旧伤害值，GetDamage()等价于Data.EvaluatedData.Magnitude
		float TotalDamage = GetDamage();
		//重置伤害值以便下次使用
		SetDamage(0.f);

		//当前护盾值
		float CurrentShield = GetShield();
		if (CurrentShield > 0.f) 
		{
			SetShield(CurrentShield - TotalDamage);
			float RemainingDamage = TotalDamage - CurrentShield;
			
			//还有剩余伤害
			if (RemainingDamage > 0.f)
			{
				SetHealth(GetHealth() - RemainingDamage);
			}
		}
		else
		{
			//设置生命值
			SetHealth(GetHealth() - TotalDamage);
		}

		

		//如果资产标签里有Effects.HitReaction
		if (Data.EffectSpec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("Effects.HitReaction")))
			&& Data.EvaluatedData.Magnitude != 0.f)
		{
			//创建一个标签容器
			FGameplayTagContainer HitReactionTagContainer;
			//只放一个标签
			HitReactionTagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.HitReaction")));
			//获取ASC，尝试按类激活能力
			GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(HitReactionTagContainer);
		}

	}
	//如果是生命值被修改
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		/*把生命值设置为当前生命值
		实际是调用了上面的PreAttributeChange进行钳制*/
		SetHealth(GetHealth());
	}
	//如果是耐力被修改
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(GetStamina());
	}
}

void UBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	//变化的属性是生命值且新值小于等于0，就尝试激活死亡能力
	if (Attribute == GetHealthAttribute() && NewValue <= 0.f)
	{
		FGameplayTagContainer DeathAbilityTagContainer;
		DeathAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Death")));
		GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(DeathAbilityTagContainer);
	}
}