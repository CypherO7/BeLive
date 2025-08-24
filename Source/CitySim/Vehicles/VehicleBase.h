#pragma once
#include "CoreMinimal.h"
#include "ChaosWheeledVehiclePawn.h"
#include "VehicleBase.generated.h"

class ACityCharacter;
class UNiagaraComponent;
class UAudioComponent;
class UCurveFloat;
class UTimelineComponent;

UCLASS()
class BELIVE_API AVehicleBase : public AChaosWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    AVehicleBase();

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float MaxThrottle = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float MaxSteer = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float EngineIdleRPM = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float MaxEngineRPM = 6000.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float AccelerationSmoothness = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float SteeringSmoothness = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float BrakeSmoothness = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float CameraDistance = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float CameraHeight = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float CameraLagSpeed = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    UCurveFloat* EngineSoundCurve;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    UCurveFloat* ExhaustVFXCurve;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float ExhaustVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float TireSmokeThreshold = 0.7f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float HornVolume = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float BrakeLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float TurnSignalBlinkRate = 1.0f;

    virtual void SetupPlayerInputComponent(UInputComponent* IC) override;
    virtual void Tick(float DeltaTime) override;

    void Throttle(float Value);
    void Steer(float Value);
    void Brake(float Value);
    void HandbrakePressed();
    void HandbrakeReleased();
    void HornPressed();
    void HornReleased();
    void ToggleLights();
    void LeftTurnSignal();
    void RightTurnSignal();

    void OnEnteredVehicle(class ACityCharacter* Driver);
    void OnExitedVehicle(class ACityCharacter* Driver);

protected:
    virtual void BeginPlay() override;

private:
    // Visual Effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* ExhaustVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* TireSmokeVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* BrakeLightVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* TurnSignalVFX;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* EngineAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* HornAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* BrakeAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* TireScreechAudio;

    // Timelines
    UPROPERTY()
    UTimelineComponent* EngineSoundTimeline;

    UPROPERTY()
    UTimelineComponent* ExhaustVFXTimeline;

    UPROPERTY()
    UTimelineComponent* TurnSignalTimeline;

    // State Variables
    float CurrentThrottle = 0.0f;
    float CurrentSteering = 0.0f;
    float CurrentBrake = 0.0f;
    float CurrentEngineRPM = 0.0f;
    bool bHornPressed = false;
    bool bLightsOn = false;
    bool bLeftTurnSignal = false;
    bool bRightTurnSignal = false;
    bool bBrakePressed = false;
    bool bHandbrakePressed = false;
    float LastSpeed = 0.0f;
    FVector LastLocation = FVector::ZeroVector;

    // Enhanced Functions
    void UpdateEngineSound(float DeltaTime);
    void UpdateExhaustVFX(float DeltaTime);
    void UpdateTireSmoke(float DeltaTime);
    void UpdateBrakeLights();
    void UpdateTurnSignals(float DeltaTime);
    void UpdateVehiclePhysics(float DeltaTime);
    void UpdateCameraEffects(float DeltaTime);
    void PlayTireScreechSound();
    void StopTireScreechSound();

    // Timeline Callbacks
    UFUNCTION()
    void OnEngineSoundUpdate(float Value);

    UFUNCTION()
    void OnExhaustVFXUpdate(float Value);

    UFUNCTION()
    void OnTurnSignalUpdate(float Value);
};
