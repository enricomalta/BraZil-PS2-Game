#pragma once

namespace Deep_forest {

enum class ItemType {
  Weapon,
  Ammo,
  Magazine,
  Food,
  Drink,
  Medical,
  Equipment,
  Tool,
  Misc
};

// Where an item can be worn or holstered. Items without a slot only live in
// the cargo grids.
enum class EquipSlot : signed char {
  None = -1,
  Head,
  Torso,
  Vest,
  Legs,
  Feet,
  Hands,
  Backpack,
  Holster,
  WeaponBack1,
  WeaponBack2,
  WeaponSecondary,
  WeaponMelee,
  Count
};

constexpr int kEquipSlotCount = static_cast<int>(EquipSlot::Count);

struct ItemDefinition {
  int id;
  const char* name;
  ItemType type;
  float weight;
  int maxStack;
  int width;   // footprint in grid cells, unrotated
  int height;
  EquipSlot slot;
};

struct Item {
  int id = 0;
  const char* name = nullptr;
  ItemType type = ItemType::Misc;
  int quantity = 0;
  float weight = 0.0F;
  int maxStack = 0;
  int ammoCount = 0;
  int ammoCapacity = 0;
  int ammoType = 0;
  EquipSlot slot = EquipSlot::None;
  // Footprint and placement. container/gridX/gridY are only meaningful for an
  // item that sits in a cargo grid (not for one in an equipment slot).
  unsigned char width = 1;
  unsigned char height = 1;
  signed char container = 0;
  unsigned char gridX = 0;
  unsigned char gridY = 0;
  bool rotated = false;

  int footprintW() const { return rotated ? height : width; }
  int footprintH() const { return rotated ? width : height; }
};

constexpr int kItemBandage = 1;
constexpr int kItemPistol = 2;
constexpr int kItemPistolMagazine = 3;
constexpr int kItemAmmo9mm = 4;
constexpr int kItemCannedFood = 5;
constexpr int kItemWater = 6;
constexpr int kItemMedkit = 7;
constexpr int kItemBackpack = 8;
constexpr int kItemVest = 9;
constexpr int kItemHelmet = 10;
constexpr int kItemHolster = 11;
// Item records the cargo grids can hold at once (a bound, not a layout).
constexpr int kInventoryRecordCount = 24;
constexpr int kPistolMagazineCapacity = 8;

inline const ItemDefinition* itemDefinition(int id) {
  static const ItemDefinition definitions[] = {
      {kItemBandage, "Bandagem", ItemType::Medical, 0.2F, 5, 1, 1,
       EquipSlot::None},
      {kItemPistol, "Pistola 9mm", ItemType::Weapon, 1.0F, 1, 2, 2,
       EquipSlot::WeaponSecondary},
      {kItemPistolMagazine, "Carregador 9mm", ItemType::Magazine, 0.15F, 1, 1,
       2, EquipSlot::None},
      {kItemAmmo9mm, "Municao 9mm", ItemType::Ammo, 0.01F, 30, 1, 1,
       EquipSlot::None},
      {kItemCannedFood, "Comida enlatada", ItemType::Food, 0.35F, 3, 1, 1,
       EquipSlot::None},
      {kItemWater, "Agua", ItemType::Drink, 0.5F, 3, 1, 2, EquipSlot::None},
      {kItemMedkit, "Kit medico", ItemType::Medical, 0.6F, 2, 2, 1,
       EquipSlot::None},
      {kItemBackpack, "Mochila", ItemType::Equipment, 1.0F, 1, 3, 3,
       EquipSlot::Backpack},
      {kItemVest, "Colete", ItemType::Equipment, 1.5F, 1, 3, 2,
       EquipSlot::Vest},
      {kItemHelmet, "Capacete", ItemType::Equipment, 0.8F, 1, 2, 2,
       EquipSlot::Head},
      {kItemHolster, "Coldre", ItemType::Equipment, 0.4F, 1, 2, 2,
       EquipSlot::Holster},
  };
  for (const ItemDefinition& definition : definitions)
    if (definition.id == id) return &definition;
  return nullptr;
}

}  // namespace Deep_forest
