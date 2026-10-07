// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "demo/GameplayAbilitySystem/DemoAbilitySystemComponent.h"
#include "../AttributeSets/BasicAttributeSet.h"
#include "../AttributeSets/CombatAttributeSet.h"

// Sets default values
ABasePlayerCharacter::ABasePlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//添加ASC
	//为UAbilitySystemComponent创建默认子对象AbilitySystemComponent。相当于将组件添加到角色中并命名为AbilitySystemComponent（TEXT里的）
	AbilitySystemComponent = CreateDefaultSubobject<UDemoAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	//开启ASC网络复制功能，复制到每一个客户端上
	AbilitySystemComponent->SetIsReplicated(true);
	//设置ASC的复制模式，复制模式决定哪些对象会被复制。因为要对玩家使用混合模式，敌人使用极简模式，所以要自己定义枚举类型
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	//为角色移动组件添加默认值
	//设置胶囊体组件大小
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	//角色朝向不跟随控制器
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//角色朝向与移动方向对齐
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//角色转向速度
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;  //跳跃初速度
	GetCharacterMovement()->AirControl = 0.35f;  //空中操控性
	GetCharacterMovement()->MaxWalkSpeed = 500.f;  //最大行走速度
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;  //最小模拟行走速度
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;  //地面刹车减速度
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;  //空中刹车减速度


	//添加基础属性集
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));

	//添加战斗属性集
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));
}

// Called when the game starts or when spawned
void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABasePlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		RegisterDeadTagListener();
		GrantAbilities(StartingAbilities);
	}
}

void ABasePlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		RegisterDeadTagListener();
	}
}
UAbilitySystemComponent* ABasePlayerCharacter::GetAbilitySystemComponent()const
{
	return AbilitySystemComponent;
}

TArray<FGameplayAbilitySpecHandle> ABasePlayerCharacter::GrantAbilities(
	TArray<TSubclassOf<UGameplayAbility>>AbilitiesToGrant)
{
	//如果没有ASC或者没有权限（只有服务器能授予技能）
	if (!AbilitySystemComponent||!HasAuthority())
	{
		//返回空数组
		return TArray<FGameplayAbilitySpecHandle>();
	}

	//创建一个技能规格句柄数组，保存授予过的技能的规格句柄
	TArray<FGameplayAbilitySpecHandle>AbilityHandles;

	//遍历技能数组，逐个授予
	for (TSubclassOf<UGameplayAbility> Ability : AbilitiesToGrant)
	{
		//SpecHandle就是要存入AbilityHandles里的
		
		/*获取ASC，调用giveability函数，这个函数需要传入一个规格spec作为参数
		规格需要传入参数，参数分别为(授予的技能，技能等级，输入绑定ID，授予对象)
		ID-1就是INDEX_NONE不绑定，授予对象就是当前这个角色基类*/
		FGameplayAbilitySpecHandle SpecHandle=AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability,1,-1,this));

		//SpecHandle存入AbilityHandles
		AbilityHandles.Add(SpecHandle);
	}
	SendAbilitiesChangedEvent();
	//返回技能句柄数组
	return AbilityHandles;
}

void ABasePlayerCharacter::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	//如果没有ASC或者没有权限（只有服务器能移除技能）
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}
	//遍历技能句柄数组
	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		//获取ASC，调用ClearAbility，传入技能句柄作为参数
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}

	SendAbilitiesChangedEvent();
}

void ABasePlayerCharacter::SendAbilitiesChangedEvent()
{
	//获取事件数据
	FGameplayEventData EventData;
	//创建事件标签   RequestGameplayTag请求标签   FName输入标签名称
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Abilities.Changed"));
	//事件发起者
	EventData.Instigator = this;
	//事件接收者
	EventData.Target = this;

	//直接用蓝图库的SendGameplayEventToActor   this是发送对象
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

//这是虚幻引擎创建复制事件的命名规范，在函数名后追加_Implementation。调用用原名
void ABasePlayerCharacter::ServerSendGameplayEventToSelf_Implementation(FGameplayEventData EventData)
{
	//和SendAbilitiesChangedEvent的区别是，这部分是客户端运行在服务器上的
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

//加上_Implementation声明如果没有蓝图实现，就用这个函数；如果有蓝图实现，就用蓝图的
void ABasePlayerCharacter::HandleDeath_Implementation()
{
	//CDO（蓝图模板）没有世界，改它的组件等于直接改蓝图默认值，
	//下一次运行用的就会是死亡后的状态，所以这里直接跳过
	if (IsTemplate())
	{
		return;
	}

	//先把死亡前的碰撞设置存起来，EndPlay 时还原
	CachePreDeathCollision();

	//获取网格体并模拟物理效果
	GetMesh()->SetSimulatePhysics(true);

	//获取网格体并启用碰撞。启用查询碰撞和物理碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//胶囊体设为无碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//禁用移动
	GetCharacterMovement()->DisableMovement();

	FVector Impulse = GetActorForwardVector() * -20000;
	Impulse.Z = 15000;
	//获取网格体并在其位置添加脉冲，脉冲为前向向量负值（击退）和向上向量（击飞）
	GetMesh()->AddImpulseAtLocation(Impulse, GetActorLocation());
}

void ABasePlayerCharacter::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	//新数量>0意味着添加了一个标签
	if (NewCount > 0)
	{
		//死亡功能
		HandleDeath();
	}
}

void ABasePlayerCharacter::RegisterDeadTagListener()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	/*RegisterGameplayTagEvent注册一个标签事件监听器，当指定Tag的叠加层数（Count）发生变化时触发回调。
	.AddUObject(this, &ABasePlayerCharacter::OnDeadTagChanged) 把当前对象（this）和它的成员函数绑定为回调。*/
	auto& DeadTagEvent = AbilitySystemComponent->RegisterGameplayTagEvent(
		FGameplayTag::RequestGameplayTag(FName("State.Dead")));

	//PossessedBy 和 OnRep_PlayerState 都会调用本函数，
	//先移除旧绑定，避免同一次标签变化触发多次 HandleDeath
	DeadTagEvent.RemoveAll(this);
	DeadTagEvent.AddUObject(this, &ABasePlayerCharacter::OnDeadTagChanged);
}

void ABasePlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//本次运行结束前，把死亡时改掉的碰撞设置还原回去
	RestorePreDeathCollision();

	Super::EndPlay(EndPlayReason);
}

void ABasePlayerCharacter::CachePreDeathCollision()
{
	//只缓存一次：重复触发死亡时，不允许用死亡后的状态覆盖掉原始状态
	if (bHasCachedPreDeathCollision)
	{
		return;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		CachedCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
		CachedCapsuleCollisionProfile = Capsule->GetCollisionProfileName();
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		CachedMeshCollisionEnabled = MeshComp->GetCollisionEnabled();
		CachedMeshCollisionProfile = MeshComp->GetCollisionProfileName();
		bCachedMeshSimulatePhysics = MeshComp->IsSimulatingPhysics();
	}

	bHasCachedPreDeathCollision = true;
}

void ABasePlayerCharacter::RestorePreDeathCollision()
{
	if (!bHasCachedPreDeathCollision)
	{
		return;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		//碰撞预设里包含碰撞开关、对象类型和所有通道的响应，优先用预设还原
		if (CachedCapsuleCollisionProfile != NAME_None)
		{
			Capsule->SetCollisionProfileName(CachedCapsuleCollisionProfile, false);
		}
		Capsule->SetCollisionEnabled(CachedCapsuleCollisionEnabled);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		//先还原是否模拟物理，再还原碰撞
		MeshComp->SetSimulatePhysics(bCachedMeshSimulatePhysics);
		if (CachedMeshCollisionProfile != NAME_None)
		{
			MeshComp->SetCollisionProfileName(CachedMeshCollisionProfile, false);
		}
		MeshComp->SetCollisionEnabled(CachedMeshCollisionEnabled);
	}

	bHasCachedPreDeathCollision = false;
}
