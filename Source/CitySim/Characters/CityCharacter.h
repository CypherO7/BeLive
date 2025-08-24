#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "CityCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNearbyInteractComponent;
class AVehicleBase;
class UCurveFloat;
class UNiagaraComponent;
class UPostProcessComponent;

UCLASS()
class BELIVE_API ACityCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACityCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

private:
    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
    UNearbyInteractComponent* InteractComp;

    // Movement & Animation
    UPROPERTY(EditAnywhere, Category = "Movement")
    float SprintMultiplier = 1.8f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float RunSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float JumpZVelocity = 500.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AirControl = 0.3f;

    // Camera Settings
    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraSmoothness = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraTiltAmount = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    UCurveFloat* CameraTiltCurve;

    // Visual Effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* FootstepVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UPostProcessComponent* PostProcessComponent;

    // State Variables
    bool bIsSprinting = false;
    bool bIsMoving = false;
    bool bIsInAir = false;
    AVehicleBase* CurrentVehicle = nullptr;
    FVector LastMovementDirection = FVector::ZeroVector;
    float CurrentCameraTilt = 0.0f;
    float TargetCameraTilt = 0.0f;

    // Timelines
    UPROPERTY()
    UTimelineComponent* CameraTiltTimeline;

    // Input Functions
    void ApplyInputMappings();
    void MoveX(float Value);
    void MoveY(float Value);
    void LookX(float Value);
    void LookY(float Value);
    void StartSprint();
    void StopSprint();
    void DoJump();
    void Interact();
    void ExitVehicle();

    // Enhanced Movement Functions
    void UpdateMovementAnimation();
    void UpdateCameraTilt(float DeltaTime);
    void UpdateFootstepVFX();
    void PlayFootstepEffect();

    // Timeline Callbacks
    UFUNCTION()
    void OnCameraTiltUpdate(float Value);

    // Enhanced Interaction
    UFUNCTION()
    void OnInteractableFound(AActor* Interactable);
};
