#pragma once
#include "raylib.h"

// -------------------------------------------------------
// AfterBloom UI Style — Light Theme, Sunflower/Orange
// Prefixed AB_ to avoid collision with raylib color macros
// -------------------------------------------------------
namespace UI {

static const Color BG        = {252, 244, 218, 255}; // warm cream
static const Color PANEL     = {255, 250, 232, 255}; // lighter cream panel
static const Color AB_YELLOW = {255, 185,   0, 255}; // sunflower (slightly deepened for light bg)
static const Color AB_ORANGE = {215,  75,   0, 255}; // burnt orange (darker for contrast)
static const Color TEXT      = { 38,  26,  10, 255}; // dark warm brown
static const Color TEXT_DIM  = {148, 112,  55, 255}; // muted warm
static const Color BTN_IDLE  = {255, 185,   0, 255}; // yellow button bg
static const Color BTN_HOVER = {215,  75,   0, 255}; // orange hover
static const Color BTN_TEXT  = { 38,  26,  10, 255}; // dark text on buttons
static const Color LINE      = {210, 168,  65, 180}; // golden divider

static constexpr float ACCENT_H = 3.0f;
static constexpr float BTN_W    = 260.0f;
static constexpr float BTN_H    = 50.0f;

} // namespace UI
