#include "Vehicles/VehicleBase.h"
#include "ChaosVehicleMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Curves/CurveFloat.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AVehicleBase::AVehicleBase()
{
    PrimaryActorTick.bCanEverTick = true;
    Tags.Add(FName("Usable"));

    // Enhanced Spring Arm for Vehicle Camera
    USpringArmComponent* VehicleSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleSpringArm"));
    VehicleSpringArm->SetupAttachment(RootComponent);
    VehicleSpringArm->TargetArmLength = CameraDistance;
    VehicleSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, CameraHeight));
    VehicleSpringArm->bUsePawnControlRotation = true;
    VehicleSpringArm->bEnableCameraLag = true;
    VehicleSpringArm->CameraLagSpeed = CameraLagSpeed;
    VehicleSpringArm->CameraRotationLagSpeed = 3.0f;
    VehicleSpringArm->CameraLagMaxDistance = 100.0f;
    VehicleSpringArm->bDoCollisionTest = true;
    VehicleSpringArm->ProbeSize = 15.0f;

    // Vehicle Camera
    UCameraComponent* VehicleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VehicleCamera"));
    VehicleCamera->SetupAttachment(VehicleSpringArm);
    VehicleCamera->bUsePawnControlRotation = false;
    VehicleCamera->FieldOfView = 85.0f;

    // Visual Effects
    ExhaustVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExhaustVFX"));
    ExhaustVFX->SetupAttachment(RootComponent);
    ExhaustVFX->SetAutoActivate(false);

    TireSmokeVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TireSmokeVFX"));
    TireSmokeVFX->SetupAttachment(RootComponent);
    TireSmokeVFX->SetAutoActivate(false);

    BrakeLightVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BrakeLightVFX"));
    BrakeLightVFX->SetupAttachment(RootComponent);
    BrakeLightVFX->SetAutoActivate(false);

    TurnSignalVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TurnSignalVFX"));
    TurnSignalVFX->SetupAttachment(RootComponent);
    TurnSignalVFX->SetAutoActivate(false);

    // Audio Components
    EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
    EngineAudio->SetupAttachment(RootComponent);
    EngineAudio->bAutoActivate = true;

    HornAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HornAudio"));
    HornAudio->SetupAttachment(RootComponent);
    HornAudio->bAutoActivate = false;

    BrakeAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BrakeAudio"));
    BrakeAudio->SetupAttachment(RootComponent);
    BrakeAudio->bAutoActivate = false;

    TireScreechAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TireScreechAudio"));
    TireScreechAudio->SetupAttachment(RootComponent);
    TireScreechAudio->bAutoActivate = false;

    // Timelines
    EngineSoundTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("EngineSoundTimeline"));
    ExhaustVFXTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ExhaustVFXTimeline"));
    TurnSignalTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TurnSignalTimeline"));
}

void AVehicleBase::BeginPlay()
{
    Super::BeginPlay();

    // Setup Timelines
    if (EngineSoundCurve)
    {
        FOnTimelineFloat EngineSoundCallback;
        EngineSoundCallback.BindUFunction(this, FName("OnEngineSoundUpdate"));
        EngineSoundTimeline->AddInterpFloat(EngineSoundCurve, EngineSoundCallback);
        EngineSoundTimeline->SetLooping(true);
        EngineSoundTimeline->Play();
    }

    if (ExhaustVFXCurve)
    {
        FOnTimelineFloat ExhaustVFXCallback;
        ExhaustVFXCallback.BindUFunction(this, FName("OnExhaustVFXUpdate"));
        ExhaustVFXTimeline->AddInterpFloat(ExhaustVFXCurve, ExhaustVFXCallback);
        ExhaustVFXTimeline->SetLooping(true);
        ExhaustVFXTimeline->Play();
    }

    // Setup Turn Signal Timeline
    FOnTimelineFloat TurnSignalCallback;
    TurnSignalCallback.BindUFunction(this, FName("OnTurnSignalUpdate"));
    TurnSignalTimeline->AddInterpFloat(nullptr, TurnSignalCallback);
    TurnSignalTimeline->SetLooping(true);

    // Initialize engine sound
    CurrentEngineRPM = EngineIdleRPM;
    if (EngineAudio)
    {
        EngineAudio->SetFloatParameter(FName("RPM"), CurrentEngineRPM);
    }
}

void AVehicleBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEngineSound(DeltaTime);
    UpdateExhaustVFX(DeltaTime);
    UpdateTireSmoke(DeltaTime);
    UpdateBrakeLights();
    UpdateTurnSignals(DeltaTime);
    UpdateVehiclePhysics(DeltaTime);
    UpdateCameraEffects(DeltaTime);
}

void AVehicleBase::SetupPlayerInputComponent(UInputComponent* IC)
{
    Super::SetupPlayerInputComponent(IC);
    
    // Enhanced vehicle controls
    IC->BindAxis("Throttle", this, &AVehicleBase::Throttle);
    IC->BindAxis("Steer", this, &AVehicleBase::Steer);
    IC->BindAxis("Brake", this, &AVehicleBase::Brake);
    
    IC->BindAction("Handbrake", IE_Pressed, this, &AVehicleBase::HandbrakePressed);
    IC->BindAction("Handbrake", IE_Released, this, &AVehicleBase::HandbrakeReleased);
    IC->BindAction("Horn", IE_Pressed, this, &AVehicleBase::HornPressed);
    IC->BindAction("Horn", IE_Released, this, &AVehicleBase::HornReleased);
    IC->BindAction("ToggleLights", IE_Pressed, this, &AVehicleBase::ToggleLights);
    IC->BindAction("LeftTurnSignal", IE_Pressed, this, &AVehicleBase::LeftTurnSignal);
    IC->BindAction("RightTurnSignal", IE_Pressed, this, &AVehicleBase::RightTurnSignal);
}

void AVehicleBase::Throttle(float V)
{
    float TargetThrottle = FMath::Clamp(V, -MaxThrottle, MaxThrottle);
    CurrentThrottle = FMath::FInterpTo(CurrentThrottle, TargetThrottle, GetWorld()->GetDeltaSeconds(), AccelerationSmoothness);
    
    if (auto* Move = Cast<UChaosVehicleMovementComponent>(GetVehicleMovement()))
    {
        Move->SetThrottleInput(CurrentThrottle);
    }
}

void AVehicleBase::Steer(float V)
{
    float TargetSteering = FMath::Clamp(V, -MaxSteer, MaxSteer);
    CurrentSteering = FMath::FInterpTo(CurrentSteering, TargetSteering, GetWorld()->GetDeltaSeconds(), SteeringSmoothness);
    
    if (auto* Move = Cast<UChaosVehicleMovementComponent>(GetVehicleMovement()))
    {
        Move->SetSteeringInput(CurrentSteering);
    }
}

void AVehicleBase::Brake(float V)
{
    float TargetBrake = FMath::Clamp(V, 0.f, 1.f);
    CurrentBrake = FMath::FInterpTo(CurrentBrake, TargetBrake, GetWorld()->GetDeltaSeconds(), BrakeSmoothness);
    
    if (auto* Move = Cast<UChaosVehicleMovementComponent>(GetVehicleMovement()))
    {
        Move->SetBrakeInput(CurrentBrake);
    }

    // Brake effects
    if (CurrentBrake > 0.1f && !bBrakePressed)
    {
        bBrakePressed = true;
        if (BrakeAudio)
        {
            BrakeAudio->Play();
        }
    }
    else if (CurrentBrake <= 0.1f && bBrakePressed)
    {
        bBrakePressed = false;
        if (BrakeAudio)
        {
            BrakeAudio->Stop();
        }
    }
}

void AVehicleBase::HandbrakePressed()
{
    bHandbrakePressed = true;
    if (auto* Move = Cast<UChaosVehicleMovementComponent>(GetVehicleMovement()))
    {
        Move->SetHandbrakeInput(true);
    }
}

void AVehicleBase::HandbrakeReleased()
{
    bHandbrakePressed = false;
    if (auto* Move = Cast<UChaosVehicleMovementComponent>(GetVehicleMovement()))
    {
        Move->SetHandbrakeInput(false);
    }
}

void AVehicleBase::HornPressed()
{
    bHornPressed = true;
    if (HornAudio)
    {
        HornAudio->SetVolumeMultiplier(HornVolume);
        HornAudio->Play();
    }
}

void AVehicleBase::HornReleased()
{
    bHornPressed = false;
    if (HornAudio)
    {
        HornAudio->Stop();
    }
}

void AVehicleBase::ToggleLights()
{
    bLightsOn = !bLightsOn;
    // You can add light component activation/deactivation here
}

void AVehicleBase::LeftTurnSignal()
{
    bLeftTurnSignal = !bLeftTurnSignal;
    bRightTurnSignal = false;
    
    if (bLeftTurnSignal)
    {
        TurnSignalTimeline->Play();
    }
    else
    {
        TurnSignalTimeline->Stop();
        if (TurnSignalVFX)
        {
            TurnSignalVFX->Deactivate();
        }
    }
}

void AVehicleBase::RightTurnSignal()
{
    bRightTurnSignal = !bRightTurnSignal;
    bLeftTurnSignal = false;
    
    if (bRightTurnSignal)
    {
        TurnSignalTimeline->Play();
    }
    else
    {
        TurnSignalTimeline->Stop();
        if (TurnSignalVFX)
        {
            TurnSignalVFX->Deactivate();
        }
    }
}

void AVehicleBase::UpdateEngineSound(float DeltaTime)
{
    if (!EngineAudio) return;

    // Calculate engine RPM based on throttle and speed
    float Speed = GetVelocity().Size();
    float TargetRPM = EngineIdleRPM + (MaxEngineRPM - EngineIdleRPM) * FMath::Abs(CurrentThrottle);
    
    // Add speed influence
    TargetRPM += Speed * 10.0f;
    TargetRPM = FMath::Clamp(TargetRPM, EngineIdleRPM, MaxEngineRPM);
    
    CurrentEngineRPM = FMath::FInterpTo(CurrentEngineRPM, TargetRPM, DeltaTime, 2.0f);
    
    // Update engine sound parameters
    EngineAudio->SetFloatParameter(FName("RPM"), CurrentEngineRPM);
    EngineAudio->SetFloatParameter(FName("Throttle"), FMath::Abs(CurrentThrottle));
    EngineAudio->SetFloatParameter(FName("Speed"), Speed);
}

void AVehicleBase::UpdateExhaustVFX(float DeltaTime)
{
    if (!ExhaustVFX) return;

    float ExhaustIntensity = FMath::Abs(CurrentThrottle) * ExhaustVFXIntensity;
    
    if (ExhaustIntensity > 0.1f)
    {
        if (!ExhaustVFX->IsActive())
        {
            ExhaustVFX->Activate();
        }
        ExhaustVFX->SetFloatParameter(FName("Intensity"), ExhaustIntensity);
    }
    else
    {
        if (ExhaustVFX->IsActive())
        {
            ExhaustVFX->Deactivate();
        }
    }
}

void AVehicleBase::UpdateTireSmoke(float DeltaTime)
{
    if (!TireSmokeVFX) return;

    float Speed = GetVelocity().Size();
    float Acceleration = (Speed - LastSpeed) / DeltaTime;
    
    // Check for hard braking or acceleration
    bool bShouldSmoke = (FMath::Abs(CurrentBrake) > TireSmokeThreshold) || 
                       (FMath::Abs(Acceleration) > 500.0f) ||
                       (bHandbrakePressed && Speed > 100.0f);

    if (bShouldSmoke)
    {
        if (!TireSmokeVFX->IsActive())
        {
            TireSmokeVFX->Activate();
            PlayTireScreechSound();
        }
    }
    else
    {
        if (TireSmokeVFX->IsActive())
        {
            TireSmokeVFX->Deactivate();
            StopTireScreechSound();
        }
    }

    LastSpeed = Speed;
}

void AVehicleBase::UpdateBrakeLights()
{
    if (!BrakeLightVFX) return;

    if (CurrentBrake > 0.1f || bHandbrakePressed)
    {
        if (!BrakeLightVFX->IsActive())
        {
            BrakeLightVFX->Activate();
        }
        BrakeLightVFX->SetFloatParameter(FName("Intensity"), BrakeLightIntensity * CurrentBrake);
    }
    else
    {
        if (BrakeLightVFX->IsActive())
        {
            BrakeLightVFX->Deactivate();
        }
    }
}

void AVehicleBase::UpdateTurnSignals(float DeltaTime)
{
    if (!TurnSignalVFX) return;

    if (bLeftTurnSignal || bRightTurnSignal)
    {
        if (!TurnSignalVFX->IsActive())
        {
            TurnSignalVFX->Activate();
        }
        
        // Set direction parameter
        TurnSignalVFX->SetFloatParameter(FName("Direction"), bLeftTurnSignal ? -1.0f : 1.0f);
    }
}

void AVehicleBase::UpdateVehiclePhysics(float DeltaTime)
{
    // Enhanced physics updates can go here
    // Such as suspension effects, weight transfer, etc.
}

void AVehicleBase::UpdateCameraEffects(float DeltaTime)
{
    // Camera shake based on speed and terrain
    float Speed = GetVelocity().Size();
    float CameraShake = FMath::Clamp(Speed / 1000.0f, 0.0f, 0.1f);
    
    // You can apply camera shake here
}

void AVehicleBase::PlayTireScreechSound()
{
    if (TireScreechAudio && !TireScreechAudio->IsPlaying())
    {
        TireScreechAudio->Play();
    }
}

void AVehicleBase::StopTireScreechSound()
{
    if (TireScreechAudio && TireScreechAudio->IsPlaying())
    {
        TireScreechAudio->Stop();
    }
}

void AVehicleBase::OnEngineSoundUpdate(float Value)
{
    // Timeline callback for engine sound variations
    if (EngineAudio)
    {
        EngineAudio->SetFloatParameter(FName("TimelineValue"), Value);
    }
}

void AVehicleBase::OnExhaustVFXUpdate(float Value)
{
    // Timeline callback for exhaust VFX variations
    if (ExhaustVFX && ExhaustVFX->IsActive())
    {
        ExhaustVFX->SetFloatParameter(FName("TimelineValue"), Value);
    }
}

void AVehicleBase::OnTurnSignalUpdate(float Value)
{
    // Timeline callback for turn signal blinking
    if (TurnSignalVFX && TurnSignalVFX->IsActive())
    {
        TurnSignalVFX->SetFloatParameter(FName("BlinkValue"), Value);
    }
}

void AVehicleBase::OnEnteredVehicle(ACityCharacter* Driver) 
{
    // Enhanced vehicle entry effects
    if (Driver)
    {
        // You can add entry animations or effects here
    }
}

void AVehicleBase::OnExitedVehicle(ACityCharacter* Driver)  
{
    // Enhanced vehicle exit effects
    if (Driver)
    {
        // You can add exit animations or effects here
    }
}
