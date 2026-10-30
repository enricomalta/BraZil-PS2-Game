#pragma once

#include "gameplay/item.hpp"

namespace Deep_forest {

class Inventory {
 public:
  void reset() {
    for (int i = 0; i < kInventoryMaxSlotCount; ++i) slots_[i] = Item();
    backpackEquipped_ = false;
    vestEquipped_ = false;
    helmetEquipped_ = false;
  }

  int add(const ItemDefinition& definition, int quantity) {
    if (definition.id <= 0 || !definition.name || definition.maxStack <= 0 ||
        quantity <= 0)
      return 0;

    int remaining = quantity;
    for (int i = 0; i < slotCount() && remaining > 0; ++i) {
      Item& slot = slots_[i];
      if (slot.id != definition.id || slot.quantity >= slot.maxStack) continue;
      const int space = slot.maxStack - slot.quantity;
      const int amount = remaining < space ? remaining : space;
      slot.quantity += amount;
      remaining -= amount;
    }

    for (int i = 0; i < slotCount() && remaining > 0; ++i) {
      Item& slot = slots_[i];
      if (slot.id != 0) continue;
      const int amount =
          remaining < definition.maxStack ? remaining : definition.maxStack;
      slot.id = definition.id;
      slot.name = definition.name;
      slot.type = definition.type;
      slot.quantity = amount;
      slot.weight = definition.weight;
      slot.maxStack = definition.maxStack;
      if (definition.type == ItemType::Ammo) slot.ammoType = definition.id;
      remaining -= amount;
    }
    return quantity - remaining;
  }

  bool addMagazine(int rounds, int capacity = kPistolMagazineCapacity) {
    if (rounds < 0 || capacity <= 0 || rounds > capacity) return false;
    const ItemDefinition* definition = itemDefinition(kItemPistolMagazine);
    if (!definition) return false;
    const int index = firstEmptySlot();
    if (index < 0) return false;
    slots_[index] = Item();
    slots_[index].id = definition->id;
    slots_[index].name = definition->name;
    slots_[index].type = definition->type;
    slots_[index].quantity = 1;
    slots_[index].weight = definition->weight;
    slots_[index].maxStack = 1;
    slots_[index].ammoCount = rounds;
    slots_[index].ammoCapacity = capacity;
    slots_[index].ammoType = kItemAmmo9mm;
    return true;
  }

  int addItem(const Item& item, int quantity) {
    if (item.id <= 0 || quantity <= 0) return 0;
    const ItemDefinition* definition = itemDefinition(item.id);
    if (!definition) return 0;
    if (item.type == ItemType::Magazine) {
      int added = 0;
      for (; added < quantity; ++added)
        if (!addMagazine(item.ammoCount, item.ammoCapacity)) break;
      return added;
    }

    const int added = add(*definition, quantity);
    if (added > 0 && definition->type == ItemType::Ammo) {
      for (int i = 0; i < slotCount(); ++i)
        if (slots_[i].id == item.id) slots_[i].ammoType = item.ammoType;
    }
    return added;
  }

  bool remove(int id, int quantity) {
    if (id <= 0 || quantity <= 0 || count(id) < quantity) return false;

    int remaining = quantity;
    for (int i = 0; i < slotCount() && remaining > 0; ++i) {
      if (slots_[i].id != id) continue;
      const int amount =
          remaining < slots_[i].quantity ? remaining : slots_[i].quantity;
      slots_[i].quantity -= amount;
      remaining -= amount;
      if (slots_[i].quantity == 0) slots_[i] = Item();
    }
    return remaining == 0;
  }

  bool removeFromSlot(int index, int quantity) {
    if (index < 0 || index >= slotCount() || quantity <= 0 ||
        slots_[index].id == 0 || slots_[index].quantity < quantity)
      return false;
    slots_[index].quantity -= quantity;
    if (slots_[index].quantity == 0) slots_[index] = Item();
    return true;
  }

  int count(int id) const {
    int total = 0;
    for (int i = 0; i < slotCount(); ++i)
      if (slots_[i].id == id) total += slots_[i].quantity;
    return total;
  }

  int firstSlotOfType(ItemType type) const {
    for (int i = 0; i < slotCount(); ++i)
      if (slots_[i].id != 0 && slots_[i].type == type) return i;
    return -1;
  }

  int firstLoadedMagazineSlot() const {
    for (int i = 0; i < slotCount(); ++i)
      if (slots_[i].id == kItemPistolMagazine && slots_[i].ammoCount > 0)
        return i;
    return -1;
  }

  int firstEmptySlot() const {
    for (int i = 0; i < slotCount(); ++i)
      if (slots_[i].id == 0) return i;
    return -1;
  }

  bool loadMagazine(int magazineSlot, int ammoSlot) {
    if (magazineSlot < 0 || magazineSlot >= slotCount() || ammoSlot < 0 ||
        ammoSlot >= slotCount() || magazineSlot == ammoSlot)
      return false;
    Item& magazine = slots_[magazineSlot];
    Item& ammo = slots_[ammoSlot];
    if (magazine.id != kItemPistolMagazine ||
        magazine.ammoCount >= magazine.ammoCapacity ||
        ammo.id != kItemAmmo9mm || ammo.quantity <= 0 ||
        ammo.ammoType != magazine.ammoType)
      return false;
    --ammo.quantity;
    ++magazine.ammoCount;
    if (ammo.quantity == 0) ammo = Item();
    return true;
  }

  int firstAmmoSlot(int ammoType) const {
    for (int i = 0; i < slotCount(); ++i)
      if (slots_[i].id == kItemAmmo9mm && slots_[i].quantity > 0 &&
          slots_[i].ammoType == ammoType)
        return i;
    return -1;
  }

  Item* mutableSlot(int index) {
    return index >= 0 && index < slotCount() ? &slots_[index] : nullptr;
  }

  const Item* slot(int index) const {
    return index >= 0 && index < slotCount() ? &slots_[index] : nullptr;
  }

  int slotCount() const {
    return backpackEquipped_ ? kInventoryMaxSlotCount
                             : kInventoryBaseSlotCount;
  }
  bool backpackEquipped() const { return backpackEquipped_; }
  bool vestEquipped() const { return vestEquipped_; }
  bool helmetEquipped() const { return helmetEquipped_; }

  bool toggleEquipment(int id) {
    if (count(id) <= 0) return false;
    if (id == kItemBackpack) {
      const bool next = !backpackEquipped_;
      if (!next) {
        for (int i = kInventoryBaseSlotCount;
             i < kInventoryMaxSlotCount; ++i)
          if (slots_[i].id != 0) return false;
      }
      backpackEquipped_ = next;
      return true;
    }
    if (id == kItemVest) {
      vestEquipped_ = !vestEquipped_;
      return true;
    }
    if (id == kItemHelmet) {
      helmetEquipped_ = !helmetEquipped_;
      return true;
    }
    return false;
  }

 private:
  Item slots_[kInventoryMaxSlotCount];
  bool backpackEquipped_ = false;
  bool vestEquipped_ = false;
  bool helmetEquipped_ = false;
};

Inventory& survivalInventory();
int& survivalInventorySelection();
bool survivalInventoryOpen();
int survivalInventoryDropQuantity();
const char* survivalInventoryMessage();
bool survivalInventoryToastVisible();

}  // namespace Deep_forest
