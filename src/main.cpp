#include "fumbo.hpp"
#include "game/areas/prologue.hpp"
#include "game/core/globals.hpp"
#include "game/core/settings.hpp"
#include "game/states/shared_mechanics.hpp"
#include "game/states/title_state.hpp"

int main(int argc, char **argv) {
  Fumbo::Instance().Init(1280, 720, "After Bloom", 0);
  Fumbo::Assets::AddAssetPack("images.fpk");
  Fumbo::Assets::AddAssetPack("fonts.fpk");
  Fumbo::Assets::AddAssetPack("audio.fpk");

  // Load Game Globals
  LoadGlobalAssets();

  // Instantiate persistent mechanics state
  auto sharedMechanics = std::make_shared<SharedMechanics>();
  Fumbo::Instance().SharedState(sharedMechanics);
  Fumbo::Instance().PauseSharedState(true); // Shared mechanics paused in menus

  // Start Game with Title State
  Fumbo::Instance().Run(std::make_shared<TitleState>());

  // Cleanup Game Globals
  UnloadGlobalAssets();

  Fumbo::Instance().Quit();

  return 0;
}
