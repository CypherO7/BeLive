#include "AI/NPCAIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

void ANPCAIController::OnPossess(APawn* P)
{
    Super::OnPossess(P);
    GetWorldTimerManager().SetTimer(Timer, this, &ANPCAIController::MoveToRandomPoint, RepathTime, true, 0.5f);
}

void ANPCAIController::MoveToRandomPoint()
{
    APawn* P = GetPawn(); if (!P) return;
    FNavLocation Loc;
    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        if (Nav->GetRandomPointInNavigableRadius(P->GetActorLocation(), WanderRadius, Loc))
        {
            MoveToLocation(Loc.Location, 50.f, true, true, true, false, nullptr, true);
        }
    }
}
