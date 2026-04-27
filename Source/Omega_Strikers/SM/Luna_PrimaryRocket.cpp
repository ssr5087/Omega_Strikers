// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_PrimaryRocket.h"

#include "Components/BoxComponent.h"
#include "Core/CoreBall.h"


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
	
	FTimerHandle SpeedChanger;
	GetWorld()->GetTimerManager().SetTimer(SpeedChanger, [this]()->void {Speed = 5000.f;}, 0.35f, false);
	// 타이머 써서 몇 초 뒤에 사라지게 or 숨겨놓고 충돌 다 꺼놓고 다음에 소환할 때 또?(x) 그러면 결국 Luna에서도 몇 개 뜯어 고쳐야 함 나아아아ㅏ중에 리팩토링 할 시간 있으면 그 때 하자
}

// Called every frame
void ALuna_PrimaryRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * DeltaTime * Speed);
}

void ALuna_PrimaryRocket::InitRocket(float Owner_Power, AActor* InOwnerActor, EOSTeam InTeamSide)
{
	Luna_Power = Owner_Power;
	OwnerActorRef = InOwnerActor;
	
	// 데이터 적용 필요
	ImpactData.TeamSide = InTeamSide;
	ImpactData.Direction = FVector2D(GetActorForwardVector().X, GetActorForwardVector().Y).GetSafeNormal();
	ImpactData.CoreKnockbackPower = 3390.f;
	ImpactData.PlayerKnockbackPower = 1500.f;
	ImpactData.PlayerDamage = 100.0f;
}

// 충돌 시 데미지 전달해주는 로직 넣기
void ALuna_PrimaryRocket::OnRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == OwnerActorRef) {return;}
	// OtherActor가 존재하고 그 놈이 인터페이스 구현했으면
	if (OtherActor && OtherActor->Implements<UOSImpactReceiver>())
	{
		// 그 대상에 대해 인터페이스 함수를 실행해라
		bool bIsSuccess = IOSImpactReceiver::Execute_ReceiveImpact(OtherActor, ImpactData, OwnerActorRef);
		
		auto name = OtherActor->GetName();
		if (bIsSuccess)
		{
			this->Destroy();
		}
	}
}

