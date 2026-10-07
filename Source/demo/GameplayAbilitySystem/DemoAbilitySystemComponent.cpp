// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoAbilitySystemComponent.h"
#include "Character/BasePlayerCharacter.h"

UDemoAbilitySystemComponent::UDemoAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDemoAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	//发送一个gameplay事件，说明技能已经改变，这样UI就会更新
	//角色是被转成基础角色类的拥有者
	ABasePlayerCharacter* Character = Cast<ABasePlayerCharacter>(GetOwner());

	//如果角色无效
	if (!Character)
	{
		return;
	}

	//技能是否改变的布尔值
	bool bAbilitiesChanged = false;
	//能力数量不相等
	if (LastActivetableAbilities.Num() != ActivatableAbilities.Items.Num())
	{
		bAbilitiesChanged = true;
	}
	else
	{
		//遍历能力比对是不是相同
		for (int32 i = 0;i < LastActivetableAbilities.Num();i++)
		{
			if (LastActivetableAbilities[i].Ability != ActivatableAbilities.Items[i].Ability)
			{
				bAbilitiesChanged = true;
				break;
			}

		}
	}

	//如果改变了
	if (bAbilitiesChanged)
	{
		//调用角色的发送技能变更事件
		Character->SendAbilitiesChangedEvent();
		//更新可激活能力的数组
		LastActivetableAbilities = ActivatableAbilities.Items;
	}

	
}