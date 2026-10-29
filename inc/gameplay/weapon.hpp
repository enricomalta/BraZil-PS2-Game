#pragma once

namespace Deep_forest {

enum class ShotResult {
  Fired,
  Empty,
  CoolingDown
};

enum class ReloadResult {
  Reloaded,
  MagazineFull,
  ReserveEmpty
};

class Weapon {
 public:
  void reset() {
    magazineAmmo_ = 8;
    reserveAmmo_ = 24;
    cooldown_ = 0.0F;
    shotSequence_ = 0;
    reloadSequence_ = 0;
  }

  void update(float deltaTime) {
    if (deltaTime <= 0.0F || cooldown_ <= 0.0F) return;
    cooldown_ = deltaTime >= cooldown_ ? 0.0F : cooldown_ - deltaTime;
  }

  ShotResult fire() {
    if (cooldown_ > 0.0F) return ShotResult::CoolingDown;
    if (magazineAmmo_ <= 0) return ShotResult::Empty;

    --magazineAmmo_;
    cooldown_ = 0.35F;
    ++shotSequence_;
    return ShotResult::Fired;
  }

  ReloadResult reload() {
    if (magazineAmmo_ >= 8) return ReloadResult::MagazineFull;
    if (reserveAmmo_ <= 0) return ReloadResult::ReserveEmpty;

    int transfer = 8 - magazineAmmo_;
    if (transfer > reserveAmmo_) transfer = reserveAmmo_;
    magazineAmmo_ += transfer;
    reserveAmmo_ -= transfer;
    ++reloadSequence_;
    return ReloadResult::Reloaded;
  }

  int magazineAmmo() const { return magazineAmmo_; }
  int reserveAmmo() const { return reserveAmmo_; }
  unsigned int shotSequence() const { return shotSequence_; }
  unsigned int reloadSequence() const { return reloadSequence_; }

 private:
  int magazineAmmo_ = 8;
  int reserveAmmo_ = 24;
  float cooldown_ = 0.0F;
  unsigned int shotSequence_ = 0;
  unsigned int reloadSequence_ = 0;
};

Weapon& survivalWeapon();

}  // namespace Deep_forest
