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

struct ItemDefinition {
  int id;
  const char* name;
  ItemType type;
  float weight;
  int maxStack;
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
constexpr int kInventoryBaseSlotCount = 8;
constexpr int kInventoryMaxSlotCount = 12;
constexpr int kPistolMagazineCapacity = 8;

inline const ItemDefinition* itemDefinition(int id) {
  static const ItemDefinition definitions[] = {
      {kItemBandage, "Bandagem", ItemType::Medical, 0.2F, 5},
      {kItemPistol, "Pistola 9mm", ItemType::Weapon, 1.0F, 1},
      {kItemPistolMagazine, "Carregador 9mm", ItemType::Magazine, 0.15F, 1},
      {kItemAmmo9mm, "Municao 9mm", ItemType::Ammo, 0.01F, 30},
      {kItemCannedFood, "Comida enlatada", ItemType::Food, 0.35F, 3},
      {kItemWater, "Agua", ItemType::Drink, 0.5F, 3},
      {kItemMedkit, "Kit medico", ItemType::Medical, 0.6F, 2},
      {kItemBackpack, "Mochila", ItemType::Equipment, 1.0F, 1},
      {kItemVest, "Colete", ItemType::Equipment, 1.5F, 1},
      {kItemHelmet, "Capacete", ItemType::Equipment, 0.8F, 1},
  };
  for (const ItemDefinition& definition : definitions)
    if (definition.id == id) return &definition;
  return nullptr;
}

}  // namespace Deep_forest
