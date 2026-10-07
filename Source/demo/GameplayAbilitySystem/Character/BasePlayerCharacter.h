// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BasePlayerCharacter.generated.h"

//能力系统接口只有一个函数，就是获取能力系统组件
UCLASS()
class DEMO_API ABasePlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABasePlayerCharacter();
	//类似于创建ASC类
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;   //指向UAbilitySystemComponent的指针变量AbilitySystemComponent

	//和添加ASC一样添加属性集，使其能被所有角色使用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributeSet;

	//添加战斗属性集
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UCombatAttributeSet* CombatAttributeSet;

protected:
	//设置玩家复制模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	//开始运行时自动授予的技能类数组
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//敌人被控制器附身时的情况
	virtual void PossessedBy(AController* NewController)override;

	//玩家状态复制回调。玩家状态被复制时调用的函数（玩家加服务器/玩家重生）
	virtual void OnRep_PlayerState()override;

	//死亡标签添加或移除的回调，传入标签本身和数量
	virtual void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	//注册【死亡标签】事件监听（只在运行时调用）
	//放在运行时而不是构造函数，是为了避免 CDO（模板对象）也被绑定：
	//否则 CDO 一旦被判定死亡，HandleDeath 就会把死亡状态写进蓝图默认值
	void RegisterDeadTagListener();

	//死亡功能函数
	//蓝图原生事件，C++给出默认值，蓝图可重写
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category="Damage")
	void HandleDeath();
	//EndPlay：本次运行结束时（PIE 停止/角色销毁）把死亡时改过的碰撞设置还原回去
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//缓存死亡前的碰撞设置（只在第一次死亡时记录）
	void CachePreDeathCollision();

	//还原死亡前的碰撞设置
	void RestorePreDeathCollision();

	//是否已经缓存过死亡前的碰撞设置
	bool bHasCachedPreDeathCollision = false;

	//死亡前：胶囊体的碰撞开关和碰撞预设
	ECollisionEnabled::Type CachedCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FName CachedCapsuleCollisionProfile = NAME_None;

	//死亡前：网格体的碰撞开关、碰撞预设、是否模拟物理
	ECollisionEnabled::Type CachedMeshCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FName CachedMeshCollisionProfile = NAME_None;
	bool bCachedMeshSimulatePhysics = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//重写父类的获取ASC的接口
	virtual UAbilitySystemComponent* GetAbilitySystemComponent()const override;

	//授予一组技能
	//因为要在蓝图中调用，所以写反射宏
	UFUNCTION(BlueprintCallable,Category="AbilitySystem")
	/*TArray<TSubclassOf<UGameplayAbility>>每个元素都是UGameplayAbility的子类的数组。意思就是多个技能的数组
	返回一个句柄类型的数组是仿照give ability函数的返回句柄*/
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>>AbilitiesToGrant);

	//移除一组技能
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	/*不需要返回值
	仿照clear ability接收句柄作为参数*/
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);

	
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	//自动为技能改变创建事件标签，然后发送出去
	void SendAbilitiesChangedEvent();

	//server意为服务端PRC，reliable意为可靠传输
	UFUNCTION(Server,Reliable,BlueprintCallable, Category = "AbilitySystem")
	//参数不能用引用，这段代码运行在不同的机器上，内存地址不一样
	void ServerSendGameplayEventToSelf(FGameplayEventData EventData);
};
