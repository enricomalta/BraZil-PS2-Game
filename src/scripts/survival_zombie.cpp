#include <math.h>
#include <string.h>

#include "gameplay/zombie.hpp"
#include "gameplay/player.hpp"
#include "gameplay/survival_audio.hpp"
#include "model_data.gen.hpp"
#include "scripts/script.hpp"

namespace Deep_forest {
namespace {

constexpr float kPerceptionRange = 8.0F;
constexpr float kAttackRange = 2.2F;
constexpr float kAttackCooldown = 1.2F;
constexpr float kAttackDamage = 12.0F;
constexpr float kInvestigateSpeed = 0.85F;
constexpr float kChaseSpeed = 1.15F;
constexpr float kPatrolSpeed = 0.45F;
constexpr float kZombieRadius = 0.45F;
constexpr float kPi = 3.14159265358979323846F;

Zombie zombieStates[kMaxSurvivalZombies];
int zombieCount = 0;
unsigned int zombieSceneGeneration = 0;
bool zombiesInitialized = false;
GameplayNoise latestNoise;

int findZombieModel() {
  for (int i = 0; i < MODEL_COUNT; ++i) {
    if (strcmp(MODEL_SOURCES[i], "res/models/zombie.obj") == 0) {
      return i;
    }
  }
  return -1;
}

float distanceSquared(float ax, float az, float bx, float bz) {
  const float dx = bx - ax;
  const float dz = bz - az;
  return dx * dx + dz * dz;
}

bool lineIntersectsBox(const float start[3], const float end[3],
                       const RuntimeObject& obstacle) {
  float nearDistance = 0.0F;
  float farDistance = 1.0F;
  for (int axis = 0; axis < 3; ++axis) {
    const float halfExtent = fabsf(obstacle.data.scale[axis]) * 0.5F;
    const float minimum = obstacle.data.position[axis] - halfExtent;
    const float maximum = obstacle.data.position[axis] + halfExtent;
    const float delta = end[axis] - start[axis];
    if (fabsf(delta) < 0.0001F) {
      if (start[axis] < minimum || start[axis] > maximum) return false;
      continue;
    }

    float first = (minimum - start[axis]) / delta;
    float second = (maximum - start[axis]) / delta;
    if (first > second) {
      const float swap = first;
      first = second;
      second = swap;
    }
    if (first > nearDistance) nearDistance = first;
    if (second < farDistance) farDistance = second;
    if (nearDistance > farDistance) return false;
  }
  return farDistance >= 0.0F && nearDistance <= 1.0F;
}

bool canSeePlayer(const RuntimeObject& zombie, const PlayerPosition& player,
                  int zombieObjectIndex, const RuntimeObject* objects,
                  int objectCount) {
  const float start[3] = {zombie.data.position[0],
                           zombie.data.position[1] + 1.0F,
                           zombie.data.position[2]};
  const float end[3] = {player.x, player.y, player.z};
  for (int i = 0; i < objectCount; ++i) {
    const RuntimeObject& obstacle = objects[i];
    if (i == zombieObjectIndex || !obstacle.active || !obstacle.visible ||
        obstacle.data.type != 0 || obstacle.data.collision == 2)
      continue;
    if (lineIntersectsBox(start, end, obstacle)) return false;
  }
  return true;
}

}  // namespace

void Zombie::reset(int objectIndex, float startX, float startZ) {
  objectIndex_ = objectIndex;
  health_ = 100.0F;
  attackCooldown_ = 0.0F;
  investigateX_ = 0.0F;
  investigateZ_ = 0.0F;
  homeX_ = startX;
  homeZ_ = startZ;
  patrolX_ = startX;
  patrolZ_ = startZ;
  patrolTimer_ = 0.6F + static_cast<float>(objectIndex % 4) * 0.35F;
  lastNoiseSequence_ = latestNoise.sequence;
  patrolIndex_ = 0;
  patrolMoving_ = false;
  state_ = ZombieState::Idle;
}

void initializeSurvivalZombies(ScriptContext& ctx) {
  if (zombiesInitialized && zombieSceneGeneration == ctx.sceneGeneration)
    return;

  zombiesInitialized = true;
  zombieSceneGeneration = ctx.sceneGeneration;
  zombieCount = 0;
  resetGameplayNoise();

  for (int i = 0; i < kMaxSurvivalZombies; ++i) {
    zombieStates[i].reset(-1, 0.0F, 0.0F);
  }

  const int zombieModel = findZombieModel();
  if (zombieModel < 0) {
    TYRA_WARN("Zombie: model res/models/zombie.obj is missing from the scene");
    return;
  }

  int matchingObjects = 0;
  for (int i = 0; i < ctx.objectCount; ++i) {
    const RuntimeObject& object = ctx.objects[i];
    if (!object.active || object.data.type != 5 ||
        object.data.model != zombieModel)
      continue;

    ++matchingObjects;
    if (zombieCount >= kMaxSurvivalZombies) continue;

    zombieStates[zombieCount].reset(i, object.data.position[0],
                                    object.data.position[2]);
    ctx.objects[i].wantsMatrixPath = true;
    ++zombieCount;
  }

  if (matchingObjects > kMaxSurvivalZombies) {
    TYRA_WARN("Zombie: scene has ", matchingObjects,
              " zombies; runtime limit is ", kMaxSurvivalZombies);
  } else if (zombieCount == 0) {
    TYRA_WARN("Zombie: no active zombie objects found");
  } else {
    TYRA_LOG("Zombie: initialized ", zombieCount, " enemies");
  }
}

bool Zombie::damage(float amount) {
  if (!alive() || !(amount > 0.0F)) return false;
  health_ = amount >= health_ ? 0.0F : health_ - amount;
  if (health_ <= 0.0F) state_ = ZombieState::Dead;
  return true;
}

bool Zombie::moveToward(RuntimeObject& object, float x, float z, float speed,
                        const RuntimeObject* objects, int objectCount,
                        float deltaTime) {
  const float dx = x - object.data.position[0];
  const float dz = z - object.data.position[2];
  const float distance = sqrtf(dx * dx + dz * dz);
  if (distance <= 0.001F) return true;

  const float step = speed * deltaTime;
  float moveX;
  float moveZ;
  if (distance <= step) {
    moveX = dx;
    moveZ = dz;
  } else {
    moveX = dx / distance * step;
    moveZ = dz / distance * step;
  }

  bool moved = false;
  const float candidateX = object.data.position[0] + moveX;
  if (!isBlocked(candidateX, object.data.position[2], objects, objectCount)) {
    object.data.position[0] = candidateX;
    moved = fabsf(moveX) > 0.0001F;
  }
  const float candidateZ = object.data.position[2] + moveZ;
  if (!isBlocked(object.data.position[0], candidateZ, objects, objectCount)) {
    object.data.position[2] = candidateZ;
    moved = moved || fabsf(moveZ) > 0.0001F;
  }

  const float actualX = object.data.position[0] - (candidateX - moveX);
  const float actualZ = object.data.position[2] - (candidateZ - moveZ);
  if (moved) {
    object.data.rotation[1] = atan2f(actualX, actualZ) * 180.0F / kPi;
    if (!object.onMatrixPath) object.dirty = true;
  }
  const float remainingX = x - object.data.position[0];
  const float remainingZ = z - object.data.position[2];
  return remainingX * remainingX + remainingZ * remainingZ <= 0.01F;
}

bool Zombie::isBlocked(float x, float z, const RuntimeObject* objects,
                       int objectCount) const {
  for (int i = 0; i < objectCount; ++i) {
    const RuntimeObject& obstacle = objects[i];
    if (i == objectIndex_ || !obstacle.active || !obstacle.visible ||
        obstacle.data.type != 0 || obstacle.data.collision == 2)
      continue;
    const float halfX = fabsf(obstacle.data.scale[0]) * 0.5F + kZombieRadius;
    const float halfZ = fabsf(obstacle.data.scale[2]) * 0.5F + kZombieRadius;
    if (fabsf(x - obstacle.data.position[0]) < halfX &&
        fabsf(z - obstacle.data.position[2]) < halfZ)
      return true;
  }
  return false;
}

void Zombie::updatePatrol(RuntimeObject& object, const RuntimeObject* objects,
                          int objectCount, float deltaTime) {
  static const float kPatrolOffsets[4][2] = {
      {-2.0F, 0.0F}, {0.0F, 2.0F}, {2.0F, 0.0F}, {0.0F, -2.0F}};
  if (!patrolMoving_) {
    if (patrolTimer_ > 0.0F) {
      patrolTimer_ =
          deltaTime >= patrolTimer_ ? 0.0F : patrolTimer_ - deltaTime;
      return;
    }
    patrolX_ = homeX_ + kPatrolOffsets[patrolIndex_][0];
    patrolZ_ = homeZ_ + kPatrolOffsets[patrolIndex_][1];
    patrolMoving_ = true;
  }

  if (moveToward(object, patrolX_, patrolZ_, kPatrolSpeed, objects,
                 objectCount, deltaTime)) {
    patrolMoving_ = false;
    patrolIndex_ = static_cast<unsigned char>((patrolIndex_ + 1) % 4);
    patrolTimer_ = 1.0F + static_cast<float>(objectIndex_ % 3) * 0.4F;
  }
}

bool Zombie::update(RuntimeObject& object, const PlayerPosition& player,
                    const GameplayNoise& noise, const RuntimeObject* objects,
                    int objectCount, float deltaTime) {
  if (!alive() || deltaTime <= 0.0F) return false;
  if (attackCooldown_ > 0.0F) {
    attackCooldown_ = deltaTime >= attackCooldown_
                          ? 0.0F
                          : attackCooldown_ - deltaTime;
  }

  const float x = object.data.position[0];
  const float z = object.data.position[2];
  const float playerDistanceSquared = distanceSquared(x, z, player.x, player.z);
  const float perceptionSquared = kPerceptionRange * kPerceptionRange;
  const float attackSquared = kAttackRange * kAttackRange;
  const bool seesPlayer =
      playerDistanceSquared <= perceptionSquared &&
      canSeePlayer(object, player, objectIndex_, objects, objectCount);

  if (noise.sequence != lastNoiseSequence_) {
    lastNoiseSequence_ = noise.sequence;
    if (noise.radius > 0.0F &&
        distanceSquared(x, z, noise.x, noise.z) <= noise.radius * noise.radius) {
      investigateX_ = noise.x;
      investigateZ_ = noise.z;
      if (playerDistanceSquared > perceptionSquared)
        if (state_ != ZombieState::Chase && state_ != ZombieState::Attack)
          state_ = ZombieState::Investigate;
    }
  }

  if (seesPlayer) {
    if (state_ != ZombieState::Attack) state_ = ZombieState::Chase;
    patrolMoving_ = false;
  } else if (state_ == ZombieState::Chase || state_ == ZombieState::Attack) {
    state_ = ZombieState::Idle;
    patrolMoving_ = false;
    attackCooldown_ = 0.0F;
  }

  switch (state_) {
    case ZombieState::Idle:
      updatePatrol(object, objects, objectCount, deltaTime);
      break;
    case ZombieState::Investigate:
      if (moveToward(object, investigateX_, investigateZ_, kInvestigateSpeed,
                     objects, objectCount, deltaTime))
        state_ = ZombieState::Idle;
      break;
    case ZombieState::Chase:
      if (playerDistanceSquared <= attackSquared) {
        state_ = ZombieState::Attack;
        attackCooldown_ = 0.0F;
      } else {
        moveToward(object, player.x, player.z, kChaseSpeed, objects,
                   objectCount, deltaTime);
      }
      break;
    case ZombieState::Attack:
      if (playerDistanceSquared > attackSquared * 1.4F) {
        state_ = ZombieState::Chase;
        break;
      }
      if (attackCooldown_ <= 0.0F) {
        attackCooldown_ = kAttackCooldown;
        return true;
      }
      break;
    case ZombieState::Dead:
      break;
  }
  return false;
}

Zombie& survivalZombie(int index) {
  return zombieStates[index];
}

int survivalZombieCount() {
  return zombieCount;
}

void publishGameplayNoise(float x, float y, float z, float radius) {
  latestNoise.x = x;
  latestNoise.y = y;
  latestNoise.z = z;
  latestNoise.radius = radius;
  ++latestNoise.sequence;
}

GameplayNoise gameplayNoise() {
  return latestNoise;
}

void resetGameplayNoise() {
  latestNoise = GameplayNoise();
}

class SurvivalZombieScript : public Script {
 public:
  void update(ScriptContext& ctx) override {
    initializeSurvivalZombies(ctx);
    if (!ctx.engine) return;

    for (int i = 0; i < survivalZombieCount(); ++i) {
      Zombie& zombie = survivalZombie(i);
      const int objectIndex = zombie.objectIndex();
      if (objectIndex < 0 || objectIndex >= ctx.objectCount) continue;

      RuntimeObject& object = ctx.objects[objectIndex];
      if (!object.active || !object.visible || !zombie.alive()) continue;

      const ZombieState previousState = zombie.state();
      const bool attacked =
          zombie.update(object, survivalPlayer().position(), gameplayNoise(),
                        ctx.objects, ctx.objectCount, g_frameDt);
      if (zombie.state() == ZombieState::Chase &&
          previousState != ZombieState::Chase &&
          previousState != ZombieState::Attack) {
        playSurvivalSound(ctx, SurvivalSound::ZombieAttack);
      }

      if (attacked && survivalPlayer().damage(kAttackDamage)) {
        playSurvivalSound(ctx, SurvivalSound::ZombieAttack);
        TYRA_LOG("Zombie: enemy ", i + 1, " attacks; player HP ",
                 survivalPlayer().health());
      }
    }
  }
};

}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalZombieScript);
