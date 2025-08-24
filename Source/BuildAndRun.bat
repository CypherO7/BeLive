@echo off
echo ========================================
echo BeLive - Beautiful City Simulation
echo ========================================
echo.

echo Building BeLive project...
echo.

REM Check if Unreal Engine is installed
if not exist "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe" (
    echo ERROR: Unreal Engine 5.3 not found!
    echo Please install Unreal Engine 5.3 from Epic Games Launcher
    pause
    exit /b 1
)

REM Build the project
echo Building project...
"C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" BeLive Win64 Development -Project="%~dp0BeLive.uproject" -WaitMutex -FromMsBuild

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.

REM Ask user if they want to run the game
set /p choice="Do you want to run BeLive now? (y/n): "
if /i "%choice%"=="y" (
    echo.
    echo Starting BeLive...
    "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0BeLive.uproject"
)

echo.
echo Done!
pause
