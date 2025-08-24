#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherManager.generated.h"

UENUM(BlueprintType)
enum class EWeatherType : uint8 
{ 
    Clear, 
    LightRain, 
    HeavyRain, 
    Foggy,
    Stormy,
    Snowy,
    Cloudy
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night,
    Midnight
};

UCLASS()
class BELIVE_API AWeatherManager : public AActor
{
    GENERATED_BODY()

public:
    AWeatherManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayLengthSeconds = 600.f; // 10 min per day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeAcceleration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType Weather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableDynamicWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableRainEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableSnowEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableWindEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableLightningEffects = true;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    ETimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetNormalizedTime() const { return TimeAccum / DayLengthSeconds; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetTimeAcceleration(float NewAcceleration) { TimeAcceleration = NewAcceleration; }

protected:
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    class ADirectionalLight* Sun = nullptr;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere = nullptr;

    UPROPERTY()
    class AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY()
    class UNiagaraComponent* RainVFX = nullptr;

    UPROPERTY()
    class UNiagaraComponent* SnowVFX = nullptr;

    UPROPERTY()
    class UNiagaraComponent* LightningVFX = nullptr;

    UPROPERTY()
    class UNiagaraComponent* WindVFX = nullptr;

    UPROPERTY()
    class UAudioComponent* RainAudio = nullptr;

    UPROPERTY()
    class UAudioComponent* ThunderAudio = nullptr;

    UPROPERTY()
    class UAudioComponent* WindAudio = nullptr;

    // State Variables
    float TimeAccum = 0.f;
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Noon;
    EWeatherType TargetWeather = EWeatherType::Clear;
    float WeatherTransitionTimer = 0.0f;
    float WeatherChangeTimer = 0.0f;
    float LastLightningTime = 0.0f;
    float LightningInterval = 10.0f;

    // Weather Parameters
    float CurrentFogDensity = 0.0f;
    float CurrentWindIntensity = 0.0f;
    float CurrentSunIntensity = 1.0f;
    FLinearColor CurrentSunColor = FLinearColor::White;
    FLinearColor CurrentSkyColor = FLinearColor::Blue;

    // Enhanced Functions
    void UpdateSun(float NormalizedTime);
    void UpdateAtmosphere();
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateLighting(float DeltaTime);
    void UpdateAudio(float DeltaTime);
    void UpdateWindEffects(float DeltaTime);
    void UpdateLightningEffects(float DeltaTime);
    void TransitionWeather(float DeltaTime);
    void ChangeWeatherRandomly();
    void ApplyWeather(EWeatherType Type);
    void UpdateTimeOfDay(float NormalizedTime);
    void SetupWeatherEffects();
    void UpdateRainIntensity(float Intensity);
    void UpdateSnowIntensity(float Intensity);
    void UpdateFogDensity(float Density);
    void UpdateWindIntensity(float Intensity);
    void TriggerLightning();
    void UpdateSkyAtmosphere();
    void UpdatePostProcessSettings();
};
