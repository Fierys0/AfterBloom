#include "npc.hpp"
#include "../core/ui_style.hpp"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------
// Spawn
// ---------------------------------------------------------------
void NpcEntity::Spawn(Vector2 position, Vector2 size) {
  Destroy();

  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  physicsObj = new Fumbo::Graphic2D::Object();
  physicsObj->SetPosition(position);
  physicsObj->SetRectangle((int)size.x, (int)size.y);
  physicsObj->SetColor(color);
  physicsObj->SetBodyType(Fumbo::Graphic2D::BodyType::Static);
  physicsObj->SetCollidable(true);
  physics.AddObject(physicsObj);

  spawnPos = position;
  talking = false;
  playerNear = false;
  dialogueIndex = 0;

  npcFont =
      Fumbo::Assets::LoadFont("assets/fonts/Montserrat-ExtraBold.ttf", 24);

  vn = std::make_unique<Fumbo::UI::VisualNovel>("", 30.0f);

  Fumbo::UI::MessageBoxStyle boxStyle;
  boxStyle.backgroundColor = Color{UI::PANEL.r, UI::PANEL.g, UI::PANEL.b, 220};
  boxStyle.borderColor = UI::LINE;
  boxStyle.borderThickness = 2.0f;
  boxStyle.borderRounding = 0.0f; // flat for full width
  boxStyle.paddingTop = 20.0f;
  boxStyle.paddingBottom = 20.0f;
  boxStyle.paddingLeft = 40.0f;
  boxStyle.paddingRight = 40.0f;
  boxStyle.enableShadow = true;
  boxStyle.shadowOffset = {0.0f, -2.0f}; // shadow above
  boxStyle.shadowColor = Color{0, 0, 0, 50};
  vn->SetMessageBoxStyle(boxStyle);

  // Fill the screen width
  vn->SetMessageBoxBounds(Rectangle{0, 520, 1280, 200});
  vn->EnableMessageBox(true);

  Fumbo::UI::TextStyle textStyle;
  textStyle.alignment = Fumbo::UI::TextAlign::LEFT;
  textStyle.lineHeightMultiplier = 1.2f;
  textStyle.enableShadow = false;
  vn->SetTextStyle(textStyle);
}

// ---------------------------------------------------------------
// Destroy
// ---------------------------------------------------------------
void NpcEntity::Destroy() {
  if (physicsObj) {
    Fumbo::Graphic2D::Physics::Instance().RemoveObject(physicsObj);
    delete physicsObj;
    physicsObj = nullptr;
  }
  if (npcFont.texture.id != 0) {
    UnloadFont(npcFont);
    npcFont = {0};
  }
}

// ---------------------------------------------------------------
// StartLine  (internal)
// ---------------------------------------------------------------
void NpcEntity::StartLine(int idx) {
  dialogueIndex = idx;
  vn->SetSpeaker(
      label, Color{100, 160, 60, 255}); // Match prologue style green for NPC
  vn->SetText(dialogue[idx]);
}

// ---------------------------------------------------------------
// Update  (call every frame)
// ---------------------------------------------------------------
void NpcEntity::Update(Vector2 playerPos, bool interactPressed) {
  if (!physicsObj)
    return;

  float dx = playerPos.x - spawnPos.x;
  float dy = playerPos.y - spawnPos.y;
  playerNear = (sqrtf(dx * dx + dy * dy) <= radius);

  float dt = GetFrameTime();

  if (talking) {
    vn->Update(dt);

    // KEY_E or SPACE to advance / close
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || interactPressed) {
      if (vn->IsComplete()) {
        int next = dialogueIndex + 1;
        if (next < (int)dialogue.size()) {
          StartLine(next);
        } else {
          // Close dialogue
          talking = false;
        }
      } else {
        vn->Skip();
      }
    }
    return; // don't allow opening again while talking
  }

  // Not talking - open dialogue if player presses E nearby
  if (playerNear && (IsKeyPressed(KEY_E) || interactPressed) &&
      !dialogue.empty()) {
    talking = true;
    StartLine(0);
  }
}

// ---------------------------------------------------------------
// DrawWorld  (inside BeginMode2D)
// ---------------------------------------------------------------
void NpcEntity::DrawWorld() const {
  if (!physicsObj)
    return;

  // Prompt "[E] Talk" when player is close and not already talking
  if (playerNear && !talking) {
    Fumbo::Graphic2D::DrawText(TextFormat("[E] Talk to %s", label.c_str()),
                               {spawnPos.x - 30, spawnPos.y - 50}, {}, 16,
                               WHITE);
  }
}

// ---------------------------------------------------------------
// DrawHUD  (screen-space - called in DrawDirty)
// ---------------------------------------------------------------
void NpcEntity::DrawHUD() const {
  if (!talking || dialogue.empty())
    return;
  vn->DrawComplete(npcFont, 26, 1.0f, UI::TEXT);
}
