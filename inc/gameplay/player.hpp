#pragma once

namespace Deep_forest {

struct PlayerPosition {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;
};

class Player {
 public:
  void synchronize(float x, float y, float z, float lookX, float lookZ);
  bool damage(float amount, bool headshot = false);
  bool heal(float amount);
  void updateSurvival(float deltaTime, bool sprinting);
  void restoreHunger(float amount);
  void restoreThirst(float amount);
  void startBleeding();
  void stopBleeding();
  void setArmor(bool vest, bool helmet);
  void reset();

  const PlayerPosition& position() const { return position_; }
  float facingX() const { return facingX_; }
  float facingZ() const { return facingZ_; }
  float health() const { return health_; }
  float maxHealth() const { return 100.0F; }
  float hunger() const { return hunger_; }
  float thirst() const { return thirst_; }
  float stamina() const { return stamina_; }
  bool bleeding() const { return bleeding_; }
  bool alive() const { return health_ > 0.0F; }

 private:
  PlayerPosition position_;
  float facingX_ = 0.0F;
  float facingZ_ = 1.0F;
  float health_ = 100.0F;
  float hunger_ = 100.0F;
  float thirst_ = 100.0F;
  float stamina_ = 100.0F;
  float bleedTimer_ = 0.0F;
  bool bleeding_ = false;
  bool vest_ = false;
  bool helmet_ = false;
};

Player& survivalPlayer();

}  // namespace Deep_forest
