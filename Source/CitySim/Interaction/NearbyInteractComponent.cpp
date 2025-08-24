#include "Interaction/NearbyInteractComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AActor* UNearbyInteractComponent::FindClosestUsable() const
{
    TArray<FHitResult> Hits;
    FVector P = GetOwner()->GetActorLocation();
    UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(), P, P, Radius,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false, {}, EDrawDebugTrace::None, Hits, true);

    float BestDist = TNumericLimits<float>::Max();
    AActor* Best = nullptr;
    for (const FHitResult& H : Hits)
    {
        AActor* A = H.GetActor();
        if (!A) continue;
        if (A->ActorHasTag(FName("Usable")))
        {
            float D = FVector::DistSquared(P, A->GetActorLocation());
            if (D < BestDist) { BestDist = D; Best = A; }
        }
    }
    return Best;
}
