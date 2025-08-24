#include "Characters/CityCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Interaction/NearbyInteractComponent.h"
#include "Vehicles/VehicleBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/TimelineComponent.h"
#include "NiagaraComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Curves/CurveFloat.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACityCharacter::ACityCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Enhanced Spring Arm Setup
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 350.f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 3.0f;
    SpringArm->CameraRotationLagSpeed = 5.0f;
    SpringArm->CameraLagMaxDistance = 50.0f;
    SpringArm->bDoCollisionTest = true;
    SpringArm->ProbeSize = 12.0f;

    // Enhanced Camera Setup
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
    Camera->FieldOfView = 90.0f;

    // Interaction Component
    InteractComp = CreateDefaultSubobject<UNearbyInteractComponent>(TEXT("InteractComp"));

    // Visual Effects
    FootstepVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FootstepVFX"));
    FootstepVFX->SetupAttachment(RootComponent);
    FootstepVFX->SetAutoActivate(false);

    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(RootComponent);

    // Timeline for Camera Tilt
    CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTiltTimeline"));

    // Enhanced Movement Setup
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
    GetCharacterMovement()->AirControl = AirControl;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpAirControl = 0.3f;
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->MaxAcceleration = 2048.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
}

void ACityCharacter::BeginPlay()
{
    Super::BeginPlay();
    ApplyInputMappings();

    // Setup Camera Tilt Timeline
    if (CameraTiltCurve)
    {
        FOnTimelineFloat TimelineCallback;
        TimelineCallback.BindUFunction(this, FName("OnCameraTiltUpdate"));
        CameraTiltTimeline->AddInterpFloat(CameraTiltCurve, TimelineCallback);
        CameraTiltTimeline->SetLooping(true);
    }

    // Setup interaction callbacks
    if (InteractComp)
    {
        // You can add delegates here for interaction feedback
    }
}

void ACityCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMovementAnimation();
    UpdateCameraTilt(DeltaTime);
    UpdateFootstepVFX();
}

void ACityCharacter::ApplyInputMappings()
{
    // Enhanced input mappings will be set up in SetupPlayerInputComponent
}

void ACityCharacter::MoveX(float V)
{
    if (!Controller || FMath::IsNearlyZero(V)) 
    {
        bIsMoving = false;
        return;
    }
    
    bIsMoving = true;
    const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
    const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
    AddMovementInput(Right, V);
    LastMovementDirection = Right * V;
}

void ACityCharacter::MoveY(float V)
{
    if (!Controller || FMath::IsNearlyZero(V)) 
    {
        bIsMoving = false;
        return;
    }
    
    bIsMoving = true;
    const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
    const FVector Fwd = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    AddMovementInput(Fwd, V);
    LastMovementDirection = Fwd * V;
}

void ACityCharacter::LookX(float V) 
{ 
    AddControllerYawInput(V * 1.5f); // Enhanced sensitivity
}

void ACityCharacter::LookY(float V) 
{ 
    AddControllerPitchInput(V * 1.5f); // Enhanced sensitivity
}

void ACityCharacter::StartSprint()
{
    if (!bIsSprinting && !bIsInAir)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed * SprintMultiplier;
        
        // Camera effects for sprinting
        if (SpringArm)
        {
            SpringArm->TargetArmLength = 400.f; // Pull back camera slightly
        }
        
        // Start camera tilt timeline
        if (CameraTiltTimeline && !CameraTiltTimeline->IsPlaying())
        {
            CameraTiltTimeline->Play();
        }
    }
}

void ACityCharacter::StopSprint()
{
    if (bIsSprinting)
    {
        bIsSprinting = false;
        GetCharacterMovement()->MaxWalkSpeed = bIsMoving ? RunSpeed : WalkSpeed;
        
        // Reset camera
        if (SpringArm)
        {
            SpringArm->TargetArmLength = 350.f;
        }
        
        // Stop camera tilt
        if (CameraTiltTimeline && CameraTiltTimeline->IsPlaying())
        {
            CameraTiltTimeline->Stop();
        }
    }
}

void ACityCharacter::DoJump() 
{ 
    if (!bIsInAir)
    {
        Jump();
        bIsInAir = true;
        
        // Camera effects for jumping
        if (SpringArm)
        {
            SpringArm->TargetArmLength = 450.f;
        }
    }
}

void ACityCharacter::Interact()
{
    if (CurrentVehicle) return;
    
    if (AActor* Target = InteractComp->FindClosestUsable())
    {
        if (AVehicleBase* Veh = Cast<AVehicleBase>(Target))
        {
            CurrentVehicle = Veh;
            
            // Smooth transition effects
            GetMesh()->SetVisibility(false, true);
            DetachFromControllerPendingDestroy();
            
            AController* PC = UGameplayStatics::GetPlayerController(this, 0);
            if (PC) 
            { 
                PC->Possess(Veh);
                
                // Camera transition effect
                if (SpringArm)
                {
                    SpringArm->TargetArmLength = 600.f; // Vehicle camera distance
                }
            }
            
            Veh->OnEnteredVehicle(this);
        }
    }
}

void ACityCharacter::ExitVehicle()
{
    if (!CurrentVehicle) return;
    
    AController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->Possess(this);
        GetMesh()->SetVisibility(true, true);
        
        // Smooth exit positioning
        const FVector ExitLoc = CurrentVehicle->GetActorLocation()
                              + CurrentVehicle->GetActorRightVector() * 120.f;
        SetActorLocation(ExitLoc);
        
        // Reset camera
        if (SpringArm)
        {
            SpringArm->TargetArmLength = 350.f;
        }
    }
    
    CurrentVehicle->OnExitedVehicle(this);
    CurrentVehicle = nullptr;
}

void ACityCharacter::UpdateMovementAnimation()
{
    // Update movement state
    bIsInAir = GetCharacterMovement()->IsFalling();
    
    // Reset camera when landing
    if (!bIsInAir && SpringArm && SpringArm->TargetArmLength > 400.f)
    {
        SpringArm->TargetArmLength = bIsSprinting ? 400.f : 350.f;
    }
}

void ACityCharacter::UpdateCameraTilt(float DeltaTime)
{
    if (!SpringArm) return;
    
    // Smooth camera tilt based on movement
    float TargetTilt = 0.0f;
    if (bIsMoving && !bIsInAir)
    {
        TargetTilt = CameraTiltAmount * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f);
    }
    
    CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, CameraSmoothness);
    
    // Apply tilt to spring arm
    FRotator CurrentRotation = SpringArm->GetRelativeRotation();
    CurrentRotation.Roll = CurrentCameraTilt;
    SpringArm->SetRelativeRotation(CurrentRotation);
}

void ACityCharacter::UpdateFootstepVFX()
{
    if (bIsMoving && !bIsInAir && !bIsSprinting)
    {
        // Play footstep effects at intervals
        static float LastFootstepTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastFootstepTime > 0.5f) // Every 0.5 seconds
        {
            PlayFootstepEffect();
            LastFootstepTime = CurrentTime;
        }
    }
}

void ACityCharacter::PlayFootstepEffect()
{
    if (FootstepVFX)
    {
        FootstepVFX->Activate(true);
        
        // Deactivate after a short time
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (FootstepVFX)
            {
                FootstepVFX->Deactivate();
            }
        }, 0.2f, false);
    }
}

void ACityCharacter::OnCameraTiltUpdate(float Value)
{
    // Timeline callback for camera tilt animation
    if (SpringArm)
    {
        FRotator CurrentRotation = SpringArm->GetRelativeRotation();
        CurrentRotation.Roll = Value * CameraTiltAmount;
        SpringArm->SetRelativeRotation(CurrentRotation);
    }
}

void ACityCharacter::OnInteractableFound(AActor* Interactable)
{
    // Visual feedback when interactable is found
    if (Interactable)
    {
        // You can add highlight effects here
    }
}

void ACityCharacter::SetupPlayerInputComponent(UInputComponent* IC)
{
    check(IC);
    
    // Enhanced input bindings with better responsiveness
    IC->BindAxis("MoveX", this, &ACityCharacter::MoveX);
    IC->BindAxis("MoveY", this, &ACityCharacter::MoveY);
    IC->BindAxis("LookX", this, &ACityCharacter::LookX);
    IC->BindAxis("LookY", this, &ACityCharacter::LookY);
    
    IC->BindAction("Jump", IE_Pressed, this, &ACityCharacter::DoJump);
    IC->BindAction("Sprint", IE_Pressed, this, &ACityCharacter::StartSprint);
    IC->BindAction("Sprint", IE_Released, this, &ACityCharacter::StopSprint);
    IC->BindAction("Interact", IE_Pressed, this, &ACityCharacter::Interact);
    IC->BindAction("ExitVehicle", IE_Pressed, this, &ACityCharacter::ExitVehicle);
}
