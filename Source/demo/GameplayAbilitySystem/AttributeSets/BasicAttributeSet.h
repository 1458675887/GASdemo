// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	//构造函数
	UBasicAttributeSet();

	//生命值
	//ReplicatedUsing表示这个变量需要网络复制，并且在客户端收到新值后要执行 OnRep_Health 这个函数
	UPROPERTY(BlueprintReadonly,Category="Attributes",ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	//这个宏就是封装好的get,set,init和获取health对象本身四个函数（每个获取到的都包含基础值和当前值）。需要包含ASC头文件
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health);
	//最大生命值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth);

	//耐力值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Stamina);
	//最大耐力值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxStamina);

	//伤害，不需要复制，所以删掉复制宏
	UPROPERTY(BlueprintReadonly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Damage);

	//护盾值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Shield);
	//最大护盾值
	UPROPERTY(BlueprintReadonly, Category = "Attributes", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxShield);

public:
	/*处理生命值变量的网络同步
	接收一个游戏属性数据类型 的旧值的引用，作为参数*/
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue)const 
	{
		//属性从服务器复制到客户端时，比对并通知变化，为预测系统提供支持
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
	}

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Stamina, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxStamina, OldValue);
	}

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Shield, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue)const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxShield, OldValue);
	}

	//告诉引擎哪些属性需要网络同步。在实现里声明同步方式
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/*告诉我哪些属性（Attribute）发生了变化，该属性的新值（NewValue）是多少
	每当属性值被手动设置时，这个函数被调用
	注意是手动设置*/
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)override;

	//属性被游戏效果修改时，这个函数被调用
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)override;

	//某个属性发生变化时，引擎自动调用
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)override;
};
