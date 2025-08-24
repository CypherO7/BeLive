#include "Characters/NPCCharacter.h"
#include "AI/NPCAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

ANPCCharacter::ANPCCharacter()
{
    AIControllerClass = ANPCAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    GetCharacterMovement()->MaxWalkSpeed = 280.f;
}

void ANPCCharacter::BeginPlay() { Super::BeginPlay(); }
