#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

UCLASS()
class BELIVE_API ANPCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANPCCharacter();

protected:
    virtual void BeginPlay() override;
};
