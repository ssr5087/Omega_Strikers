// Fill out your copyright notice in the Description page of Project Settings.
// 콜리전 세팅 + 게임 로직 (골대, 배리어, 스폰) 담당

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreArena.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AGoalBarrier;

// ═══════════════════════════════════════════════════════
// CoreArena — 아레나 메시 + 콜리전 + 게임 로직
// ═══════════════════════════════════════════════════════
UCLASS(Blueprintable)
class OMEGA_STRIKERS_API ACoreArena : public AActor
{
	GENERATED_BODY()

public:
	ACoreArena();
	virtual void BeginPlay() override;
	
	// ───────────────────────────────────────────
	// 비주얼: 기존 아레나 메시
	// ───────────────────────────────────────────
	
	// 아레나 비주얼 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Visual")
	UStaticMeshComponent* ArenaMesh;
	
	// 에디터에서 아레나 메시 에셋 지정 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Visual")
	UStaticMesh* ArenaMeshAsset;
	
	// 메시 스케일 (원본 메시 크기에 맞게 조절)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Visual")
	FVector MeshScale = FVector(1.f, 1.f, 1.f);
	
	// 메시 위치 오프셋 (피봇이 중앙이 아닌 경우)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Visual")
	FVector MeshOffset = FVector::ZeroVector;
	
	// 메시 회전 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Visual")
	FRotator MeshRotation = FRotator::ZeroRotator;
	
	// ───────────────────────────────────────────
	// 콜리전 모드
	// ───────────────────────────────────────────
	
	// true: 메시 자체 콜리전 사용 (곡선 벽 그대로)
	// false: 별도 콜리전 볼륨 수동 배치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Collision")
	bool bUseMeshCollision = true;
	
	// 코어 다이 벽 메시 콤포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Collision")
	UStaticMeshComponent* WallMesh;
	
	// 코어 다이 벽 메시 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Collision")
	UStaticMesh* WallMeshAsset;
	
	// 벽 반발 계수 (높을수록 잘 튕김)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Collision", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallRestitution = 0.9f;
	
	// 벽 마찰 계수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Collision", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallFriction = 0.2f;
	
	// ───────────────────────────────────────────
	// 골대
	// ───────────────────────────────────────────
	
	// 좌측 (Blue) 골 트리거 위치 (아레나 중심 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector LeftGoalLocation = FVector(0.f, -3400.f, 0.f);
	
	// 우측 (Red) 골 트리거 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector RightGoalLocation = FVector(0.f, 3400.f, 0.f);
	
	// 골 트리거 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector GoalExtent = FVector(1200.f, 300.f, 200.f);
	
	// 골 트리거 스태틱 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Goals")
	UStaticMeshComponent* LeftGoalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Goals")
	UStaticMeshComponent* RightGoalMesh;
	
	// 골 트리거 메쉬 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	UStaticMesh* LeftGoalMeshAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	UStaticMesh* RightGoalMeshAsset;
	
	// 골대 메시 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector GoalMeshScale = FVector(1.f, 1.f, 1.f);
	
	// 골대 메시 위치 오프셋 (트리거 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector LeftGoalMeshOffset = FVector(0.f, 600.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FVector RightGoalMeshOffset = FVector(0.f, -600.f, 0.f);
 
	// 골대 메시 회전 오프셋 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	FRotator GoalMeshRotation = FRotator::ZeroRotator;
 
	// 오른쪽 골대 180도 회전 여부 (보통 true)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	//bool bMirrorRightGoal = true;
	
	// ───────────────────────────────────────────
	// 골 게이트 (배리어 전부 파괴 후 열림)
	// ───────────────────────────────────────────
	
	// 팀0 (Blue) 골 게이트 오픈 체크
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Goals", Replicated)
	bool bLeftGateOpen = false;
	
	// 팀1 (Red) 골 게이트 오픈 체크
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena|Goals", Replicated)
	bool bRightGateOpen = false;
	
	// 배리어 전부 파괴 후 게이트 열리기까지 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Goals")
	float GateOpenDelay = 1.5f;
	
	// ───────────────────────────────────────────
	// 스폰 위치
	// ───────────────────────────────────────────
	
	// 코어 스폰 위치 (아레나 중심 기준 오프셋)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Spawns")
	FVector CoreSpawnOffset = FVector(0.f, 0.f, 50.f);
	
	// 팀A (Blue) 스폰 위치들 (아레나 중심 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Spawns")
	TArray<FVector> TeamASpawnOffsets;
	
	// 팀B (Red) 스폰 위치들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Spawns")
	TArray<FVector> TeamBSpawnOffsets;
	
	// ───────────────────────────────────────────
	// 넉아웃 경계
	// ───────────────────────────────────────────
	
	// 이 영역 밖으로 나가면 KO 판정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Knockout")
	FVector KnockoutBoundsExtent = FVector(2700.f, 4500.f, 500.f);
	
	// ───────────────────────────────────────────
	// 함수
	// ───────────────────────────────────────────
	
	// 라운드 시작 : 모든 배리어 리셋 + 게이트 Close
	UFUNCTION(BlueprintCallable, Category = "Arena")
	void ResetForNewRound();
	
	// 특정 팀의 게이트가 열렸는지 체크
	UFUNCTION(BlueprintPure, Category = "Arena")
	bool IsGateOpen(int32 TeamIndex) const;
	
	UFUNCTION(BlueprintPure, Category = "Arena")
	FVector GetCoreSpawnLocation() const;
	
	UFUNCTION(BlueprintPure, Category = "Arena")
	TArray<FVector> GetTeamSpawnLocations(int32 TeamIndex) const;
	
	UFUNCTION(BlueprintPure, Category = "Arena")
	bool IsOutOfBounds(const FVector& Location) const;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* ArenaRoot;
	
	UPROPERTY()
	UBoxComponent* LeftGoalTrigger;
	
	UPROPERTY()
	UBoxComponent* RightGoalTrigger;
	
	UPROPERTY()
	class UPhysicalMaterial* ArenaPhysMat;
	
	// 레벨에 배치된 GoalBarrier 참조 (BeginPlay에서 자동으로 수집)
	UPROPERTY()
	TArray<AGoalBarrier*> FoundBarriers;
	
	void RefreshArenaMesh();
	void SetupMeshCollision();
	void SetupGoalTriggers();
	void SetupDefaultSpawns();
	void CollectBarriers();
	void CheckGateStatus(int32 TeamIndex);
	
	UFUNCTION()
	void OnBarrierDestroyed(AGoalBarrier* Barrier);
	
	UFUNCTION()
	void OnGoalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OpenGate(int32 TeamIndex);
	
	FTimerHandle LeftGateTimerHandle;
	FTimerHandle RightGateTimerHandle;
};
