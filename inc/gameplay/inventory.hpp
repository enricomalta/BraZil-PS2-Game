#pragma once

#include "gameplay/item.hpp"

namespace Deep_forest {

// Cargo containers. Pockets are always there; the backpack grid exists only
// while a backpack sits in its equipment slot. Vest and pants grids are
// future containers - add them here and in containerWidth/Height.
enum class Container : signed char { Pockets, Backpack, Count };

constexpr int kContainerCount = static_cast<int>(Container::Count);
constexpr int kPocketsW = 4;
constexpr int kPocketsH = 3;
constexpr int kBackpackW = 5;
constexpr int kBackpackH = 5;
// Carried weight in kg: no penalty up to the comfort limit, full penalty at
// the maximum (it only slows and tires the player; nothing is refused).
constexpr float kWeightComfort = 10.0F;
constexpr float kWeightMax = 25.0F;

// Entry indices 0..kInventoryRecordCount-1 are cargo records, the rest are the
// equipment slots in EquipSlot order. One index space keeps selection simple.
constexpr int kInventoryEntryCount = kInventoryRecordCount + kEquipSlotCount;

enum class EquipResult {
  Ok,
  NotEquippable,
  SlotOccupied,
  NoSpace,
  BackpackNotEmpty,
  HolsterNotEmpty,
  NeedsHolster,
  Invalid
};

class Inventory {
 public:
  static int containerWidth(int container) {
    return container == static_cast<int>(Container::Backpack) ? kBackpackW
                                                              : kPocketsW;
  }
  static int containerHeight(int container) {
    return container == static_cast<int>(Container::Backpack) ? kBackpackH
                                                               : kPocketsH;
  }

  void reset() {
    for (int i = 0; i < kInventoryRecordCount; ++i) records_[i] = Item();
    for (int i = 0; i < kEquipSlotCount; ++i) equipment_[i] = Item();
  }

  bool containerActive(int container) const {
    if (container == static_cast<int>(Container::Pockets)) return true;
    return container == static_cast<int>(Container::Backpack) &&
           equipment_[static_cast<int>(EquipSlot::Backpack)].id != 0;
  }

  int usedCells(int container) const {
    int cells = 0;
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id != 0 && records_[i].container == container)
        cells += records_[i].footprintW() * records_[i].footprintH();
    return cells;
  }

  float totalWeight() const {
    float total = 0.0F;
    for (int i = 0; i < kInventoryRecordCount; ++i)
      total += records_[i].weight * static_cast<float>(records_[i].quantity);
    for (int i = 0; i < kEquipSlotCount; ++i)
      total += equipment_[i].weight * static_cast<float>(equipment_[i].quantity);
    return total;
  }

  // 0 while comfortable, 1 at or above the maximum.
  float loadFactor() const {
    const float factor =
        (totalWeight() - kWeightComfort) / (kWeightMax - kWeightComfort);
    return factor < 0.0F ? 0.0F : (factor > 1.0F ? 1.0F : factor);
  }

  int totalCells(int container) const {
    return containerActive(container)
               ? containerWidth(container) * containerHeight(container)
               : 0;
  }

  // True when the w x h rectangle at (x, y) is inside an active container and
  // overlaps no other record. `ignore` is a record being moved or rotated.
  bool canPlace(int container, int x, int y, int w, int h,
                int ignore = -1) const {
    if (container < 0 || container >= kContainerCount ||
        !containerActive(container) || w <= 0 || h <= 0 || x < 0 || y < 0 ||
        x + w > containerWidth(container) ||
        y + h > containerHeight(container))
      return false;
    for (int i = 0; i < kInventoryRecordCount; ++i) {
      const Item& other = records_[i];
      if (i == ignore || other.id == 0 || other.container != container)
        continue;
      if (x < other.gridX + other.footprintW() && other.gridX < x + w &&
          y < other.gridY + other.footprintH() && other.gridY < y + h)
        return false;
    }
    return true;
  }

  // Fills item.container/gridX/gridY/rotated with the first free spot, trying
  // the upright footprint before the rotated one in every active container.
  bool findSpot(Item& item) const {
    for (int c = 0; c < kContainerCount; ++c) {
      if (!containerActive(c)) continue;
      for (int turn = 0; turn < 2; ++turn) {
        if (turn == 1 && item.width == item.height) break;
        const int w = turn ? item.height : item.width;
        const int h = turn ? item.width : item.height;
        for (int y = 0; y + h <= containerHeight(c); ++y)
          for (int x = 0; x + w <= containerWidth(c); ++x)
            if (canPlace(c, x, y, w, h)) {
              item.container = static_cast<signed char>(c);
              item.gridX = static_cast<unsigned char>(x);
              item.gridY = static_cast<unsigned char>(y);
              item.rotated = turn == 1;
              return true;
            }
      }
    }
    return false;
  }

  static Item makeItem(const ItemDefinition& definition, int quantity) {
    Item item;
    item.id = definition.id;
    item.name = definition.name;
    item.type = definition.type;
    item.quantity = quantity;
    item.weight = definition.weight;
    item.maxStack = definition.maxStack;
    item.slot = definition.slot;
    item.width = static_cast<unsigned char>(definition.width);
    item.height = static_cast<unsigned char>(definition.height);
    if (definition.type == ItemType::Ammo) item.ammoType = definition.id;
    return item;
  }

  // Stores a fully built item in the first free record and grid spot.
  int place(Item item) {
    const int record = firstEmptySlot();
    if (record < 0 || !findSpot(item)) return -1;
    records_[record] = item;
    return record;
  }

  // Stores an item at an exact grid spot (used when dragging between two
  // inventories); the caller removes it from the source afterwards.
  bool placeAt(Item item, int container, int x, int y, bool rotated) {
    const int w = rotated ? item.height : item.width;
    const int h = rotated ? item.width : item.height;
    const int record = firstEmptySlot();
    if (record < 0 || !canPlace(container, x, y, w, h)) return false;
    item.container = static_cast<signed char>(container);
    item.gridX = static_cast<unsigned char>(x);
    item.gridY = static_cast<unsigned char>(y);
    item.rotated = rotated;
    records_[record] = item;
    return true;
  }

  int add(const ItemDefinition& definition, int quantity) {
    if (definition.id <= 0 || !definition.name || definition.maxStack <= 0 ||
        definition.width <= 0 || definition.height <= 0 || quantity <= 0)
      return 0;

    int remaining = quantity;
    for (int i = 0; i < kInventoryRecordCount && remaining > 0; ++i) {
      Item& slot = records_[i];
      if (slot.id != definition.id || slot.quantity >= slot.maxStack) continue;
      const int space = slot.maxStack - slot.quantity;
      const int amount = remaining < space ? remaining : space;
      slot.quantity += amount;
      remaining -= amount;
    }

    while (remaining > 0) {
      const int amount =
          remaining < definition.maxStack ? remaining : definition.maxStack;
      if (place(makeItem(definition, amount)) < 0) break;
      remaining -= amount;
    }
    return quantity - remaining;
  }

  bool addMagazine(int rounds, int capacity = kPistolMagazineCapacity) {
    if (rounds < 0 || capacity <= 0 || rounds > capacity) return false;
    const ItemDefinition* definition = itemDefinition(kItemPistolMagazine);
    if (!definition) return false;
    Item magazine = makeItem(*definition, 1);
    magazine.maxStack = 1;
    magazine.ammoCount = rounds;
    magazine.ammoCapacity = capacity;
    magazine.ammoType = kItemAmmo9mm;
    return place(magazine) >= 0;
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
      for (int i = 0; i < kInventoryRecordCount; ++i)
        if (records_[i].id == item.id) records_[i].ammoType = item.ammoType;
    }
    return added;
  }

  bool remove(int id, int quantity) {
    if (id <= 0 || quantity <= 0 || count(id) < quantity) return false;

    int remaining = quantity;
    for (int i = 0; i < kInventoryRecordCount && remaining > 0; ++i) {
      if (records_[i].id != id) continue;
      const int amount =
          remaining < records_[i].quantity ? remaining : records_[i].quantity;
      records_[i].quantity -= amount;
      remaining -= amount;
      if (records_[i].quantity == 0) records_[i] = Item();
    }
    return remaining == 0;
  }

  bool removeFromSlot(int index, int quantity) {
    if (index < 0 || index >= kInventoryRecordCount || quantity <= 0 ||
        records_[index].id == 0 || records_[index].quantity < quantity)
      return false;
    records_[index].quantity -= quantity;
    if (records_[index].quantity == 0) records_[index] = Item();
    return true;
  }

  int count(int id) const {
    int total = 0;
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id == id) total += records_[i].quantity;
    return total;
  }

  int firstSlotOfType(ItemType type) const {
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id != 0 && records_[i].type == type) return i;
    return -1;
  }

  int firstLoadedMagazineSlot() const {
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id == kItemPistolMagazine && records_[i].ammoCount > 0)
        return i;
    return -1;
  }

  int firstEmptySlot() const {
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id == 0) return i;
    return -1;
  }

  bool loadMagazine(int magazineSlot, int ammoSlot) {
    if (magazineSlot < 0 || magazineSlot >= kInventoryRecordCount ||
        ammoSlot < 0 || ammoSlot >= kInventoryRecordCount ||
        magazineSlot == ammoSlot)
      return false;
    Item& magazine = records_[magazineSlot];
    Item& ammo = records_[ammoSlot];
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
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (records_[i].id == kItemAmmo9mm && records_[i].quantity > 0 &&
          records_[i].ammoType == ammoType)
        return i;
    return -1;
  }

  Item* mutableSlot(int index) {
    if (index < 0 || index >= kInventoryEntryCount) return nullptr;
    return index < kInventoryRecordCount
               ? &records_[index]
               : &equipment_[index - kInventoryRecordCount];
  }

  const Item* slot(int index) const {
    if (index < 0 || index >= kInventoryEntryCount) return nullptr;
    return index < kInventoryRecordCount
               ? &records_[index]
               : &equipment_[index - kInventoryRecordCount];
  }

  int slotCount() const { return kInventoryEntryCount; }

  static bool isEquipmentEntry(int index) {
    return index >= kInventoryRecordCount && index < kInventoryEntryCount;
  }

  static EquipSlot equipSlotOfEntry(int index) {
    return isEquipmentEntry(index)
               ? static_cast<EquipSlot>(index - kInventoryRecordCount)
               : EquipSlot::None;
  }

  // nullptr when the slot is empty.
  const Item* equipped(EquipSlot slot) const {
    const int index = static_cast<int>(slot);
    return index >= 0 && index < kEquipSlotCount && equipment_[index].id != 0
               ? &equipment_[index]
               : nullptr;
  }

  bool backpackEquipped() const {
    return equipment_[static_cast<int>(EquipSlot::Backpack)].id != 0;
  }
  bool vestEquipped() const {
    return equipment_[static_cast<int>(EquipSlot::Vest)].id != 0;
  }
  bool helmetEquipped() const {
    return equipment_[static_cast<int>(EquipSlot::Head)].id != 0;
  }
  bool holsterEquipped() const {
    return equipment_[static_cast<int>(EquipSlot::Holster)].id != 0;
  }

  // Moves a cargo record into the slot its item belongs to.
  EquipResult equip(int record) {
    if (record < 0 || record >= kInventoryRecordCount ||
        records_[record].id == 0)
      return EquipResult::Invalid;
    const int slotIndex = static_cast<int>(records_[record].slot);
    if (slotIndex < 0 || slotIndex >= kEquipSlotCount)
      return EquipResult::NotEquippable;
    if (equipment_[slotIndex].id != 0) return EquipResult::SlotOccupied;
    if (records_[record].slot == EquipSlot::WeaponSecondary &&
        !holsterEquipped())
      return EquipResult::NeedsHolster;
    equipment_[slotIndex] = records_[record];
    records_[record] = Item();
    return EquipResult::Ok;
  }

  // Moves an equipped item back into the cargo grids.
  EquipResult unequip(EquipSlot slot) {
    const int slotIndex = static_cast<int>(slot);
    if (slotIndex < 0 || slotIndex >= kEquipSlotCount ||
        equipment_[slotIndex].id == 0)
      return EquipResult::Invalid;
    if (slot == EquipSlot::Backpack && usedCells(
            static_cast<int>(Container::Backpack)) > 0)
      return EquipResult::BackpackNotEmpty;
    if (slot == EquipSlot::Holster &&
        equipment_[static_cast<int>(EquipSlot::WeaponSecondary)].id != 0)
      return EquipResult::HolsterNotEmpty;

    const Item item = equipment_[slotIndex];
    equipment_[slotIndex] = Item();  // the backpack's own grid closes first
    Item placed = item;
    const int record = firstEmptySlot();
    if (record < 0 || !findSpot(placed)) {
      equipment_[slotIndex] = item;
      return EquipResult::NoSpace;
    }
    records_[record] = placed;
    return EquipResult::Ok;
  }

  // Equip toggle for whichever entry the cursor is on.
  EquipResult toggleEquipment(int entry) {
    if (isEquipmentEntry(entry)) return unequip(equipSlotOfEntry(entry));
    return equip(entry);
  }

  bool moveItem(int record, int container, int x, int y, bool rotated) {
    if (record < 0 || record >= kInventoryRecordCount ||
        records_[record].id == 0)
      return false;
    Item& item = records_[record];
    const int w = rotated ? item.height : item.width;
    const int h = rotated ? item.width : item.height;
    if (!canPlace(container, x, y, w, h, record)) return false;
    item.container = static_cast<signed char>(container);
    item.gridX = static_cast<unsigned char>(x);
    item.gridY = static_cast<unsigned char>(y);
    item.rotated = rotated;
    return true;
  }

  bool rotateItem(int record) {
    if (record < 0 || record >= kInventoryRecordCount ||
        records_[record].id == 0)
      return false;
    const Item& item = records_[record];
    return moveItem(record, item.container, item.gridX, item.gridY,
                    !item.rotated);
  }

 private:
  Item records_[kInventoryRecordCount];
  Item equipment_[kEquipSlotCount];
};

struct InventoryGrab {
  bool active = false;
  bool fromCrate = false;  // which inventory `record` belongs to
  int record = -1;
  int container = 0;  // 0 pockets, 1 backpack, 2 crate grid
  int x = 0;
  int y = 0;
  bool rotated = false;
};

Inventory& survivalInventory();
// The open loot crate's contents, or nullptr while no crate is open.
const Inventory* survivalCrate();
bool survivalCrateFocused();
int survivalCrateSelection();
// Set every frame by the game loop: a menu or the debugger owns the pad.
// (Not a ScriptContext field: the editor regenerates script.hpp.)
extern bool g_survivalPadOwned;
const InventoryGrab& survivalInventoryGrab();
int& survivalInventorySelection();
bool survivalInventoryOpen();
int survivalInventoryDropQuantity();
bool survivalInventoryDropMode();
int survivalCursorPanel();
int survivalCursorX();
int survivalCursorY();
const char* survivalInventoryMessage();
bool survivalInventoryToastVisible();

}  // namespace Deep_forest
