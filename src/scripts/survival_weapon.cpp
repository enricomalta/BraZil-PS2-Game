#include <math.h>
#include <string.h>

#include "gameplay/inventory.hpp"
#include "gameplay/weapon.hpp"
#include "model_data.gen.hpp"
#include "scene_data.hpp"
#include "scripts/script.hpp"

namespace Deep_forest {
namespace {

constexpr float kPi = 3.14159265358979323846F;

int findWeaponObject(const ScriptContext& ctx) {
  if (ctx.scene < 0 || ctx.scene >= SCENE_COUNT) return -1;
  int weaponModel = -1;
  for (int i = 0; i < MODEL_COUNT; ++i) {
    if (strcmp(MODEL_SOURCES[i], "res/models/weapon.obj") == 0) {
      weaponModel = i;
      break;
    }
  }
  if (weaponModel < 0) return -1;

  const int sceneObjectCount =
      ctx.objectCount < SCENE_OBJECT_COUNTS[ctx.scene]
          ? ctx.objectCount
          : SCENE_OBJECT_COUNTS[ctx.scene];
  for (int i = 0; i < sceneObjectCount; ++i) {
    const RuntimeObject& object = ctx.objects[i];
    if (object.active && object.data.type == 5 &&
        object.data.model == weaponModel)
      return i;
  }
  return -1;
}

class SurvivalWeaponScript : public Script {
 public:
  void update(ScriptContext& ctx) override {
    if (sceneGeneration_ != ctx.sceneGeneration) {
      sceneGeneration_ = ctx.sceneGeneration;
      objectIndex_ = findWeaponObject(ctx);
      lastShotSequence_ = survivalWeapon().shotSequence();
      lastReloadSequence_ = survivalWeapon().reloadSequence();
      recoil_ = 0.0F;
      reloadTimer_ = 0.0F;
      if (objectIndex_ < 0) {
        TYRA_WARN("Weapon: res/models/weapon.obj is missing from scene");
        return;
      }
      ctx.objects[objectIndex_].wantsMatrixPath = true;
    }

    if (!ctx.engine || objectIndex_ < 0 || objectIndex_ >= ctx.objectCount)
      return;
    RuntimeObject& object = ctx.objects[objectIndex_];
    if (!object.active) return;

    const Weapon& weapon = survivalWeapon();
    if (!weapon.owned()) {
      // A dropped pistol stays where the inventory script put it.
      if (weapon.dropped()) return;
      object.visible = true;
      const SceneObjectData& pickup = SCENE_OBJECTS[objectIndex_];
      for (int axis = 0; axis < 3; ++axis) {
        object.data.position[axis] = pickup.position[axis];
        object.data.rotation[axis] = pickup.rotation[axis];
      }
      if (!object.onMatrixPath) object.dirty = true;
      return;
    }
    if (!weapon.equipped()) {
      object.visible = false;
      return;
    }
    object.visible = true;
    if (weapon.shotSequence() != lastShotSequence_) {
      lastShotSequence_ = weapon.shotSequence();
      recoil_ = 0.12F;
    }
    if (weapon.reloadSequence() != lastReloadSequence_) {
      lastReloadSequence_ = weapon.reloadSequence();
      reloadTimer_ = 0.45F;
    }
    if (recoil_ > 0.0F)
      recoil_ = g_frameDt >= recoil_ ? 0.0F : recoil_ - g_frameDt;
    if (reloadTimer_ > 0.0F)
      reloadTimer_ =
          g_frameDt >= reloadTimer_ ? 0.0F : reloadTimer_ - g_frameDt;

    Tyra::Vec4 forward = ctx.playerLook;
    const float forwardLength =
        sqrtf(forward.x * forward.x + forward.y * forward.y +
              forward.z * forward.z);
    if (forwardLength < 0.0001F) return;
    forward.x /= forwardLength;
    forward.y /= forwardLength;
    forward.z /= forwardLength;

    const float horizontalLength =
        sqrtf(forward.x * forward.x + forward.z * forward.z);
    Tyra::Vec4 right(1.0F, 0.0F, 0.0F);
    if (horizontalLength > 0.0001F) {
      right.x = forward.z / horizontalLength;
      right.z = -forward.x / horizontalLength;
    }
    const Tyra::Vec4 up(forward.y * right.z - forward.z * right.y,
                        forward.z * right.x - forward.x * right.z,
                        forward.x * right.y - forward.y * right.x);
    const float recoilOffset = recoil_;
    object.data.position[0] =
        ctx.playerPosition.x + right.x * 0.32F + up.x * -0.28F +
        forward.x * (0.65F - recoilOffset);
    object.data.position[1] =
        ctx.playerPosition.y + right.y * 0.32F + up.y * -0.28F +
        forward.y * (0.65F - recoilOffset);
    object.data.position[2] =
        ctx.playerPosition.z + right.z * 0.32F + up.z * -0.28F +
        forward.z * (0.65F - recoilOffset);
    object.data.rotation[0] = -asinf(forward.y) * 180.0F / kPi;
    object.data.rotation[1] = atan2f(forward.x, forward.z) * 180.0F / kPi;
    object.data.rotation[2] =
        reloadTimer_ > 0.0F ? 20.0F * reloadTimer_ / 0.45F : 0.0F;
    if (!object.onMatrixPath) object.dirty = true;
  }

 private:
  unsigned int sceneGeneration_ = 0;
  unsigned int lastShotSequence_ = 0;
  unsigned int lastReloadSequence_ = 0;
  int objectIndex_ = -1;
  float recoil_ = 0.0F;
  float reloadTimer_ = 0.0F;
};

}  // namespace
}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalWeaponScript);
