# 🚀 BeLive Application Setup Guide

This guide will help you build and run your beautiful BeLive city simulation application.

## 📋 Prerequisites

### Required Software:
1. **Unreal Engine 5.3** - Download from Epic Games Launcher
2. **Visual Studio 2022** (Community Edition is free) - For C++ compilation
3. **Windows 10/11** - 64-bit operating system

### System Requirements:
- **CPU**: Intel Core i5-8400 or AMD Ryzen 5 2600
- **RAM**: 16 GB RAM
- **GPU**: NVIDIA GTX 1060 or AMD RX 580 (6GB VRAM)
- **Storage**: 50 GB available space
- **DirectX**: Version 12

## 🛠️ Installation Steps

### Step 1: Install Unreal Engine 5.3
1. Open Epic Games Launcher
2. Go to "Unreal Engine" tab
3. Click "Install Engine"
4. Select version 5.3
5. Choose installation location
6. Wait for download and installation (may take 1-2 hours)

### Step 2: Install Visual Studio 2022
1. Download Visual Studio 2022 Community from Microsoft
2. During installation, select:
   - **Desktop development with C++**
   - **Game development with C++**
   - **Windows 10/11 SDK**
3. Complete installation

### Step 3: Project Setup
1. **Clone or download** your BeLive project files
2. **Verify file structure** matches the expected layout
3. **Double-click** `BeLive.uproject` to open in Unreal Engine

## 🔧 Building the Application

### Method 1: Using the Batch File (Recommended)
1. **Double-click** `BuildAndRun.bat`
2. The script will:
   - Check for Unreal Engine installation
   - Build the project automatically
   - Offer to run the game when complete

### Method 2: Manual Build
1. **Open Command Prompt** in project directory
2. **Run build command**:
   ```batch
   "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" BeLive Win64 Development -Project="BeLive.uproject"
   ```

### Method 3: Using Unreal Editor
1. **Open** `BeLive.uproject` in Unreal Editor
2. **Wait** for project to compile
3. **Click** "Play" button to run

## 🎮 Running the Application

### From Unreal Editor:
1. **Open** the project in Unreal Editor
2. **Create** or **open** a level
3. **Add** your game objects:
   - Place `CityCharacter` as player
   - Add `VehicleBase` instances
   - Place `WeatherManager` in the world
4. **Click** "Play" button

### Standalone Game:
1. **Build** the project (see above)
2. **Navigate** to `Binaries/Win64/`
3. **Run** `BeLive.exe`

## 🎯 Project Structure

```
BeLive/
├── BeLive.uproject              # Main project file
├── Source/
│   └── BeLive/
│       ├── BeLive.Build.cs      # Build configuration
│       ├── BeLive.cpp           # Module implementation
│       ├── BeLive.h             # Module header
│       ├── BeLive.Target.cs     # Game target
│       ├── BeLiveEditor.Target.cs # Editor target
│       ├── CityGameMode.h/cpp   # Main game mode
│       ├── Characters/          # Character classes
│       ├── Vehicles/            # Vehicle system
│       ├── Interaction/         # Interaction system
│       ├── AI/                  # AI controllers
│       ├── World/               # Weather system
│       └── UI/                  # HUD system
├── Config/                      # Configuration files
├── Content/                     # Game assets
├── BuildAndRun.bat             # Build script
└── README.md                   # Project documentation
```

## ⚙️ Configuration Files

### Input Configuration (`Config/DefaultInput.ini`)
- **Character Controls**: WASD movement, mouse look, space jump
- **Vehicle Controls**: WASD for throttle/steering, space handbrake
- **Interaction**: E for interact, F for exit vehicle
- **Vehicle Features**: H for horn, L for lights, Q/E for turn signals

### Game Configuration (`Config/DefaultGame.ini`)
- **Project Settings**: Name, version, company info
- **Renderer Settings**: Anti-aliasing, post-processing
- **Physics Settings**: Gravity, collision profiles

### Engine Configuration (`Config/DefaultEngine.ini`)
- **Game Mode**: Default game mode and maps
- **Performance**: Optimized settings for smooth gameplay

## 🐛 Troubleshooting

### Common Issues:

#### 1. **Build Fails**
- **Solution**: Ensure Visual Studio 2022 is installed with C++ components
- **Check**: Unreal Engine 5.3 is properly installed

#### 2. **Missing Dependencies**
- **Solution**: Verify all required plugins are enabled:
  - ChaosVehicles
  - EnhancedInput
  - Niagara
  - NavigationSystem

#### 3. **Compilation Errors**
- **Solution**: Clean and rebuild:
  ```batch
  "C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" BeLive Win64 Development -Project="BeLive.uproject" -Clean
  ```

#### 4. **Performance Issues**
- **Solution**: Adjust settings in `Config/DefaultEngine.ini`
- **Check**: Graphics drivers are up to date

#### 5. **Input Not Working**
- **Solution**: Verify input mappings in `Config/DefaultInput.ini`
- **Check**: No conflicting input devices

## 🎨 Customization

### Adding New Features:
1. **Create** new C++ classes in appropriate directories
2. **Update** `BeLive.Build.cs` with new dependencies
3. **Rebuild** the project
4. **Test** in Unreal Editor

### Modifying Settings:
1. **Edit** configuration files in `Config/` directory
2. **Restart** Unreal Editor
3. **Test** changes

## 📦 Distribution

### Creating Standalone Build:
1. **In Unreal Editor**: File → Package Project → Windows
2. **Choose** target directory
3. **Wait** for packaging to complete
4. **Distribute** the packaged folder

### Requirements for Distribution:
- **Target System**: Windows 10/11 64-bit
- **Dependencies**: Visual C++ Redistributable 2019
- **Storage**: 2-5 GB depending on assets

## 🆘 Support

### Getting Help:
1. **Check** this setup guide
2. **Review** README.md for project details
3. **Verify** all prerequisites are installed
4. **Check** Unreal Engine documentation

### Useful Commands:
```batch
# Clean build
"C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" BeLive Win64 Development -Project="BeLive.uproject" -Clean

# Generate project files
"C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="BeLive.uproject" -game -rocket -progress
```

---

**🎉 Congratulations! Your BeLive application is ready to run!**

Enjoy your beautiful city simulation! 🌆✨
