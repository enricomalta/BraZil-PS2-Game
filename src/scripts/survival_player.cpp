#include "gameplay/player.hpp"
#include "gameplay/inventory.hpp"
#include "gameplay/survival_audio.hpp"
#include "gameplay/zombie.hpp"
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
      baseSprintSpeed = -1.0F;
      setDeathScreen(ctx, false);
    }

    playerState.synchronize(ctx.playerPosition.x, ctx.playerPosition.y,
                            ctx.playerPosition.z, ctx.playerLook.x,
                            ctx.playerLook.z);
    const float moveX = ctx.playerPosition.x - previousX;
    const float moveZ = ctx.playerPosition.z - previousZ;
    const bool sprinting =
        ctx.engine && inputPressed(ctx.engine->pad, IA_SPRINT) &&
        moveX * moveX + moveZ * moveZ > 0.000001F;
    playerState.setLoad(survivalInventory().loadFactor());
    playerState.updateSurvival(g_frameDt, sprinting);
    playerState.setArmor(survivalInventory().vestEquipped(),
                         survivalInventory().helmetEquipped());
    if (ctx.hudBarValue && ctx.hudBarSet && ctx.hudBarCount >= 6) {
      ctx.hudBarValue[3] = playerState.hunger();
      ctx.hudBarValue[4] = playerState.thirst();
      ctx.hudBarValue[5] = playerState.stamina();
      ctx.hudBarSet[3] = 1;
      ctx.hudBarSet[4] = 1;
      ctx.hudBarSet[5] = 1;
    }
    if (ctx.playerSpeeds[0]) {
      if (baseSprintSpeed < 0.0F) baseSprintSpeed = ctx.playerSpeeds[0][2];
      const float staminaFactor =
          (0.55F + 0.45F * playerState.stamina() / 100.0F) *
          (1.0F - 0.2F * playerState.load());
      ctx.playerSpeeds[0][2] =
          playerState.stamina() > 0.0F
              ? baseSprintSpeed * staminaFactor
              : ctx.playerSpeeds[0][0];
    }
    if (ctx.textRequest && ctx.textCount > 10)
      ctx.textRequest[10] =
          playerState.alive() && playerState.bleeding() ? 1 : 0;
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
    if (ctx.textRequest && ctx.textCount >= 10)
      for (int i = 7; i <= 9; ++i) ctx.textRequest[i] = dead ? 0 : 1;
    if (ctx.textRequest && ctx.textCount > 10)
      ctx.textRequest[10] = dead ? 0 : (playerState.bleeding() ? 1 : 0);
    if (ctx.hudElemRequest && ctx.hudElemCount >= HUD_ELEM_BAR0 + 6) {
      ctx.hudElemRequest[0] = dead ? 1 : 0;
      for (int i = 0; i < 6; ++i)
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
      const bool sprinting =
          ctx.engine && inputPressed(ctx.engine->pad, IA_SPRINT);
      publishGameplayNoise(ctx.playerPosition.x, ctx.playerPosition.y,
                           ctx.playerPosition.z, sprinting ? 20.0F : 5.0F);
      footstepTimer = sprinting ? 0.28F : 0.42F;
    }
  }

  unsigned int sceneGeneration = 0;
  bool deathLocked = false;
  bool gameOverShown = false;
  float previousX = 0.0F;
  float previousZ = 0.0F;
  float footstepTimer = 0.0F;
  float baseSprintSpeed = -1.0F;
};

void Player::synchronize(float x, float y, float z, float lookX, float lookZ) {
  position_.x = x;
  position_.y = y;
  position_.z = z;
  facingX_ = lookX;
  facingZ_ = lookZ;
}

bool Player::damage(float amount, bool headshot) {
  if (!alive() || !(amount > 0.0F)) return false;
  if (headshot && helmet_) amount *= 0.5F;
  if (vest_) amount *= 0.7F;
  health_ = amount >= health_ ? 0.0F : health_ - amount;
  return true;
}

bool Player::heal(float amount) {
  if (!alive() || !(amount > 0.0F) || health_ >= maxHealth()) return false;
  health_ += amount;
  if (health_ > maxHealth()) health_ = maxHealth();
  return true;
}

void Player::updateSurvival(float deltaTime, bool sprinting) {
  if (deltaTime <= 0.0F || !alive()) return;

  hunger_ -= 0.01F * deltaTime;
  thirst_ -= 0.02F * deltaTime;
  if (hunger_ < 0.0F) hunger_ = 0.0F;
  if (thirst_ < 0.0F) thirst_ = 0.0F;

  if (sprinting && stamina_ > 0.0F) {
    float drain = 12.0F * (1.0F + 0.5F * load_);
    if (hunger_ < 25.0F) drain *= 1.5F;
    if (thirst_ < 25.0F) drain *= 1.75F;
    stamina_ -= drain * deltaTime;
    if (stamina_ < 0.0F) stamina_ = 0.0F;
  } else {
    float recovery = 18.0F * (1.0F - 0.3F * load_);
    if (hunger_ < 50.0F) recovery *= 0.7F;
    if (thirst_ < 50.0F) recovery *= 0.55F;
    stamina_ += recovery * deltaTime;
    if (stamina_ > 100.0F) stamina_ = 100.0F;
  }

  if (bleeding_) {
    bleedTimer_ -= deltaTime;
    if (bleedTimer_ <= 0.0F) {
      damage(1.0F);
      bleedTimer_ = 1.0F;
    }
  }
  if (hunger_ <= 0.0F && thirst_ <= 0.0F) damage(deltaTime);
}

void Player::restoreHunger(float amount) {
  if (!(amount > 0.0F)) return;
  hunger_ += amount;
  if (hunger_ > 100.0F) hunger_ = 100.0F;
}

void Player::restoreThirst(float amount) {
  if (!(amount > 0.0F)) return;
  thirst_ += amount;
  if (thirst_ > 100.0F) thirst_ = 100.0F;
}

void Player::startBleeding() {
  if (!alive() || bleeding_) return;
  bleeding_ = true;
  bleedTimer_ = 1.0F;
}

void Player::stopBleeding() {
  bleeding_ = false;
  bleedTimer_ = 0.0F;
}

void Player::setArmor(bool vest, bool helmet) {
  vest_ = vest;
  helmet_ = helmet;
}

void Player::reset() {
  position_ = PlayerPosition();
  facingX_ = 0.0F;
  facingZ_ = 1.0F;
  health_ = maxHealth();
  hunger_ = 100.0F;
  thirst_ = 100.0F;
  stamina_ = 100.0F;
  bleedTimer_ = 0.0F;
  bleeding_ = false;
  vest_ = false;
  helmet_ = false;
}

Player& survivalPlayer() {
  return playerState;
}

}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalPlayerScript);
