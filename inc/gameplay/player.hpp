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
  bool damage(float amount);
  void reset();

  const PlayerPosition& position() const { return position_; }
  float facingX() const { return facingX_; }
  float facingZ() const { return facingZ_; }
  float health() const { return health_; }
  float maxHealth() const { return 100.0F; }
  bool alive() const { return health_ > 0.0F; }

 private:
  PlayerPosition position_;
  float facingX_ = 0.0F;
  float facingZ_ = 1.0F;
  float health_ = 100.0F;
};

Player& survivalPlayer();

}  // namespace Deep_forest
