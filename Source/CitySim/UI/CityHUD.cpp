#include "UI/CityHUD.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ACityHUD::ACityHUD()
{
    // Default constructor
}

void ACityHUD::BeginPlay()
{
    Super::BeginPlay();

    // Create main HUD
    if (MainHUDClass)
    {
        MainHUD = CreateWidget<UUserWidget>(GetWorld(), MainHUDClass);
        if (MainHUD)
        {
            MainHUD->AddToViewport();
        }
    }

    // Create interaction prompt (hidden by default)
    if (InteractionPromptClass)
    {
        InteractionPrompt = CreateWidget<UUserWidget>(GetWorld(), InteractionPromptClass);
        if (InteractionPrompt)
        {
            InteractionPrompt->AddToViewport();
            InteractionPrompt->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ACityHUD::ShowVehicleHUD()
{
    if (!VehicleHUD && VehicleHUDClass)
    {
        VehicleHUD = CreateWidget<UUserWidget>(GetWorld(), VehicleHUDClass);
        if (VehicleHUD)
        {
            VehicleHUD->AddToViewport();
        }
    }
    else if (VehicleHUD)
    {
        VehicleHUD->SetVisibility(ESlateVisibility::Visible);
    }
}

void ACityHUD::HideVehicleHUD()
{
    if (VehicleHUD)
    {
        VehicleHUD->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ACityHUD::TogglePauseMenu()
{
    if (!PauseMenu && PauseMenuClass)
    {
        PauseMenu = CreateWidget<UUserWidget>(GetWorld(), PauseMenuClass);
        if (PauseMenu)
        {
            PauseMenu->AddToViewport();
            PauseMenu->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (PauseMenu)
    {
        bIsPaused = !bIsPaused;
        
        if (bIsPaused)
        {
            PauseMenu->SetVisibility(ESlateVisibility::Visible);
            UGameplayStatics::SetGamePaused(GetWorld(), true);
        }
        else
        {
            PauseMenu->SetVisibility(ESlateVisibility::Hidden);
            UGameplayStatics::SetGamePaused(GetWorld(), false);
        }
    }
}

void ACityHUD::ShowInteractionPrompt(const FString& PromptText)
{
    if (InteractionPrompt)
    {
        InteractionPrompt->SetVisibility(ESlateVisibility::Visible);
        // You can set the prompt text here if you have a text widget
    }
}

void ACityHUD::HideInteractionPrompt()
{
    if (InteractionPrompt)
    {
        InteractionPrompt->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ACityHUD::UpdateSpeedometer(float Speed)
{
    // Update speedometer in vehicle HUD
    if (VehicleHUD)
    {
        // You can call Blueprint functions here to update the speed display
    }
}

void ACityHUD::UpdateWeatherDisplay(const FString& WeatherText)
{
    // Update weather display in main HUD
    if (MainHUD)
    {
        // You can call Blueprint functions here to update the weather display
    }
}

void ACityHUD::UpdateTimeDisplay(const FString& TimeText)
{
    // Update time display in main HUD
    if (MainHUD)
    {
        // You can call Blueprint functions here to update the time display
    }
}
