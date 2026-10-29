#include "gameplay/player.hpp"
#include "gameplay/survival_audio.hpp"
#include "hud_data.gen.hpp"
#include "input_map.gen.hpp"
#include "scripts/script.hpp"

namespace Deep_forest {
namespace {

Player playerState;

}  // namespace

class SurvivalPlayerScript : public Script {
 public:
  void update(ScriptContext& ctx) override {
    if (sceneGeneration != ctx.sceneGeneration) {
      playerState.reset();
      deathLocked = false;
      gameOverShown = false;
      sceneGeneration = ctx.sceneGeneration;
      previousX = ctx.playerPosition.x;
      previousZ = ctx.playerPosition.z;
      footstepTimer = 0.0F;
      setDeathScreen(ctx, false);
    }

    playerState.synchronize(ctx.playerPosition.x, ctx.playerPosition.y,
                            ctx.playerPosition.z, ctx.playerLook.x,
                            ctx.playerLook.z);
    updateFootsteps(ctx);
    if (!playerState.alive() && !deathLocked) {
      ctx.lockInput = 0;
      deathLocked = true;
    }

    if (!playerState.alive()) {
      if (!gameOverShown) {
        setDeathScreen(ctx, true);
        gameOverShown = true;
      }
      if (ctx.engine && inputClicked(ctx.engine->pad, IA_ROLE_CONFIRM))
        ctx.requestScene = ctx.scene;
    }
  }

 private:
  // Texts: 0 reticle, 1 aim-on-zombie, 2-4 HUD labels, 5 title, 6 prompt.
  // Elements: 0 is the black image, bars start at HUD_ELEM_BAR0.
  void setDeathScreen(ScriptContext& ctx, bool dead) {
    if (ctx.textRequest && ctx.textCount >= 7) {
      ctx.textRequest[0] = dead ? 0 : 1;
      ctx.textRequest[1] = 0;
      for (int i = 2; i <= 4; ++i) ctx.textRequest[i] = dead ? 0 : 1;
      ctx.textRequest[5] = dead ? 1 : 0;
      ctx.textRequest[6] = dead ? 1 : 0;
    }
    if (ctx.hudElemRequest && ctx.hudElemCount >= HUD_ELEM_BAR0 + 3) {
      ctx.hudElemRequest[0] = dead ? 1 : 0;
      for (int i = 0; i < 3; ++i)
        ctx.hudElemRequest[HUD_ELEM_BAR0 + i] = dead ? 0 : 1;
    }
  }

  void updateFootsteps(ScriptContext& ctx) {
    const float dx = ctx.playerPosition.x - previousX;
    const float dz = ctx.playerPosition.z - previousZ;
    previousX = ctx.playerPosition.x;
    previousZ = ctx.playerPosition.z;
    if (!playerState.alive() || dx * dx + dz * dz < 0.00000025F) {
      footstepTimer = 0.0F;
      return;
    }

    footstepTimer -= g_frameDt;
    if (footstepTimer <= 0.0F) {
      playSurvivalSound(ctx, SurvivalSound::Footstep);
      footstepTimer = 0.42F;
    }
  }

  unsigned int sceneGeneration = 0;
  bool deathLocked = false;
  bool gameOverShown = false;
  float previousX = 0.0F;
  float previousZ = 0.0F;
  float footstepTimer = 0.0F;
};

void Player::synchronize(float x, float y, float z, float lookX, float lookZ) {
  position_.x = x;
  position_.y = y;
  position_.z = z;
  facingX_ = lookX;
  facingZ_ = lookZ;
}

bool Player::damage(float amount) {
  if (!alive() || !(amount > 0.0F)) return false;
  health_ = amount >= health_ ? 0.0F : health_ - amount;
  return true;
}

void Player::reset() {
  position_ = PlayerPosition();
  facingX_ = 0.0F;
  facingZ_ = 1.0F;
  health_ = maxHealth();
}

Player& survivalPlayer() {
  return playerState;
}

}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalPlayerScript);
