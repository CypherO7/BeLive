#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "CityHUD.generated.h"

UCLASS()
class BELIVE_API ACityHUD : public AHUD
{
    GENERATED_BODY()

public:
    ACityHUD();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainHUDClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> VehicleHUDClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> InteractionPromptClass;

private:
    UPROPERTY()
    UUserWidget* MainHUD;

    UPROPERTY()
    UUserWidget* VehicleHUD;

    UPROPERTY()
    UUserWidget* PauseMenu;

    UPROPERTY()
    UUserWidget* InteractionPrompt;

    bool bIsPaused = false;

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowVehicleHUD();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideVehicleHUD();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void TogglePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowInteractionPrompt(const FString& PromptText);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateSpeedometer(float Speed);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateWeatherDisplay(const FString& WeatherText);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateTimeDisplay(const FString& TimeText);
};
