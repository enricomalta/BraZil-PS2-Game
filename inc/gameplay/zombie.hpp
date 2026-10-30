#pragma once

#include "gameplay/player.hpp"
#include "scripts/script.hpp"

namespace Deep_forest {

constexpr int kMaxSurvivalZombies = 8;

struct GameplayNoise {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;
  float radius = 0.0F;
  unsigned int sequence = 0;
};

enum class ZombieState {
  Idle,
  Investigate,
  Search,
  Chase,
  Attack,
  Stagger,
  Dead
};

class Zombie {
 public:
  void reset(int objectIndex, float startX, float startZ);
  bool damage(float amount);
  bool update(RuntimeObject& object, const PlayerPosition& player,
              const GameplayNoise& noise, const RuntimeObject* objects,
              int objectCount, float deltaTime);

  int objectIndex() const { return objectIndex_; }
  float health() const { return health_; }
  bool alive() const { return state_ != ZombieState::Dead; }
  ZombieState state() const { return state_; }

 private:
  bool moveToward(RuntimeObject& object, float x, float z, float speed,
                  const RuntimeObject* objects, int objectCount,
                  float deltaTime);
  bool isBlocked(float x, float z, const RuntimeObject* objects,
                 int objectCount) const;
  void updatePatrol(RuntimeObject& object, const RuntimeObject* objects,
                    int objectCount, float deltaTime);

  int objectIndex_ = -1;
  float health_ = 100.0F;
  float attackCooldown_ = 0.0F;
  float investigateX_ = 0.0F;
  float investigateZ_ = 0.0F;
  float homeX_ = 0.0F;
  float homeZ_ = 0.0F;
  float patrolX_ = 0.0F;
  float patrolZ_ = 0.0F;
  float patrolTimer_ = 0.0F;
  float searchTimer_ = 0.0F;
  float staggerTimer_ = 0.0F;
  unsigned int lastNoiseSequence_ = 0;
  unsigned char patrolIndex_ = 0;
  bool patrolMoving_ = false;
  ZombieState state_ = ZombieState::Idle;
};

Zombie& survivalZombie(int index);
int survivalZombieCount();
void initializeSurvivalZombies(ScriptContext& ctx);
void publishGameplayNoise(float x, float y, float z, float radius);
GameplayNoise gameplayNoise();
void resetGameplayNoise();

}  // namespace Deep_forest
