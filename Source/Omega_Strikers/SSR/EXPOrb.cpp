// Fill out your copyright notice in the Description page of Project Settings.


#include "EXPOrb.h"

#include "EXPComponent.h"
#include "PlayerBase.h"
#include "Components/SphereComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"


// Sets default values
AEXPOrb::AEXPOrb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// 충돌
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	
	CollisionComp->SetSphereRadius(100.f);
	// ->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComp->SetCollisionObjectType(ECC_EXPOrb);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// 메쉬
	// Mesh->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	// Mesh->SetupAttachment(RootComponent);
	
	// 이벤트 바인딩
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AEXPOrb::OnOverlapBegin);
	
	// 네트워크
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AEXPOrb::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEXPOrb::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	LOG_SR_W(TEXT("EXP 오버랩 함수 실행"));
	
	// 서버에서만 관리
	if (!HasAuthority())
		return;
	
	APlayerBase* player = Cast<APlayerBase>(OtherActor);
	if (!player)
		return;
	
	// EXPComponent 가져오기
	if (player->EXPComp)
	{
		player->EXPComp->AddEXP(EXPAmount);
	}
	
	// 이펙트 넣어도 됨 (여기서 Multicast 가능합니다)
	
	// Orb 제거
	Destroy();
}

