#include <math.h>
#include <string.h>

#include "gameplay/player.hpp"
#include "gameplay/inventory.hpp"
#include "gameplay/survival_audio.hpp"
#include "gameplay/weapon.hpp"
#include "gameplay/zombie.hpp"
#include "hud_data.gen.hpp"
#include "input_map.gen.hpp"
#include "model_data.gen.hpp"
#include "scripts/script.hpp"
#include "sound_bank.gen.hpp"

namespace Deep_forest {

Weapon& survivalWeapon() {
  static Weapon weapon;
  return weapon;
}

namespace {

constexpr float kShotDamage = 34.0F;
constexpr float kShotRange = 60.0F;
constexpr float kZombieCenterHeight = 1.0F;
constexpr float kZombieHitRadius = 0.9F;

float raySphereDistance(const Tyra::Vec4& origin, const Tyra::Vec4& direction,
                        const float center[3], float radius) {
  const float offsetX = origin.x - center[0];
  const float offsetY = origin.y - center[1];
  const float offsetZ = origin.z - center[2];
  const float projection =
      offsetX * direction.x + offsetY * direction.y + offsetZ * direction.z;
  const float constant =
      offsetX * offsetX + offsetY * offsetY + offsetZ * offsetZ -
      radius * radius;
  const float discriminant = projection * projection - constant;
  if (discriminant < 0.0F) return -1.0F;

  const float root = sqrtf(discriminant);
  float distance = -projection - root;
  if (distance < 0.0F) distance = -projection + root;
  return distance >= 0.0F ? distance : -1.0F;
}

float rayBoxDistance(const Tyra::Vec4& origin, const Tyra::Vec4& direction,
                     const RuntimeObject& object) {
  float nearDistance = 0.0F;
  float farDistance = kShotRange;
  const float originAxis[3] = {origin.x, origin.y, origin.z};
  const float directionAxis[3] = {direction.x, direction.y, direction.z};

  for (int axis = 0; axis < 3; ++axis) {
    const float halfExtent = fabsf(object.data.scale[axis]) * 0.5F;
    const float minimum = object.data.position[axis] - halfExtent;
    const float maximum = object.data.position[axis] + halfExtent;
    if (fabsf(directionAxis[axis]) < 0.0001F) {
      if (originAxis[axis] < minimum || originAxis[axis] > maximum)
        return -1.0F;
      continue;
    }

    float first = (minimum - originAxis[axis]) / directionAxis[axis];
    float second = (maximum - originAxis[axis]) / directionAxis[axis];
    if (first > second) {
      const float swap = first;
      first = second;
      second = swap;
    }
    if (first > nearDistance) nearDistance = first;
    if (second < farDistance) farDistance = second;
    if (nearDistance > farDistance) return -1.0F;
  }
  return farDistance >= 0.0F ? nearDistance : -1.0F;
}

class SurvivalCombatScript : public Script {
 public:
  void update(ScriptContext& ctx) override {
    if (sceneGeneration_ != ctx.sceneGeneration) {
      resetScene(ctx);
    }

    weapon_.update(g_frameDt);
    updateHud(ctx);
    if (!ctx.engine || !survivalPlayer().alive()) return;

    float hitDistance = -1.0F;
    const int aimedZombie = aimedZombieSlot(ctx, &hitDistance);
    if (ctx.textRequest && ctx.textCount >= 2) {
      ctx.textRequest[0] = hitDistance >= 0.0F ? 0 : 1;
      ctx.textRequest[1] = hitDistance >= 0.0F ? 1 : 0;
    }

    const bool fireHeld =
        !survivalInventoryOpen() && inputPressed(ctx.engine->pad, IA_FIRE);
    if (fireHeld) {
      fire(ctx, aimedZombie, hitDistance);
    } else {
      emptyMessageLogged_ = false;
    }
    if (inputClicked(ctx.engine->pad, IA_RELOAD)) reload(ctx);
  }

 private:
  void updateHud(ScriptContext& ctx) const {
    const bool showWeaponHud = weapon_.equipped();
    if (ctx.textRequest && ctx.textCount > 4) {
      ctx.textRequest[3] = showWeaponHud ? 1 : 0;
      ctx.textRequest[4] = showWeaponHud ? 1 : 0;
    }
    if (ctx.hudElemRequest &&
        ctx.hudElemCount >= HUD_ELEM_BAR0 + 3) {
      ctx.hudElemRequest[HUD_ELEM_BAR0 + 1] = showWeaponHud ? 1 : 0;
      ctx.hudElemRequest[HUD_ELEM_BAR0 + 2] = showWeaponHud ? 1 : 0;
    }
    if (!ctx.hudBarValue || !ctx.hudBarSet || ctx.hudBarCount < 3) return;
    ctx.hudBarValue[0] = survivalPlayer().health();
    ctx.hudBarSet[0] = 1;
    if (showWeaponHud) {
      ctx.hudBarValue[1] = static_cast<float>(weapon_.magazineAmmo());
      ctx.hudBarValue[2] =
          static_cast<float>(weapon_.reserveAmmo(survivalInventory()));
      ctx.hudBarSet[1] = 2;
      ctx.hudBarSet[2] = 2;
    }
  }

  void resetScene(ScriptContext& ctx) {
    sceneGeneration_ = ctx.sceneGeneration;
    soundbank::setBank(0, true);
    initializeSurvivalZombies(ctx);
    weapon_.reset();
    emptyMessageLogged_ = false;
  }

  void fire(ScriptContext& ctx, int zombieSlot, float hitDistance) {
    const ShotResult result = weapon_.fire();
    if (result == ShotResult::CoolingDown) return;
    if (result == ShotResult::NoWeapon) {
      if (!emptyMessageLogged_) {
        TYRA_LOG("Combat: pick up and equip a pistol before firing");
        emptyMessageLogged_ = true;
      }
      return;
    }
    if (result == ShotResult::Empty) {
      if (!emptyMessageLogged_) {
        playSurvivalSound(ctx, SurvivalSound::EmptyShot);
        TYRA_LOG("Combat: magazine empty; press L1 to reload");
        emptyMessageLogged_ = true;
      }
      return;
    }

    emptyMessageLogged_ = false;
    playSurvivalSound(ctx, SurvivalSound::Gunshot);
    padVibrate(ctx, 0.12F, false, 0.05F);
    publishGameplayNoise(ctx.playerPosition.x, ctx.playerPosition.y,
                         ctx.playerPosition.z, 35.0F);

    if (zombieSlot >= 0 && hitDistance >= 0.0F) {
      Zombie& zombie = survivalZombie(zombieSlot);
      const int objectIndex = zombie.objectIndex();
      if (objectIndex < 0 || objectIndex >= ctx.objectCount) return;
      RuntimeObject& object = ctx.objects[objectIndex];
      zombie.damage(kShotDamage);
      object.data.color[0] = 1.0F;
      object.data.color[1] = 0.85F;
      object.data.color[2] = 0.15F;
      object.dirty = true;
      padVibrate(ctx, 0.3F, true, 0.1F);
      TYRA_LOG("Combat: HIT zombie ", zombieSlot + 1, " at ", hitDistance,
               " units; HP ", zombie.health(), " ammo ",
               weapon_.magazineAmmo(), "/",
               weapon_.reserveAmmo(survivalInventory()));

      if (!zombie.alive()) {
        object.visible = false;
        object.data.collision = 2;
        object.dirty = true;
        TYRA_LOG("Combat: zombie ", zombieSlot + 1, " down");
      }
      return;
    }

    TYRA_LOG("Combat: miss; max range ", kShotRange, " units; ammo ",
             weapon_.magazineAmmo(), "/",
             weapon_.reserveAmmo(survivalInventory()));
  }

  int aimedZombieSlot(const ScriptContext& ctx, float* hitDistance) const {
    *hitDistance = -1.0F;
    Tyra::Vec4 direction = ctx.playerLook;
    const float lookLength =
        sqrtf(direction.x * direction.x + direction.y * direction.y +
              direction.z * direction.z);
    if (lookLength <= 0.0001F) return -1;
    direction.x /= lookLength;
    direction.y /= lookLength;
    direction.z /= lookLength;

    int aimedSlot = -1;
    for (int i = 0; i < survivalZombieCount(); ++i) {
      const Zombie& zombie = survivalZombie(i);
      const int objectIndex = zombie.objectIndex();
      if (!zombie.alive() || objectIndex < 0 || objectIndex >= ctx.objectCount)
        continue;

      const RuntimeObject& object = ctx.objects[objectIndex];
      if (!object.active || !object.visible) continue;
      const float radius =
          kZombieHitRadius *
          fmaxf(fabsf(object.data.scale[0]),
                fmaxf(fabsf(object.data.scale[1]),
                      fabsf(object.data.scale[2])));
      const float center[3] = {
          object.data.position[0],
          object.data.position[1] +
              kZombieCenterHeight * fabsf(object.data.scale[1]),
          object.data.position[2]};
      const float distance =
          raySphereDistance(ctx.playerPosition, direction, center, radius);
      if (distance < 0.0F || distance > kShotRange ||
          (*hitDistance >= 0.0F && distance >= *hitDistance))
        continue;

      bool blocked = false;
      for (int obstacleIndex = 0; obstacleIndex < ctx.objectCount;
           ++obstacleIndex) {
        if (obstacleIndex == objectIndex) continue;
        const RuntimeObject& obstacle = ctx.objects[obstacleIndex];
        if (!obstacle.active || !obstacle.visible ||
            obstacle.data.type != 0 || obstacle.data.collision == 2)
          continue;
        const float obstacleDistance =
            rayBoxDistance(ctx.playerPosition, direction, obstacle);
        if (obstacleDistance >= 0.0F && obstacleDistance < distance) {
          blocked = true;
          break;
        }
      }
      if (blocked) continue;

      aimedSlot = i;
      *hitDistance = distance;
    }
    return aimedSlot;
  }

  void reload(ScriptContext& ctx) {
    const ReloadResult result =
        weapon_.reload(survivalInventory(), survivalInventorySelection());
    if (result == ReloadResult::Reloaded) {
      playSurvivalSound(ctx, SurvivalSound::Reload);
      TYRA_LOG("Combat: reloaded ", weapon_.magazineAmmo(), "/",
               weapon_.magazineCapacity());
      padVibrate(ctx, 0.12F, false, 0.08F);
    } else {
      TYRA_LOG("Combat: no loaded magazine in inventory");
    }
  }

  unsigned int sceneGeneration_ = 0;
  Weapon& weapon_ = survivalWeapon();
  bool emptyMessageLogged_ = false;
};

}  // namespace
}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalCombatScript);
