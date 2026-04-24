#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OSType.h"
#include "OSImpactReceiver.generated.h"

class AActor;

UINTERFACE(Blueprintable)
class OMEGA_STRIKERS_API UOSImpactReceiver : public UInterface
{
	GENERATED_BODY()
};

class OMEGA_STRIKERS_API IOSImpactReceiver
{
	GENERATED_BODY()

public:
	// 캐릭터 & 코어 모두 인터페이스 상속받음
	// Hit Data와 Impact 원인 제공 캐릭터에 대한 정보를 받음
	// 추후 Instigator Actor는 필요 없다고 느껴질 경우 삭제해도 무방
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Impact")
	bool ReceiveImpact(const FOSImpactData& ImpactData, AActor* InstigatorActor);
};