// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoGameplayAbility.h"

UDemoGameplayAbility::UDemoGameplayAbility()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));

	//角色有死亡标签时，技能会被屏蔽
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
}

bool UDemoGameplayAbility::HasPC()const
{
	const APawn* PawnObject = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!PawnObject)
	{
		return false;
	}
	//如果是pawn就获取控制器，再返回是不是玩家控制器
	return PawnObject->GetController()->IsA<APlayerController>();
}