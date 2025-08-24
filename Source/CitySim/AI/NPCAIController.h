#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCAIController.generated.h"

UCLASS()
class BELIVE_API ANPCAIController : public AAIController
{
    GENERATED_BODY()

public:
    virtual void OnPossess(APawn* InPawn) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float WanderRadius = 1200.f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float RepathTime = 4.f;

    FTimerHandle Timer;

    void MoveToRandomPoint();
};
