// Fill out your copyright notice in the Description page of Project Settings.
// 서버 Authoritative 네트워크 Core 물리

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"
#include "CoreBall.generated.h"

class USphereComponent;
// ═══════════════════════════════════════════════════════
// 코어 상태 열거형
// ═══════════════════════════════════════════════════════
UENUM(BlueprintType)
enum class ECoreState : uint8
{
	Idle      UMETA(DisplayName = "Idle"),       // 경기장 중앙 대기
	InPlay    UMETA(DisplayName = "InPlay"),     // 이동 중
	Stunned   UMETA(DisplayName = "Stunned"),    // 멈춤 상태이상
	Scored    UMETA(DisplayName = "Scored"),     // 골 연출 중
};

// ═══════════════════════════════════════════════════════
// 히트 이벤트 구조체 (Replicated)
// ═══════════════════════════════════════════════════════
USTRUCT(BlueprintType)
struct FCoreHitEvent
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector_NetQuantize  HitLocation;
	
	UPROPERTY()
	FVector_NetQuantize  HitDirection;
	
	UPROPERTY()
	float Power = 0.f;
	
	UPROPERTY()
	uint8 HitCounter = 0; // 변경 감지용 카운터
};

// ═══════════════════════════════════════════════════════
// 골 이벤트 구조체 (Replicated)
// ═══════════════════════════════════════════════════════
USTRUCT(BlueprintType)
struct FCoreGoalEvent
{
	GENERATED_BODY()
 
	UPROPERTY()
	int32 ScoringTeam = -1; // 0=Red, 1=Blue
 
	UPROPERTY()
	uint8 GoalCounter = 0; // 변경 감지용 카운터
};

// ═══════════════════════════════════════════════════════
// 보간용 스냅샷 구조체
// ═══════════════════════════════════════════════════════
USTRUCT()
struct FCoreNetSnapshot
{
	GENERATED_BODY()
 
	FVector Location = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	float ServerTime = 0.f;
};

// ═══════════════════════════════════════════════════════
// 델리게이트 선언
// ═══════════════════════════════════════════════════════
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoreGoalScored, int32, ScoringTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoreHit, FVector, Direction, float, Power);

UCLASS()
class OMEGA_STRIKERS_API ACoreBall : public AActor, public IOSImpactReceiver
{
	GENERATED_BODY()

public:
	ACoreBall();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick( float DeltaTime ) override;

	// ═══════════════════════════════════════════
	// 물리 파라미터 (에디터 조절용)
	// ═══════════════════════════════════════════
	UPROPERTY(EditAnywhere, Category = "Core|Physics")
	float Friction = 0.985f;
 
	UPROPERTY(EditAnywhere, Category = "Core|Physics")
	float BounceCoefficient = 0.9f;
 
	UPROPERTY(EditAnywhere, Category = "Core|Physics")
	float MaxSpeed = 3000.f;
 
	UPROPERTY(EditAnywhere, Category = "Core|Physics")
	float StopThreshold = 10.f;
	
	// ═══════════════════════════════════════════
	// 네트워크 파라미터
	// ═══════════════════════════════════════════
	UPROPERTY(EditAnywhere, Category = "Core|Network")
	float InterpSpeed = 15.f; // 클라이언트 보간 속도
 
	UPROPERTY(EditAnywhere, Category = "Core|Network")
	float MaxHitDistance = 300.f; // 히트 검증 최대 거리
 
	UPROPERTY(EditAnywhere, Category = "Core|Network")
	float HitCooldown = 0.15f; // 히트 쿨다운 (초)
 
	UPROPERTY(EditAnywhere, Category = "Core|Network")
	float SnapThreshold = 500.f; // 이 이상 차이나면 즉시 텔레포트
	
	// ═══════════════════════════════════════════
	// Replicated 변수들
	// ═══════════════════════════════════════════
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedLocation)
	FVector_NetQuantize Rep_Location;
 
	UPROPERTY(Replicated)
	FVector_NetQuantize Rep_Velocity;
 
	UPROPERTY(ReplicatedUsing = OnRep_CoreState)
	ECoreState Rep_CoreState = ECoreState::Idle;
 
	UPROPERTY(ReplicatedUsing = OnRep_HitEvent)
	FCoreHitEvent Rep_HitEvent;
 
	UPROPERTY(ReplicatedUsing = OnRep_GoalEvent)
	FCoreGoalEvent Rep_GoalEvent;
 
	UPROPERTY(Replicated)
	int32 Rep_HitCount = 0; // 연속 타격 횟수 (속도 누적용)
 
	UPROPERTY(Replicated)
	float Rep_SpeedRatio = 0.f; // 0~1 (VFX 색상 보간용)
	
	// ═══════════════════════════════════════════
	// OnRep 콜백 (클라이언트에서 호출)
	// ═══════════════════════════════════════════
	UFUNCTION()
	void OnRep_ReplicatedLocation();
 
	UFUNCTION()
	void OnRep_CoreState();
 
	UFUNCTION()
	void OnRep_HitEvent();
 
	UFUNCTION()
	void OnRep_GoalEvent();
	
	// ═══════════════════════════════════════════
	// Server RPC — 클라이언트 → 서버
	// ═══════════════════════════════════════════
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HitCore(FVector HitOrigin, FVector HitDirection, float Power);
 
	// ═══════════════════════════════════════════
	// Multicast RPC — 서버 → 모든 클라이언트
	// ═══════════════════════════════════════════
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitFX(FVector Location, FVector Direction, float Power);
 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayGoalFX(int32 ScoringTeam);
 
	// ═══════════════════════════════════════════
	// 서버 전용 — 직접 호출 API
	// ═══════════════════════════════════════════
	// 서버에서만 호출. 검증 없이 바로 힘 적용
	void ApplyHitForce(FVector Direction, float Power);
	void ResetToCenter();
	void SetHomeLocation(const FVector& NewHomeLocation);
 
	// ═══════════════════════════════════════════
	// 델리게이트
	// ═══════════════════════════════════════════
	UPROPERTY(BlueprintAssignable, Category = "Core|Events")
	FOnCoreGoalScored OnGoalScored;
 
	UPROPERTY(BlueprintAssignable, Category = "Core|Events")
	FOnCoreHit OnCoreHit;
protected:
	virtual void BeginPlay() override;

private:
	// ═══════════════════════════════════════════
	// 컴포넌트
	// ═══════════════════════════════════════════
	UPROPERTY(VisibleAnywhere, Category = "Core")
	USkeletalMeshComponent* MeshComp;
 
	UPROPERTY(VisibleAnywhere, Category = "Core")
	USkeletalMesh* MeshAsset;
	
	UPROPERTY(VisibleAnywhere, Category = "Core")
	USphereComponent* SphereComp;
 
	// ═══════════════════════════════════════════
	// 서버 전용 내부 상태
	// ═══════════════════════════════════════════
	FVector ServerVelocity = FVector::ZeroVector;
	FVector HomeLocation = FVector::ZeroVector;
	float LastHitTime = -999.f; // 히트 쿨다운 체크용
	float HitCountResetTimer = 0.f; // 연속 타격 리셋 타이머
	bool bGoalOverlapLocked = false; // 리셋 직후 재득점 방지용
 
	// 서버 물리 업데이트
	void ServerPhysicsTick(float DeltaTime);
	void ApplyFriction(float DeltaTime);
	void HandleWallBounce(const FHitResult& Hit);
	void ClampVelocity();
	void UpdateSpeedRatio();
 
	// ═══════════════════════════════════════════
	// 클라이언트 보간
	// ═══════════════════════════════════════════
	FVector ClientTargetLocation = FVector::ZeroVector;
	FVector ClientVelocity = FVector::ZeroVector;
	bool bHasReceivedFirstUpdate = false;
 
	void ClientInterpolationTick(float DeltaTime);
 
	// ═══════════════════════════════════════════
	// 골대 충돌 처리
	// ═══════════════════════════════════════════
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	
	// ═══════════════════════════════════════════
	// 임팩트 리시버 인터페이스 재정의
	// ═══════════════════════════════════════════
public:
	virtual bool ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor) override;
};
