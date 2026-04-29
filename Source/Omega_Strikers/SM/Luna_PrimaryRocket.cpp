// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_PrimaryRocket.h"

#include "Luna.h"
#include "PlayerBase.h"
#include "Components/BoxComponent.h"
#include "Core/CoreBall.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SSR/CharacterSkill.h"


// Sets default values
ALuna_PrimaryRocket::ALuna_PrimaryRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(150.0f, 70.0f, 70.0f));
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALuna_PrimaryRocket::OnRocketOverlap);
	
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Luna/Luna_Rocket.Luna_Rocket'"));
	if (TempMesh.Succeeded())
	{
		RocketMesh->SetStaticMesh(TempMesh.Object);
	}
	RocketMesh->SetupAttachment(BoxComp);
	RocketMesh->SetRelativeLocationAndRotation(FVector(10.0f, 0.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ALuna_PrimaryRocket::BeginPlay()
{
	Super::BeginPlay();
	
	
	// 소환 후 0.35초 이후에 가속, Impact Data도 같이 갱신
	FTimerHandle SpeedChanger;
	GetWorld()->GetTimerManager().SetTimer(SpeedChanger,
		[this]()->void
		{
			// 가속
			Speed = 5000.f;
			
			// 데이터 셋 불러오기
			// Owner 정보 실패 시 방어 코드
			if (!IsValid(OwnerActorRef)) {return;}
			
			// 성공 시 데이터 셋 불러오기
			FCharacterSkill* Skill = OwnerActorRef->GetSkillData(FName(TEXT("Luna_Primary_Accelerated")));
			
			// 데이터 셋 로드 실패 시 리턴
			if (!Skill) {return;}
			
			// ImpactData를 데이터 셋으로부터 계산하여 설정
			ImpactData = OwnerActorRef->MakeImpactData(*Skill);
			ImpactData.Direction = OwnerActorRef->PrimaryDir;
			
		}, 0.35f, false);
	
	
	// 타이머 써서 몇 초 뒤에 사라지게 or 숨겨놓고 충돌 다 꺼놓고 다음에 소환할 때 또?(x) 그러면 결국 Luna에서도 몇 개 뜯어 고쳐야 함 나아아아ㅏ중에 리팩토링 할 시간 있으면 그 때 하자
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer,
		[this]()->void
		{
			this->Destroy();
		}, 10.f, false);
}

// Called every frame
void ALuna_PrimaryRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * DeltaTime * Speed);
}

void ALuna_PrimaryRocket::InitRocket(AActor* InOwnerActor)
{
	// 데이터 셋 불러올 PlayerBase(Luna)로 캐스팅
	OwnerActorRef = Cast<ALuna>(InOwnerActor);
	
	// 실패 시 리턴
	if (!IsValid(OwnerActorRef)) {return;}
	
	// 성공 시 데이터 셋 불러오기
	FCharacterSkill* Skill = OwnerActorRef->GetSkillData(FName(TEXT("Luna_Primary_CloseRange")));
	
	// 데이터 셋 로드 실패 시 리턴
	if (!Skill) {return;}
	
	// ImpactData를 데이터 셋으로부터 계산하여 설정
	ImpactData = OwnerActorRef->MakeImpactData(*Skill);
	ImpactData.Direction = OwnerActorRef->PrimaryDir;
}

// 충돌 시 데미지 전달해주는 로직 넣기
void ALuna_PrimaryRocket::OnRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || OtherActor == OwnerActorRef) {return;}
	// OtherActor가 존재하고 그 놈이 인터페이스 구현했으면
	if (OtherActor && OtherActor->Implements<UOSImpactReceiver>())
	{
		// 그 대상에 대해 인터페이스 함수를 실행해라
		bool bIsSuccess = IOSImpactReceiver::Execute_ReceiveImpact(OtherActor, ImpactData, OwnerActorRef);
		
		if (bIsSuccess)
		{
			LOG_SM_E(TEXT("%f, %f"), ImpactData.PlayerDamage, ImpactData.PlayerKnockbackPower);
			
			// 파괴 이후에 BeginPlay에서 돌린 타이머가 살아있으면 오류나니까 미리 해제해주기
			if (GetWorld()->GetTimerManager().IsTimerActive(DestroyTimer))
			{
				GetWorld()->GetTimerManager().ClearTimer(DestroyTimer);
			}
			
			this->Destroy();
		}
	}
}

