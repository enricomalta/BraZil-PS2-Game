#pragma once

#include "gameplay/inventory.hpp"

namespace Deep_forest {

enum class ShotResult {
  Fired,
  Empty,
  CoolingDown,
  NoWeapon
};

enum class ReloadResult {
  Reloaded,
  NoWeapon,
  NoLoadedMagazine
};

class Weapon {
 public:
  void reset() {
    owned_ = false;
    equipped_ = false;
    dropped_ = false;
    magazine_ = Item();
    cooldown_ = 0.0F;
    shotSequence_ = 0;
    reloadSequence_ = 0;
  }

  void acquire() {
    owned_ = true;
    equipped_ = true;
    dropped_ = false;
  }
  void toggleEquipped() {
    if (owned_) equipped_ = !equipped_;
  }
  void draw() {
    if (owned_) equipped_ = true;
  }
  void holster() { equipped_ = false; }
  // Dropping the pistol hands it back to the world; only while holstered.
  bool release() {
    if (!owned_ || equipped_) return false;
    owned_ = false;
    dropped_ = true;
    return true;
  }
  bool owned() const { return owned_; }
  bool equipped() const { return equipped_; }
  // True while the pistol lies where the player dropped it (not at its spawn).
  bool dropped() const { return dropped_; }

  void update(float deltaTime) {
    if (deltaTime <= 0.0F || cooldown_ <= 0.0F) return;
    cooldown_ = deltaTime >= cooldown_ ? 0.0F : cooldown_ - deltaTime;
  }

  ShotResult fire() {
    if (!equipped_) return ShotResult::NoWeapon;
    if (cooldown_ > 0.0F) return ShotResult::CoolingDown;
    if (magazine_.id != kItemPistolMagazine || magazine_.ammoCount <= 0)
      return ShotResult::Empty;

    --magazine_.ammoCount;
    cooldown_ = 0.35F;
    ++shotSequence_;
    return ShotResult::Fired;
  }

  ReloadResult reload(Inventory& inventory, int preferredSlot = -1) {
    if (!equipped_) return ReloadResult::NoWeapon;
    int slotIndex = preferredSlot;
    const Item* candidate = inventory.slot(slotIndex);
    if (!candidate || candidate->id != kItemPistolMagazine ||
        candidate->ammoCount <= 0) {
      slotIndex = inventory.firstLoadedMagazineSlot();
      candidate = inventory.slot(slotIndex);
    }
    if (!candidate || candidate->id != kItemPistolMagazine ||
        candidate->ammoCount <= 0)
      return ReloadResult::NoLoadedMagazine;

    Item* selected = inventory.mutableSlot(slotIndex);
    if (!selected) return ReloadResult::NoLoadedMagazine;
    const Item previous = magazine_;
    const Item placement = *selected;
    magazine_ = *selected;
    *selected = previous;
    // The magazine swapped into the grid takes over the spot just vacated.
    if (selected->id != 0) {
      selected->container = placement.container;
      selected->gridX = placement.gridX;
      selected->gridY = placement.gridY;
      selected->rotated = placement.rotated;
    }
    ++reloadSequence_;
    return ReloadResult::Reloaded;
  }

  int magazineAmmo() const { return magazine_.ammoCount; }
  int magazineCapacity() const { return magazine_.ammoCapacity; }
  int reserveAmmo(const Inventory& inventory) const {
    return inventory.count(kItemAmmo9mm);
  }
  unsigned int shotSequence() const { return shotSequence_; }
  unsigned int reloadSequence() const { return reloadSequence_; }

 private:
  Item magazine_;
  float cooldown_ = 0.0F;
  unsigned int shotSequence_ = 0;
  unsigned int reloadSequence_ = 0;
  bool owned_ = false;
  bool dropped_ = false;
  bool equipped_ = false;
};

Weapon& survivalWeapon();

}  // namespace Deep_forest
