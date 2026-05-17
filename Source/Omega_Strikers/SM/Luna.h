// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "Omega_Strikers/SSR/CharacterStat.h"
#include "Luna.generated.h"

class ASkillIndicatorBase;

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

protected:
	virtual void ConfigureSkillIndicator(ESkillType SkillType, ASkillIndicatorBase* Indicator) override;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class ULunaSkillCool> CoolTimeUI;
	
	UPROPERTY()
	class ULunaSkillCool* skillUI;
	
	// 사거리 컴포넌트
	// UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	// TSubclassOf<ASkillIndicatorBase> LunaPrimaryIndicatorClass;
	//
	// UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	// TSubclassOf<ASkillIndicatorBase> LunaSecondaryIndicatorClass;
	//
	// UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	// TSubclassOf<ASkillIndicatorBase> LunaSpecialIndicatorClass;
	
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
	float SecondarySkillCool = 14.f;	// 14
	float SpecialSkillCool = 35.f;		// 35
	
	// ----------------- Primary -----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 시선 처리
	virtual void Use_PrimarySkill() override;
	
	// 애니메이션 transition
	UPROPERTY(Replicated)
	bool bPrimaryAnimTrans = false;
	
	// 입력 Complete 시점의 커서 방향 저장
	FVector2D PrimaryDir;
	
	// Primary Rocket 스폰
	void SpawnPrimaryRocket();
	
	// 스킬 사용 SFX
	void Primary_Sound();
	
	// 애니메이션 종료 후 다시 이동 방향을 바라보게 설정
	void End_PrimarySkill();
	
	// ---------------- Secondary ----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 업데이트 호출
	virtual void Use_SecondarySkill() override;
	
	// 애니메이션 transition
	UPROPERTY(Replicated)
	bool bSecondaryAnimTrans = false;
	
	// 충돌 전까지 처리 중
	UPROPERTY(Replicated)
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
	
	// 스킬 사용 SFX
	void Secondary_Sound();
	
	// 위치 값 갱신 함수
	void Update_SecondaryMove();
	
	// 충돌 시 정지 상태로 변환할 함수
	UFUNCTION()
	void OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnDashHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// ----------------- Special -----------------
	
	// 입력 Complete 바인딩 (베이스에서 완료)
	// 여기서 애니메이션 transition 및 쿨타임 관련 변수 관리 + 발사 방향 관리 + 시선 처리
	virtual void Use_SpecialSkill() override;
	
	// 애니메이션 transition
	UPROPERTY(Replicated)
	bool bSpecialAnimTrans = false;
	
	// 입력 Complete 시점의 커서 위치 저장
	FVector SpecialLoc;
	
	// Primary Rocket 스폰
	void SpawnSpecialRocket();
	
	// 스킬 사용 SFX
	void Special_Sound();
	
	// 애니메이션 종료 후 다시 이동 방향을 바라보게 설정
	void End_SpecialSkill();
	
	// =========== Networking Function ===========
	
	// 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ----------------- Primary -----------------
	
	// 스킬 사용 입력(LShift 뗐을 때) 시 서버 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_StartPrimarySkill(FVector2D SpawnDir);
	
	// 서버에서 스킬 사용 확정 시 클라 시선처리용 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartPrimaryLook();
	
	// ---------------- Secondary ----------------
	
	// 스킬 사용 입력(LShift 뗐을 때) 시 서버 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_StartSecondarySkill(FVector2D StartDir);
	
	// 방향 전환 서버 RPC, 방향은 계속 보간되니 Unreliable로 해도 됨
	UFUNCTION(Server, Unreliable)
	void ServerRPC_UpdateSecondaryDirection(FVector2D AimDir);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartSecondaryCool();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_EndSecondarySkill();
	
	
	// ----------------- Special -----------------	
	
	// 스킬 사용 입력(R 뗐을 때) 시 서버 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_StartSpecialSkill(FVector SpawnLoc);
	
	// 서버에서 스킬 사용 확정 시 클라 시선처리용 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartSpecialLook();

};
