// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
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
	
	// ================= Component =================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> SecondaryHitBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component|Primary")
	TSubclassOf<class ALuna_PrimaryRocket> PrimaryRocketFactory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component|Special")
	TSubclassOf<class ALuna_SpecialRocket> SpecialRocketFactory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|SkillRange")
	float PrimaryRange = 5000.f;
	
	
	// 선 입력된 동작들이 아직 실행 중 인지
	bool bBufferedInput = false;
	
	bool bPrimarySkillCoolDown = false;
	bool bSecondarySkillCoolDown = false;
	bool bSpecialSkillCoolDown = false;
	
	float PrimarySkillCool = 0.5f;		//6.5
	float SecondarySkillCool = 0.5f;	//14
	float SpecialSkillCool = 0.5f;		//35
	
	virtual void PlayerMove(const struct FInputActionValue& InputActionValue) override;
	
	virtual void Ready_CoreHit() override;
	virtual void Ready_PrimarySkill() override;
	virtual void Ready_SecondarySkill() override;
	virtual void Ready_SpecialSkill() override;
	virtual void Ready_Flip() override;
	
	virtual void Use_CoreHit() override;
	virtual void Use_PrimarySkill() override;
	virtual void Use_SecondarySkill() override;
	virtual void Use_SpecialSkill() override;
	virtual void Use_Flip() override;
	
	// 보조 스킬이 사용 중 인지 여부
	bool bIsProcessingSecondary = false;
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

	
};
