// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "Omega_Strikers/SSR/CharacterStat.h"
#include "Luna.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ALuna : public APlayerBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALuna();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
public:
	// 데이터 테이블 (작성자 : SSR)
// public:
// 	// DataTable
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
// 	UDataTable* CharacterStatTable;
// 	
// 	// 캐릭터 이름 (Asher 고정이면 기본값 줘도 됨)
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
// 	FName CharacterName = "Luna";
//
// 	// 레벨 (나중 대비)
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
// 	int32 Level = 1;
//
// 	// 현재 스탯 저장
// 	FCharacterStat CurrentStat;
//
// 	// 실제 적용되는 값
// 	float MaxHP;
// 	float Power;
// 	float MoveSpeed;
// 	float CooldownReduction;
//
// 	// 함수
// 	FCharacterStat* GetStatByLevel(int32 InLevel);
// 	void ApplyStat(const FCharacterStat& Stat);
// 	void LevelUp();

	
	
	
public:
	// ================= Component =================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> SecondaryHitBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component|Primary")
	TSubclassOf<class ALuna_PrimaryRocket> PrimaryRocketFactory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component|Special")
	TSubclassOf<class ALuna_SpecialRocket> SpecialRocketFactory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|SkillRange")
	float PrimaryRange = 5000.f;
	
	// =================== Skill ===================
	
	virtual void PlayerMove(const struct FInputActionValue& InputActionValue) override;
	
	virtual void Ready_CoreHit() override;
	virtual void Ready_PrimarySkill() override;
	virtual void Ready_SecondarySkill() override;
	virtual void Ready_SpecialSkill() override;
	virtual void Ready_Flip() override;
	
	virtual void Use_CoreHit() override;
	
	virtual void Use_Flip() override;
	
	// 선 입력된 동작들이 아직 실행 중 인지
	bool bBufferedInput = false;
	
	// 각 스킬 쿨타임 중인지
	bool bPrimarySkillCoolDown = false;
	bool bSecondarySkillCoolDown = false;
	bool bSpecialSkillCoolDown = false;
	
	// 각 스킬 쿨타임
	float PrimarySkillCool = 6.5f;		// 6.5
	float SecondarySkillCool = 1.5f;	// 14
	float SpecialSkillCool = 1.5f;		// 35
	
	// ----------------- Primary -----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 시선 처리
	virtual void Use_PrimarySkill() override;
	
	// 애니메이션 transition
	bool bIsProcessingPrimary = false;
	
	// 입력 Complete 시점의 커서 방향 저장
	FVector2D PrimaryDir;
	
	// Primary Rocket 스폰
	void SpawnPrimaryRocket();
	
	// 애니메이션 종료 후 다시 이동 방향을 바라보게 설정
	void End_PrimarySkill();
	
	// ---------------- Secondary ----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 업데이트 호출
	virtual void Use_SecondarySkill() override;
	
	// 애니메이션 transition
	bool bIsProcessingSecondary = false;
	
	// 충돌 시 전달할 Impact Data
	FOSImpactData SecondaryImpactData;
	
	// 방향 제어 중인지 여부
	bool bIsChangingDirection = false;
	
	// 보조 스킬 사용 중 이동 방향
	FVector CurDir;
	FVector NewDir;
	
	// 보조 스킬 사용 중 위치 값 갱신 함수 호출용 타이머
	FTimerHandle MoveTimer;
	
	// 위치 값 갱신 함수
	void Update_SecondaryMove();
	
	// 충돌 시 정지 상태로 변환할 함수
	UFUNCTION()
	void OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ----------------- Special -----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 시선 처리
	virtual void Use_SpecialSkill() override;
	
	// 애니메이션 transition
	bool bIsProcessingSpecial = false;
	
	// Primary Rocket 스폰
	void SpawnSpecialRocket();
};
