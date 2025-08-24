#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NearbyInteractComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BELIVE_API UNearbyInteractComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Interact")
    float Radius = 250.f;

    AActor* FindClosestUsable() const;
};
