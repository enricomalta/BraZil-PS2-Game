// Host-side unit test for the cargo grids and equipment slots.
// Build (VS developer prompt): cl /nologo /EHsc /std:c++17 /I inc tests\inventory_test.cpp
#include <stdio.h>

#include "gameplay/inventory.hpp"

using namespace Deep_forest;

static int failures = 0;
#define CHECK(cond)                                  \
  do {                                               \
    if (!(cond)) {                                   \
      printf("FAIL line %d: %s\n", __LINE__, #cond); \
      ++failures;                                    \
    }                                                \
  } while (0)

static const ItemDefinition& def(int id) { return *itemDefinition(id); }

static bool overlapFree(const Inventory& inv) {
  for (int a = 0; a < kInventoryRecordCount; ++a)
    for (int b = a + 1; b < kInventoryRecordCount; ++b) {
      const Item* ia = inv.slot(a);
      const Item* ib = inv.slot(b);
      if (ia->id == 0 || ib->id == 0 || ia->container != ib->container)
        continue;
      if (ia->gridX < ib->gridX + ib->footprintW() &&
          ib->gridX < ia->gridX + ia->footprintW() &&
          ia->gridY < ib->gridY + ib->footprintH() &&
          ib->gridY < ia->gridY + ia->footprintH())
        return false;
    }
  return true;
}

int main() {
  Inventory inv;
  inv.reset();

  // Pockets are 4x3 = 12 cells.
  CHECK(inv.totalCells(0) == 12);
  CHECK(inv.totalCells(1) == 0);
  CHECK(inv.add(def(kItemWater), 6) == 6);  // 2 stacks, 1x2 each
  CHECK(inv.usedCells(0) == 4);
  CHECK(inv.add(def(kItemPistol), 1) == 1);  // 2x2
  CHECK(inv.usedCells(0) == 8);
  CHECK(inv.add(def(kItemMedkit), 4) == 4);  // 2 stacks, 2x1 each
  CHECK(inv.usedCells(0) == 12);
  CHECK(inv.add(def(kItemAmmo9mm), 1) == 0);  // full
  CHECK(!inv.addMagazine(0));
  CHECK(overlapFree(inv));

  // Stacking: partial stacks fill before new records are made.
  inv.reset();
  CHECK(inv.add(def(kItemAmmo9mm), 24) == 24);
  CHECK(inv.add(def(kItemAmmo9mm), 10) == 10);
  CHECK(inv.count(kItemAmmo9mm) == 34);
  CHECK(inv.usedCells(0) == 2);

  // Backpack: equipping opens a 5x5 grid; taking it off needs it empty.
  inv.reset();
  CHECK(inv.add(def(kItemBackpack), 1) == 1);
  const int bag = inv.firstSlotOfType(ItemType::Equipment);
  CHECK(bag >= 0);
  CHECK(inv.equip(bag) == EquipResult::Ok);
  CHECK(inv.backpackEquipped());
  CHECK(inv.totalCells(1) == 25);
  CHECK(inv.slot(bag)->id == 0);
  CHECK(inv.add(def(kItemWater), 30) == 30);  // 10 stacks: 12 cells + 8 cells
  CHECK(inv.usedCells(0) == 12);
  CHECK(inv.usedCells(1) == 8);
  CHECK(overlapFree(inv));
  CHECK(inv.unequip(EquipSlot::Backpack) == EquipResult::BackpackNotEmpty);
  CHECK(inv.backpackEquipped());
  inv.remove(kItemWater, 30);
  CHECK(inv.unequip(EquipSlot::Backpack) == EquipResult::Ok);
  CHECK(!inv.backpackEquipped());
  CHECK(inv.totalCells(1) == 0);

  // Equipment slot already taken, then freed.
  inv.reset();
  inv.add(def(kItemVest), 1);
  CHECK(inv.equip(inv.firstSlotOfType(ItemType::Equipment)) ==
        EquipResult::Ok);
  inv.add(def(kItemVest), 1);
  CHECK(inv.equip(inv.firstSlotOfType(ItemType::Equipment)) ==
        EquipResult::SlotOccupied);
  CHECK(inv.vestEquipped());
  CHECK(inv.unequip(EquipSlot::Vest) == EquipResult::NoSpace);
  inv.remove(kItemVest, 1);
  CHECK(inv.unequip(EquipSlot::Vest) == EquipResult::Ok);
  CHECK(!inv.vestEquipped());

  // Non-equippable item.
  inv.reset();
  inv.add(def(kItemBandage), 1);
  CHECK(inv.equip(0) == EquipResult::NotEquippable);

  // Weight and load factor.
  inv.reset();
  CHECK(inv.totalWeight() == 0.0F && inv.loadFactor() == 0.0F);
  inv.add(def(kItemWater), 3);  // 1.5 kg
  CHECK(inv.totalWeight() > 1.49F && inv.totalWeight() < 1.51F);
  CHECK(inv.loadFactor() == 0.0F);
  inv.add(def(kItemBackpack), 1);
  inv.equip(inv.firstSlotOfType(ItemType::Equipment));
  CHECK(inv.totalWeight() > 2.49F && inv.totalWeight() < 2.51F);

  // Pistol needs the holster; the holster cannot leave while it holds one.
  inv.reset();
  inv.add(def(kItemPistol), 1);
  CHECK(inv.equip(0) == EquipResult::NeedsHolster);
  inv.add(def(kItemHolster), 1);
  CHECK(inv.equip(inv.firstSlotOfType(ItemType::Equipment)) ==
        EquipResult::Ok);
  CHECK(inv.holsterEquipped());
  CHECK(inv.equip(inv.firstSlotOfType(ItemType::Weapon)) == EquipResult::Ok);
  CHECK(inv.equipped(EquipSlot::WeaponSecondary) != nullptr);
  CHECK(inv.equipped(EquipSlot::Head) == nullptr);
  CHECK(inv.unequip(EquipSlot::Holster) == EquipResult::HolsterNotEmpty);
  CHECK(inv.unequip(EquipSlot::WeaponSecondary) == EquipResult::Ok);
  CHECK(inv.unequip(EquipSlot::Holster) == EquipResult::Ok);

  // Unequip into a full grid keeps the item equipped.
  inv.reset();
  inv.add(def(kItemHelmet), 1);
  CHECK(inv.equip(0) == EquipResult::Ok);
  inv.add(def(kItemWater), 12);  // 4 stacks: 8 cells
  inv.add(def(kItemMedkit), 4);  // 2 stacks of 2x1: full
  CHECK(inv.usedCells(0) == 12);
  CHECK(inv.unequip(EquipSlot::Head) == EquipResult::NoSpace);
  CHECK(inv.helmetEquipped());

  // Rotation and moving.
  inv.reset();
  inv.add(def(kItemWater), 1);  // 1x2 at (0,0)
  CHECK(inv.slot(0)->footprintW() == 1 && inv.slot(0)->footprintH() == 2);
  CHECK(inv.rotateItem(0));
  CHECK(inv.slot(0)->footprintW() == 2 && inv.slot(0)->footprintH() == 1);
  CHECK(inv.moveItem(0, 0, 2, 2, true));
  CHECK(!inv.moveItem(0, 0, 3, 2, true));   // out of bounds
  CHECK(!inv.moveItem(0, 1, 0, 0, false));  // backpack grid inactive
  inv.add(def(kItemWater), 4);              // two more records
  CHECK(!inv.moveItem(1, 0, 2, 2, false));  // overlaps record 0
  CHECK(overlapFree(inv));

  // Magazine loading and removal.
  inv.reset();
  CHECK(inv.addMagazine(0));
  CHECK(inv.add(def(kItemAmmo9mm), 5) == 5);
  const int mag = inv.firstSlotOfType(ItemType::Magazine);
  const int ammo = inv.firstAmmoSlot(kItemAmmo9mm);
  CHECK(inv.loadMagazine(mag, ammo));
  CHECK(inv.slot(mag)->ammoCount == 1);
  CHECK(inv.remove(kItemAmmo9mm, 4));
  CHECK(inv.count(kItemAmmo9mm) == 0);

  printf(failures ? "%d FAILURE(S)\n" : "ALL PASSED\n", failures);
  return failures ? 1 : 0;
}
