@echo off
REM build_android.bat — Compile the game as an Android APK using Gradle (Windows)
REM Output: android\app\build\outputs\apk\debug\app-debug.apk
REM
REM Requirements:
REM   - Android Studio / SDK installed, ANDROID_HOME set
REM   - NDK installed (set in android\local.properties or ANDROID_HOME\ndk)
REM   - CMake on PATH (for FPK asset generation)
REM   - Java / JDK on PATH (for Gradle)
REM
REM Usage: build_android.bat [--release] [--clean]

setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
REM Remove trailing backslash
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

set "ANDROID_DIR=%SCRIPT_DIR%\android"
set "BUILD_DIR=%SCRIPT_DIR%\build"
set "BUILD_TYPE=debug"
set "CLEAN=0"

REM ── Argument parsing ───────────────────────────────────────────────────────
:parse_args
if "%~1"=="" goto check_prereqs
if /i "%~1"=="--release" ( set "BUILD_TYPE=release" & shift & goto parse_args )
if /i "%~1"=="--clean"   ( set "CLEAN=1"            & shift & goto parse_args )
if /i "%~1"=="--help"    ( goto usage )
if /i "%~1"=="-h"        ( goto usage )
echo Unknown option: %~1
goto usage

:usage
echo Usage: build_android.bat [--release] [--clean]
echo   --release   Build a release APK (default: debug)
echo   --clean     Clean before building
exit /b 0

REM ── Prerequisite checks ────────────────────────────────────────────────────
:check_prereqs
if not exist "%ANDROID_DIR%" (
    echo ERROR: android\ directory not found at %ANDROID_DIR%
    exit /b 1
)

if "%ANDROID_HOME%"=="" if "%ANDROID_SDK_ROOT%"=="" (
    echo WARNING: Neither ANDROID_HOME nor ANDROID_SDK_ROOT is set.
    echo   Gradle will attempt to use the SDK from local.properties.
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: cmake not found on PATH. Install CMake and add it to PATH.
    exit /b 1
)

where java >nul 2>&1
if errorlevel 1 (
    echo ERROR: java not found on PATH. Install JDK and add it to PATH.
    exit /b 1
)

REM ── FPK asset pre-build ────────────────────────────────────────────────────
if not exist "%BUILD_DIR%\images.fpk" goto run_fpk_build
if not exist "%BUILD_DIR%\fonts.fpk"  goto run_fpk_build
if not exist "%BUILD_DIR%\audio.fpk"  goto run_fpk_build
echo =^> FPK assets already present, skipping desktop build.
goto android_build

:run_fpk_build
echo =^> FPK assets missing — running desktop CMake configure + pack step...

if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo   Configuring CMake...
    cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 ( echo ERROR: CMake configure failed. & exit /b 1 )
)

echo   Building asset packs...
REM Detect processor count for parallel build
for /f "tokens=2 delims==" %%i in (
    'wmic cpu get NumberOfLogicalProcessors /value ^| find "="'
) do set "NPROC=%%i"
if "%NPROC%"=="" set "NPROC=4"

cmake --build "%BUILD_DIR%" --target pack_images pack_fonts pack_audio -- -j%NPROC%
if errorlevel 1 ( echo ERROR: Asset packing failed. & exit /b 1 )
echo   FPK assets built.

REM ── Gradle build ───────────────────────────────────────────────────────────
:android_build
cd /d "%ANDROID_DIR%"

if "%CLEAN%"=="1" (
    echo =^> Cleaning previous build...
    call gradlew.bat clean
    if errorlevel 1 ( echo ERROR: Gradle clean failed. & exit /b 1 )
)

if /i "%BUILD_TYPE%"=="release" (
    echo =^> Building RELEASE APK...
    call gradlew.bat assembleRelease
    set "APK_PATH=%ANDROID_DIR%\app\build\outputs\apk\release\app-release-unsigned.apk"
) else (
    echo =^> Building DEBUG APK...
    call gradlew.bat assembleDebug
    set "APK_PATH=%ANDROID_DIR%\app\build\outputs\apk\debug\app-debug.apk"
)

if errorlevel 1 (
    echo.
    echo BUILD FAILED. Check the output above for errors.
    exit /b 1
)

REM ── Success ────────────────────────────────────────────────────────────────
echo.
echo    Build successful^^!
echo    APK: !APK_PATH!
echo.
echo To install on a connected device/emulator:
echo    adb install "!APK_PATH!"
echo.

endlocal
exit /b 0
