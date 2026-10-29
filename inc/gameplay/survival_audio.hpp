#pragma once

namespace Deep_forest {

struct ScriptContext;

enum class SurvivalSound {
  Gunshot,
  Reload,
  EmptyShot,
  Footstep,
  ZombieAttack
};

void playSurvivalSound(ScriptContext& ctx, SurvivalSound sound);

}  // namespace Deep_forest
