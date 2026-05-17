#pragma once
#include "fumbo.hpp"
#include <string>
#include <vector>

// A single line of visual novel dialogue.
// cgPath  : path to a new CG image. Empty = keep current CG.
// musicId : audio manager id for new music. Empty = keep current music.
struct VNLine {
  std::string speaker;
  std::string text;
  std::string cgPath;    // "" = no change
  std::string musicId;   // "" = no change
};

// Default prologue script (used when no custom script is supplied)
std::vector<VNLine> BuildDefaultPrologueScript();

class PrologueState : public IGameState {
public:
  // Use default prologue script
  PrologueState() = default;

  // Use a custom script (for reuse from other stages)
  explicit PrologueState(std::vector<VNLine> customScript)
      : customScript_(std::move(customScript)) {}

  ~PrologueState() override = default;
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  std::vector<VNLine> customScript_; // empty = use default

  // VisualNovel instance
  std::unique_ptr<Fumbo::UI::VisualNovel> vn;

  // Assets
  Texture2D bgTex{};
  Font testFont{};

  // Skip button (top-right corner)
  Fumbo::UI::Button skipButton;

  // Runtime state
  std::vector<VNLine> *activeScript_ = nullptr; // points to one of the two
  std::vector<VNLine> defaultScript_;
  int currentScene = 0;
  bool waitingForInput = false;
  float sceneTimer = 0.0f;
  std::string currentMusicId;  // tracks currently playing music id
  std::string currentCGPath;   // tracks currently loaded CG path

  bool isFadingOut = false;
  float fadeOutTimer = 0.0f;

  // Helper methods
  void SetupScene(int sceneNum);
  void AdvanceScene();
  void SkipAll(); // Immediately trigger fade-out
};
