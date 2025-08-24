#include "World/WeatherManager.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

AWeatherManager::AWeatherManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Weather Effects
    RainVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainVFX"));
    RainVFX->SetupAttachment(RootComponent);
    RainVFX->SetAutoActivate(false);

    SnowVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SnowVFX"));
    SnowVFX->SetupAttachment(RootComponent);
    SnowVFX->SetAutoActivate(false);

    LightningVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightningVFX"));
    LightningVFX->SetupAttachment(RootComponent);
    LightningVFX->SetAutoActivate(false);

    WindVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindVFX"));
    WindVFX->SetupAttachment(RootComponent);
    WindVFX->SetAutoActivate(false);

    // Audio Components
    RainAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RainAudio"));
    RainAudio->SetupAttachment(RootComponent);
    RainAudio->bAutoActivate = false;

    ThunderAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ThunderAudio"));
    ThunderAudio->SetupAttachment(RootComponent);
    ThunderAudio->bAutoActivate = false;

    WindAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    WindAudio->SetupAttachment(RootComponent);
    WindAudio->bAutoActivate = false;
}

void AWeatherManager::BeginPlay()
{
    Super::BeginPlay();

    // Find world components
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        Sun = *It; 
        break;
    }

    for (TActorIterator<ASkyAtmosphere> It(GetWorld()); It; ++It)
    {
        SkyAtmosphere = *It;
        break;
    }

    for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
    {
        HeightFog = *It;
        break;
    }

    // Setup initial weather
    TargetWeather = Weather;
    ApplyWeather(Weather);
    SetupWeatherEffects();
}

void AWeatherManager::Tick(float DT)
{
    Super::Tick(DT);

    if (DayLengthSeconds <= 1.f) return;

    // Update time
    TimeAccum = FMath::Fmod(TimeAccum + (DT * TimeAcceleration), DayLengthSeconds);
    const float T = TimeAccum / DayLengthSeconds; // 0..1

    UpdateSun(T);
    UpdateTimeOfDay(T);
    UpdateAtmosphere();
    UpdateWeatherEffects(DT);
    UpdateLighting(DT);
    UpdateAudio(DT);
    UpdateWindEffects(DT);
    UpdateLightningEffects(DT);
    TransitionWeather(DT);

    // Dynamic weather changes
    if (bEnableDynamicWeather)
    {
        WeatherChangeTimer += DT;
        if (WeatherChangeTimer >= WeatherChangeInterval)
        {
            ChangeWeatherRandomly();
            WeatherChangeTimer = 0.0f;
        }
    }
}

void AWeatherManager::UpdateSun(float T)
{
    if (!Sun) return;

    // Enhanced sun movement with smooth transitions
    const float Pitch = FMath::Lerp(-20.f, 200.f, T); // below horizon → overhead → set
    const float Yaw = FMath::Lerp(0.f, 360.f, T); // full rotation
    FRotator R(Pitch, Yaw, 0.f);
    Sun->SetActorRotation(R);

    // Dynamic sun intensity based on time of day
    float Intensity = 0.0f;
    if (T >= 0.25f && T <= 0.75f) // Day time
    {
        Intensity = FMath::Sin((T - 0.25f) * PI * 2.0f) * SunIntensity;
        Intensity = FMath::Clamp(Intensity, 0.1f, SunIntensity);
    }
    else if (T > 0.75f || T < 0.25f) // Night time
    {
        Intensity = 0.05f; // Minimal night lighting
    }

    CurrentSunIntensity = FMath::FInterpTo(CurrentSunIntensity, Intensity, GetWorld()->GetDeltaSeconds(), 1.0f);

    if (auto* LC = Sun->GetLightComponent())
    {
        LC->SetIntensity(40000.f * CurrentSunIntensity);
        
        // Dynamic sun color
        FLinearColor TargetColor = SunColor;
        if (T >= 0.4f && T <= 0.6f) // Midday
        {
            TargetColor = FLinearColor(1.0f, 1.0f, 1.0f); // Pure white
        }
        else if (T >= 0.6f && T <= 0.8f) // Evening
        {
            TargetColor = FLinearColor(1.0f, 0.7f, 0.5f); // Warm orange
        }
        else if (T >= 0.8f || T <= 0.2f) // Night
        {
            TargetColor = FLinearColor(0.3f, 0.4f, 0.8f); // Blue tint
        }

        CurrentSunColor = FMath::Lerp(CurrentSunColor, TargetColor, GetWorld()->GetDeltaSeconds() * 0.5f);
        LC->SetLightColor(CurrentSunColor);
    }
}

void AWeatherManager::UpdateAtmosphere()
{
    if (SkyAtmosphere)
    {
        // Update sky atmosphere based on weather and time
        float SkyIntensity = FMath::Lerp(0.3f, 1.0f, CurrentSunIntensity);
        SkyAtmosphere->SetMieScatteringScale(SkyIntensity);
        SkyAtmosphere->SetRayleighScatteringScale(SkyIntensity);
    }

    if (HeightFog)
    {
        // Update fog based on weather
        float TargetFogDensity = 0.0f;
        switch (Weather)
        {
            case EWeatherType::Foggy:
                TargetFogDensity = FogDensity;
                break;
            case EWeatherType::HeavyRain:
            case EWeatherType::Stormy:
                TargetFogDensity = FogDensity * 0.7f;
                break;
            case EWeatherType::LightRain:
                TargetFogDensity = FogDensity * 0.3f;
                break;
            default:
                TargetFogDensity = 0.0f;
                break;
        }

        CurrentFogDensity = FMath::FInterpTo(CurrentFogDensity, TargetFogDensity, GetWorld()->GetDeltaSeconds(), 0.5f);
        HeightFog->SetFogDensity(CurrentFogDensity);
    }
}

void AWeatherManager::UpdateWeatherEffects(float DeltaTime)
{
    // Update rain effects
    if (bEnableRainEffects)
    {
        float RainIntensity = 0.0f;
        switch (Weather)
        {
            case EWeatherType::LightRain:
                RainIntensity = 0.3f;
                break;
            case EWeatherType::HeavyRain:
            case EWeatherType::Stormy:
                RainIntensity = 1.0f;
                break;
        }
        UpdateRainIntensity(RainIntensity);
    }

    // Update snow effects
    if (bEnableSnowEffects)
    {
        float SnowIntensity = (Weather == EWeatherType::Snowy) ? 1.0f : 0.0f;
        UpdateSnowIntensity(SnowIntensity);
    }
}

void AWeatherManager::UpdateLighting(float DeltaTime)
{
    // Update sky color based on time and weather
    FLinearColor TargetSkyColor = SkyColor;
    
    float T = GetNormalizedTime();
    if (T >= 0.4f && T <= 0.6f) // Midday
    {
        TargetSkyColor = FLinearColor(0.5f, 0.7f, 1.0f); // Bright blue
    }
    else if (T >= 0.6f && T <= 0.8f) // Evening
    {
        TargetSkyColor = FLinearColor(1.0f, 0.6f, 0.4f); // Orange sky
    }
    else if (T >= 0.8f || T <= 0.2f) // Night
    {
        TargetSkyColor = FLinearColor(0.1f, 0.1f, 0.3f); // Dark blue
    }

    // Weather influence on sky color
    switch (Weather)
    {
        case EWeatherType::Stormy:
        case EWeatherType::HeavyRain:
            TargetSkyColor *= 0.5f; // Darker sky
            break;
        case EWeatherType::Foggy:
            TargetSkyColor *= 0.7f; // Muted sky
            break;
    }

    CurrentSkyColor = FMath::Lerp(CurrentSkyColor, TargetSkyColor, DeltaTime * 0.5f);
    UpdateSkyAtmosphere();
}

void AWeatherManager::UpdateAudio(float DeltaTime)
{
    // Update rain audio
    if (RainAudio)
    {
        float RainVolume = 0.0f;
        switch (Weather)
        {
            case EWeatherType::LightRain:
                RainVolume = 0.3f;
                break;
            case EWeatherType::HeavyRain:
            case EWeatherType::Stormy:
                RainVolume = 0.8f;
                break;
        }

        if (RainVolume > 0.0f)
        {
            if (!RainAudio->IsPlaying())
            {
                RainAudio->Play();
            }
            RainAudio->SetVolumeMultiplier(RainVolume);
        }
        else
        {
            if (RainAudio->IsPlaying())
            {
                RainAudio->Stop();
            }
        }
    }

    // Update wind audio
    if (WindAudio && bEnableWindEffects)
    {
        float WindVolume = CurrentWindIntensity * 0.5f;
        if (WindVolume > 0.1f)
        {
            if (!WindAudio->IsPlaying())
            {
                WindAudio->Play();
            }
            WindAudio->SetVolumeMultiplier(WindVolume);
        }
        else
        {
            if (WindAudio->IsPlaying())
            {
                WindAudio->Stop();
            }
        }
    }
}

void AWeatherManager::UpdateWindEffects(float DeltaTime)
{
    if (!bEnableWindEffects) return;

    float TargetWindIntensity = 0.0f;
    switch (Weather)
    {
        case EWeatherType::Stormy:
            TargetWindIntensity = WindIntensity * 2.0f;
            break;
        case EWeatherType::HeavyRain:
            TargetWindIntensity = WindIntensity * 1.5f;
            break;
        case EWeatherType::LightRain:
            TargetWindIntensity = WindIntensity * 0.5f;
            break;
    }

    CurrentWindIntensity = FMath::FInterpTo(CurrentWindIntensity, TargetWindIntensity, DeltaTime, 0.5f);
    UpdateWindIntensity(CurrentWindIntensity);
}

void AWeatherManager::UpdateLightningEffects(float DeltaTime)
{
    if (!bEnableLightningEffects || Weather != EWeatherType::Stormy) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLightningTime > LightningInterval)
    {
        TriggerLightning();
        LastLightningTime = CurrentTime;
        LightningInterval = FMath::RandRange(5.0f, 15.0f); // Random interval
    }
}

void AWeatherManager::TransitionWeather(float DeltaTime)
{
    if (Weather == TargetWeather) return;

    WeatherTransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(WeatherTransitionTimer / WeatherTransitionDuration, 0.0f, 1.0f);

    if (Alpha >= 1.0f)
    {
        Weather = TargetWeather;
        WeatherTransitionTimer = 0.0f;
        ApplyWeather(Weather);
    }
}

void AWeatherManager::ChangeWeatherRandomly()
{
    TArray<EWeatherType> WeatherTypes = { EWeatherType::Clear, EWeatherType::Cloudy, EWeatherType::LightRain, EWeatherType::Foggy };
    EWeatherType NewWeather = WeatherTypes[FMath::RandRange(0, WeatherTypes.Num() - 1)];
    
    // Don't change to the same weather
    if (NewWeather != Weather)
    {
        SetWeather(NewWeather);
    }
}

void AWeatherManager::SetWeather(EWeatherType NewWeather)
{
    TargetWeather = NewWeather;
    WeatherTransitionTimer = 0.0f;
}

void AWeatherManager::SetTimeOfDay(float NormalizedTime)
{
    TimeAccum = NormalizedTime * DayLengthSeconds;
}

ETimeOfDay AWeatherManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

void AWeatherManager::UpdateTimeOfDay(float NormalizedTime)
{
    ETimeOfDay NewTimeOfDay;
    
    if (NormalizedTime < 0.1f) NewTimeOfDay = ETimeOfDay::Dawn;
    else if (NormalizedTime < 0.25f) NewTimeOfDay = ETimeOfDay::Morning;
    else if (NormalizedTime < 0.4f) NewTimeOfDay = ETimeOfDay::Noon;
    else if (NormalizedTime < 0.6f) NewTimeOfDay = ETimeOfDay::Afternoon;
    else if (NormalizedTime < 0.75f) NewTimeOfDay = ETimeOfDay::Dusk;
    else if (NormalizedTime < 0.9f) NewTimeOfDay = ETimeOfDay::Night;
    else NewTimeOfDay = ETimeOfDay::Midnight;

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        // You can add time-of-day specific events here
    }
}

void AWeatherManager::SetupWeatherEffects()
{
    // Setup VFX systems
    if (RainVFX)
    {
        RainVFX->SetFloatParameter(FName("Intensity"), 0.0f);
    }

    if (SnowVFX)
    {
        SnowVFX->SetFloatParameter(FName("Intensity"), 0.0f);
    }

    if (WindVFX)
    {
        WindVFX->SetFloatParameter(FName("Intensity"), 0.0f);
    }
}

void AWeatherManager::UpdateRainIntensity(float Intensity)
{
    if (RainVFX)
    {
        if (Intensity > 0.0f)
        {
            if (!RainVFX->IsActive())
            {
                RainVFX->Activate();
            }
            RainVFX->SetFloatParameter(FName("Intensity"), Intensity);
        }
        else
        {
            if (RainVFX->IsActive())
            {
                RainVFX->Deactivate();
            }
        }
    }
}

void AWeatherManager::UpdateSnowIntensity(float Intensity)
{
    if (SnowVFX)
    {
        if (Intensity > 0.0f)
        {
            if (!SnowVFX->IsActive())
            {
                SnowVFX->Activate();
            }
            SnowVFX->SetFloatParameter(FName("Intensity"), Intensity);
        }
        else
        {
            if (SnowVFX->IsActive())
            {
                SnowVFX->Deactivate();
            }
        }
    }
}

void AWeatherManager::UpdateFogDensity(float Density)
{
    if (HeightFog)
    {
        HeightFog->SetFogDensity(Density);
    }
}

void AWeatherManager::UpdateWindIntensity(float Intensity)
{
    if (WindVFX)
    {
        if (Intensity > 0.1f)
        {
            if (!WindVFX->IsActive())
            {
                WindVFX->Activate();
            }
            WindVFX->SetFloatParameter(FName("Intensity"), Intensity);
            WindVFX->SetVectorParameter(FName("Direction"), WindDirection);
        }
        else
        {
            if (WindVFX->IsActive())
            {
                WindVFX->Deactivate();
            }
        }
    }
}

void AWeatherManager::TriggerLightning()
{
    if (LightningVFX)
    {
        LightningVFX->Activate(true);
        
        // Play thunder sound
        if (ThunderAudio)
        {
            ThunderAudio->Play();
        }

        // Deactivate lightning after a short time
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (LightningVFX)
            {
                LightningVFX->Deactivate();
            }
        }, 0.5f, false);
    }
}

void AWeatherManager::UpdateSkyAtmosphere()
{
    if (SkyAtmosphere)
    {
        // Update sky atmosphere with current colors
        SkyAtmosphere->SetMieScatteringColor(CurrentSkyColor);
        SkyAtmosphere->SetRayleighScatteringColor(CurrentSkyColor);
    }
}

void AWeatherManager::UpdatePostProcessSettings()
{
    // You can add post-process volume updates here
    // Such as bloom, contrast, saturation based on weather and time
}

void AWeatherManager::ApplyWeather(EWeatherType Type)
{
    switch (Type)
    {
        case EWeatherType::Clear:
            // Clear sky baseline
            break;
        case EWeatherType::Cloudy:
            // Add cloud cover
            break;
        case EWeatherType::LightRain:
            // Light rain system
            break;
        case EWeatherType::HeavyRain:
            // Heavy rain system
            break;
        case EWeatherType::Stormy:
            // Storm system with lightning
            break;
        case EWeatherType::Foggy:
            // Fog system
            break;
        case EWeatherType::Snowy:
            // Snow system
            break;
    }
}
