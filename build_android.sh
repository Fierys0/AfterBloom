#!/bin/bash
# build_android.sh — Compile the game as an Android APK using Gradle
# Output: android/app/build/outputs/apk/debug/app-debug.apk

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$SCRIPT_DIR/android"
BUILD_DIR="$SCRIPT_DIR/build"

# --- Defaults ---
BUILD_TYPE="debug"
CLEAN=0

usage() {
  echo "Usage: $0 [--release] [--clean]"
  echo "  --release   Build a release APK (default: debug)"
  echo "  --clean     Clean before building"
  exit 0
}

for arg in "$@"; do
  case $arg in
  --release) BUILD_TYPE="release" ;;
  --clean) CLEAN=1 ;;
  -h | --help) usage ;;
  *)
    echo "Unknown option: $arg"
    usage
    ;;
  esac
done

# --- Prerequisite checks ---
if [ ! -d "$ANDROID_DIR" ]; then
  echo "ERROR: android/ directory not found at $ANDROID_DIR"
  exit 1
fi

if [ -z "$ANDROID_HOME" ] && [ -z "$ANDROID_SDK_ROOT" ]; then
  echo "WARNING: Neither ANDROID_HOME nor ANDROID_SDK_ROOT is set."
  echo "  Gradle will attempt to use the SDK configured in local.properties."
fi

# --- Ensure desktop build exists for FPK generation ---
# The Gradle preBuild task copies *.fpk from build/ into the Android assets folder.
# We must run the desktop build at least once to generate those FPK files.
if [ ! -f "$BUILD_DIR/images.fpk" ] || \
   [ ! -f "$BUILD_DIR/fonts.fpk"  ] || \
   [ ! -f "$BUILD_DIR/audio.fpk"  ]; then
  echo "==> FPK assets missing — running desktop CMake configure + pack step..."

  if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "  Configuring CMake..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
  fi

  echo "  Building asset packs..."
  cmake --build "$BUILD_DIR" --target pack_images pack_fonts pack_audio -j"$(nproc)"
  echo "  FPK assets built."
else
  echo "==> FPK assets already present, skipping desktop build."
fi

cd "$ANDROID_DIR"

if [ $CLEAN -eq 1 ]; then
  echo "==> Cleaning previous build..."
  ./gradlew clean
fi

if [ "$BUILD_TYPE" = "release" ]; then
  echo "==> Building RELEASE APK..."
  ./gradlew assembleRelease
  APK_PATH="$ANDROID_DIR/app/build/outputs/apk/release/app-release-unsigned.apk"
else
  echo "==> Building DEBUG APK..."
  ./gradlew assembleDebug
  APK_PATH="$ANDROID_DIR/app/build/outputs/apk/debug/app-debug.apk"
fi

echo ""
echo "   Build successful!"
echo "   APK: $APK_PATH"
echo "   Size: $(du -sh "$APK_PATH" | cut -f1)"
echo ""
echo "To install on a connected device/emulator:"
echo "   adb install \"$APK_PATH\""
