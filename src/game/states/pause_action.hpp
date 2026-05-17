#pragma once

// Pause menu result actions — kept in a separate header to avoid
// circular includes between pause_menu.hpp and shared_mechanics.hpp.
enum class PauseAction { NONE, RESUME, RETRY, SETTINGS, QUIT };
