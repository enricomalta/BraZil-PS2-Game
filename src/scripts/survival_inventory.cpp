#include <math.h>
#include <stdio.h>

#include "gameplay/inventory.hpp"
#include "gameplay/player.hpp"
#include "gameplay/weapon.hpp"
#include "input_map.gen.hpp"
#include "scene_data.hpp"
#include "scripts/script.hpp"
#include "terrain_config.hpp"

namespace Deep_forest {
namespace {

constexpr unsigned long long kBandagePickupIdHash = 0x202630d26d7d87d4ULL;
constexpr unsigned long long kPistolPickupIdHash = 0xccf67a3fde7ac92bULL;
constexpr unsigned long long kMagazinePickupAIdHash = 0x205230d26da2c2bfULL;
constexpr unsigned long long kMagazinePickupBIdHash = 0x205430d26da34596ULL;
constexpr unsigned long long kAmmoPickupIdHash = 0x1f2a30d26ca6fdaeULL;
constexpr unsigned long long kFoodPickupIdHash = 0x1f2030d26c9ea929ULL;
constexpr unsigned long long kWaterPickupIdHash = 0x1f2230d26c9f2c00ULL;
constexpr unsigned long long kMedkitPickupIdHash = 0x1f1830d26c96d77bULL;
constexpr unsigned long long kBackpackPickupIdHash = 0x1f1c30d26c9ac052ULL;
constexpr unsigned long long kVestPickupIdHash = 0x1f1230d26c926bcdULL;
constexpr unsigned long long kHelmetPickupIdHash = 0x294d59d272d0403cULL;
constexpr unsigned long long kSupplyCrateAIdHash = 0x294959d272cc5765ULL;
constexpr unsigned long long kSupplyCrateBIdHash = 0x295359d272d4abeaULL;

constexpr float kBandageHealing = 25.0F;
constexpr float kMedkitHealing = 60.0F;
constexpr float kFoodRestore = 40.0F;
constexpr float kWaterRestore = 45.0F;

struct PickupDefinition {
  unsigned long long idHash;
  int itemId;
  int quantity;
  bool magazine;
};

const PickupDefinition kPickups[] = {
    {kPistolPickupIdHash, kItemPistol, 1, false},
    {kMagazinePickupAIdHash, kItemPistolMagazine, 1, true},
    {kMagazinePickupBIdHash, kItemPistolMagazine, 1, true},
    {kAmmoPickupIdHash, kItemAmmo9mm, 24, false},
    {kBandagePickupIdHash, kItemBandage, 1, false},
    {kFoodPickupIdHash, kItemCannedFood, 1, false},
    {kWaterPickupIdHash, kItemWater, 1, false},
    {kMedkitPickupIdHash, kItemMedkit, 1, false},
    {kBackpackPickupIdHash, kItemBackpack, 1, false},
    {kVestPickupIdHash, kItemVest, 1, false},
    {kHelmetPickupIdHash, kItemHelmet, 1, false},
};

Inventory playerInventory;
int playerInventorySelection = 0;
// Cell cursor: panel 0 pockets, 1 backpack, 2 crate, 3 equipment (cursorY is
// then the slot index). The selected entry is derived from it.
int cursorPanel = 0;
int cursorX = 0;
int cursorY = 0;
bool inventoryOpen = false;
InventoryGrab inventoryGrab;
// Loot crates: each keeps its own contents for the whole scene, rolled once the
// first time it is opened. crateActive is the open one (-1 = none).
Inventory crates[2];
bool crateRolled[2] = {false, false};
bool crateDimmed[2] = {false, false};
int crateActive = -1;
int crateObject = -1;
bool crateFocus = true;  // true: cursor is on the crate panel
int crateSelection = 0;
char inventoryMessage[64] = "INVENTARIO FECHADO";
float inventoryMessageTimer = 0.0F;
int dropQuantity = 1;
bool dropMode = false;
// A dropped item re-uses the world object of its pickup type; this remembers
// what that object now holds so picking it up again returns the same stack.
struct DroppedItem {
  int object;
  int quantity;
  int ammoCount;
};
constexpr int kMaxDroppedItems = 16;
DroppedItem droppedItems[kMaxDroppedItems];
int droppedItemCount = 0;

int findDroppedItem(int object) {
  for (int i = 0; i < droppedItemCount; ++i)
    if (droppedItems[i].object == object) return i;
  return -1;
}

struct LootDrop {
  int itemId;
  int quantity;
  bool magazine;
};

LootDrop rollContainerLoot(unsigned long long idHash, int scene, int index) {
  unsigned int state = static_cast<unsigned int>(idHash) ^
                       (static_cast<unsigned int>(scene) * 0x9E3779B9U) ^
                       (static_cast<unsigned int>(index + 1) * 0x85EBCA6BU);
  state = state * 1664525U + 1013904223U;
  switch ((state >> 16) % 9U) {
    case 0:
    case 1:
      return {kItemCannedFood, 1, false};
    case 2:
    case 3:
      return {kItemWater, 1, false};
    case 4:
      return {kItemBandage, 1, false};
    case 5:
      return {kItemAmmo9mm, 8, false};
    case 6:
      return {kItemPistolMagazine, 1, true};
    case 7:
      return {kItemHolster, 1, false};
    default:
      return {kItemMedkit, 1, false};
  }
}

void setInventoryMessage(const char* message) {
  snprintf(inventoryMessage, sizeof(inventoryMessage), "%s",
           message ? message : "");
  inventoryMessageTimer = inventoryMessage[0] ? 2.5F : 0.0F;
}

class SurvivalInventoryScript : public Script {
 public:
  void update(ScriptContext& ctx) override {
    if (!initialized_ || sceneGeneration_ != ctx.sceneGeneration) {
      initialized_ = true;
      sceneGeneration_ = ctx.sceneGeneration;
      playerInventory.reset();
      playerInventorySelection = 0;
      cursorPanel = cursorX = cursorY = 0;
      dropMode = false;
      inventoryGrab = InventoryGrab();
      crateActive = -1;
      crateObject = -1;
      for (int i = 0; i < 2; ++i) {
        crates[i].reset();
        crateRolled[i] = false;
        crateDimmed[i] = false;
      }
      loadTimer_ = 0.0F;
      droppedItemCount = 0;
      setInventoryMessage("");
    }

    if (!survivalPlayer().alive() || !ctx.engine || ctx.menuEvent >= 0) return;
    if (!inventoryOpen && inventoryMessageTimer > 0.0F) {
      inventoryMessageTimer =
          g_frameDt >= inventoryMessageTimer
              ? 0.0F
              : inventoryMessageTimer - g_frameDt;
    }
    collectPickup(ctx);

    Tyra::Pad& pad = ctx.engine->pad;
    const bool openedThisFrame =
        !inventoryOpen && inputClicked(pad, IA_ROLE_ALT);
    if (openedThisFrame) {
      inventoryOpen = true;
      // setInventoryMessage("X USA/EQUIPA  BOLINHA CARREGA");
    } else if (inventoryOpen && inputClicked(pad, IA_ROLE_BACK)) {
      if (dropMode) {
        dropMode = false;
        dropQuantity = 1;
        setInventoryMessage("DROP CANCELADO");
      } else if (inventoryGrab.active) {
        inventoryGrab.active = false;
        setInventoryMessage("MOVER CANCELADO");
      } else {
        closeCrate(ctx);
        dropMode = false;
        inventoryOpen = false;
        setInventoryMessage("");
      }
    }

    if (!inventoryOpen) {
      if (!openedThisFrame && !g_survivalPadOwned) quickEquip(pad);
      return;
    }
    syncCursor();

    if (inventoryGrab.active) {
      updateGrab(pad);
      loadTimer_ = 0.0F;
      return;
    }
    if (crateActive >= 0) {
      updateCrate(pad);
      loadTimer_ = 0.0F;
      return;
    }

    if (inputClicked(pad, IA_CONFIRM) && !dropMode) useSelectedItem();
    if (!dropMode) {
      if (pad.getClicked().L2) beginGrab();
      if (pad.getClicked().R2 &&
          !playerInventory.rotateItem(playerInventorySelection))
        setInventoryMessage("NAO GIRA AQUI");
    }

    // Circle drops. A stack of more than one first asks how many: Left/Right
    // choose the amount, Circle confirms and Triangle cancels.
    const Item* current = playerInventory.slot(playerInventorySelection);
    const int stack = current ? current->quantity : 0;
    if (dropMode) {
      if (stack < 1) {
        dropMode = false;
      } else {
        if (inputClicked(pad, IA_ROLE_MENU_LEFT))
          dropQuantity = dropQuantity > 1 ? dropQuantity - 1 : stack;
        if (inputClicked(pad, IA_ROLE_MENU_RIGHT))
          dropQuantity = dropQuantity < stack ? dropQuantity + 1 : 1;
        if (dropQuantity > stack) dropQuantity = stack;
        if (dropQuantity < 1) dropQuantity = 1;
        if (inputClicked(pad, IA_ROLE_ALT)) {
          dropMode = false;
          dropSelectedItem(ctx);
        }
      }
    } else {
      navigate(pad);
      if (!openedThisFrame && inputClicked(pad, IA_ROLE_ALT)) {
        dropQuantity = 1;
        if (stack > 1 && !Inventory::isEquipmentEntry(playerInventorySelection))
          dropMode = true;
        else
          dropSelectedItem(ctx);
      }
    }
    if (pad.getPressed().Square && !dropMode)
      loadSelectedMagazine(g_frameDt);
    else
      loadTimer_ = 0.0F;
  }

 private:
  // D-pad shortcuts outside the inventory. Down draws or stows the secondary
  // weapon from the holster; the back-weapon (Up/Right) and melee (Left)
  // slots have no items yet.
  // Puts the carried pistol into the holster slot when a holster is worn and
  // the slot is free, so the D-pad can draw it later.
  static bool assignPistolToHolster() {
    if (!playerInventory.holsterEquipped() ||
        playerInventory.equipped(EquipSlot::WeaponSecondary))
      return false;
    for (int i = 0; i < kInventoryRecordCount; ++i) {
      const Item* item = playerInventory.slot(i);
      if (item && item->id == kItemPistol)
        return playerInventory.equip(i) == EquipResult::Ok;
    }
    return false;
  }

  void quickEquip(Tyra::Pad& pad) {
    if (!inputClicked(pad, IA_ROLE_MENU_DOWN)) return;
    Weapon& weapon = survivalWeapon();
    if (weapon.equipped()) {
      weapon.holster();
      setInventoryMessage(assignPistolToHolster() ? "PISTOLA NO COLDRE"
                                                  : "PISTOLA GUARDADA");
      return;
    }
    if (!playerInventory.holsterEquipped()) {
      setInventoryMessage("SEM COLDRE EQUIPADO");
      return;
    }
    if (weapon.owned()) assignPistolToHolster();
    const Item* secondary = playerInventory.equipped(EquipSlot::WeaponSecondary);
    if (!weapon.owned() || !secondary || secondary->id != kItemPistol) {
      setInventoryMessage("PISTOLA FORA DO COLDRE");
      return;
    }
    weapon.draw();
    setInventoryMessage("PISTOLA EQUIPADA");
  }

  // The weapon state follows the pistol item: in a crate the player does not
  // own it; taken back it is owned and holstered.
  static void pistolChangedHands(bool toCrate) {
    Weapon& weapon = survivalWeapon();
    if (toCrate) {
      weapon.release();
    } else {
      weapon.acquire();
      weapon.holster();
    }
  }

  // The inventory a grab or cursor refers to.
  static Inventory& grabSource() {
    return inventoryGrab.fromCrate ? crates[crateActive] : playerInventory;
  }
  static Inventory& grabTarget() {
    return inventoryGrab.container == 2 ? crates[crateActive] : playerInventory;
  }
  // Container id inside the target inventory (the crate is its own inventory,
  // whose only grid is container 0).
  static int grabTargetContainer() {
    return inventoryGrab.container == 2 ? 0 : inventoryGrab.container;
  }

  void beginGrab() {
    const bool inCrate = crateActive >= 0 && crateFocus;
    Inventory& source = inCrate ? crates[crateActive] : playerInventory;
    const int index = inCrate ? crateSelection : playerInventorySelection;
    if (Inventory::isEquipmentEntry(index)) {
      setInventoryMessage("DESEQUIPE PARA MOVER");
      return;
    }
    const Item* item = source.slot(index);
    if (!item || item->id == 0) {
      setInventoryMessage("SLOT VAZIO");
      return;
    }
    inventoryGrab.active = true;
    inventoryGrab.fromCrate = inCrate;
    inventoryGrab.record = index;
    inventoryGrab.container = inCrate ? 2 : item->container;
    inventoryGrab.x = item->gridX;
    inventoryGrab.y = item->gridY;
    inventoryGrab.rotated = item->rotated;
    setInventoryMessage("");
  }

  static void clampGrab(const Item& item) {
    const int container = grabTargetContainer();
    const int w = inventoryGrab.rotated ? item.height : item.width;
    const int h = inventoryGrab.rotated ? item.width : item.height;
    const int maxX = Inventory::containerWidth(container) - w;
    const int maxY = Inventory::containerHeight(container) - h;
    if (inventoryGrab.x > maxX) inventoryGrab.x = maxX;
    if (inventoryGrab.y > maxY) inventoryGrab.y = maxY;
    if (inventoryGrab.x < 0) inventoryGrab.x = 0;
    if (inventoryGrab.y < 0) inventoryGrab.y = 0;
  }

  // Grab mode: D-pad moves the item by one cell (crossing between pockets,
  // backpack and the crate at the edges), R2 rotates, X places it, Triangle
  // cancels.
  void updateGrab(Tyra::Pad& pad) {
    InventoryGrab& grab = inventoryGrab;
    if (grab.fromCrate && crateActive < 0) {
      grab.active = false;
      return;
    }
    Inventory& source = grabSource();
    const Item* item = source.slot(grab.record);
    if (!item || item->id == 0 || Inventory::isEquipmentEntry(grab.record)) {
      grab.active = false;
      return;
    }
    if (grab.container == 2 && crateActive < 0) grab.container = 0;
    if (grab.container == 1 && !playerInventory.containerActive(1))
      grab.container = 0;
    const int w = grab.rotated ? item->height : item->width;
    const int h = grab.rotated ? item->width : item->height;
    const int container = grabTargetContainer();

    if (inputClicked(pad, IA_ROLE_MENU_LEFT)) {
      if (grab.container == 2 && grab.x == 0) {
        grab.container = 0;
        grab.x = Inventory::containerWidth(0);
      }
      --grab.x;
    }
    if (inputClicked(pad, IA_ROLE_MENU_RIGHT)) {
      if (crateActive >= 0 && grab.container != 2 &&
          grab.x + w >= Inventory::containerWidth(container)) {
        grab.container = 2;
        grab.x = 0;
      } else {
        ++grab.x;
      }
    }
    if (inputClicked(pad, IA_ROLE_MENU_UP)) {
      if (grab.y == 0 && grab.container == 1) {
        grab.container = 0;
        grab.y = Inventory::containerHeight(0);
      }
      --grab.y;
    }
    if (inputClicked(pad, IA_ROLE_MENU_DOWN)) {
      if (grab.y + h >= Inventory::containerHeight(container) &&
          grab.container == 0 && playerInventory.containerActive(1)) {
        grab.container = 1;
        grab.y = 0;
      } else {
        ++grab.y;
      }
    }
    if (pad.getClicked().R2) grab.rotated = !grab.rotated;
    clampGrab(*item);

    if (!inputClicked(pad, IA_CONFIRM)) return;
    Inventory& target = grabTarget();
    const int targetContainer = grabTargetContainer();
    bool placed = false;
    if (&target == &source) {
      placed = source.moveItem(grab.record, targetContainer, grab.x, grab.y,
                               grab.rotated);
    } else if (item->id == kItemPistol && !grab.fromCrate &&
               survivalWeapon().equipped()) {
      setInventoryMessage("GUARDE A PISTOLA ANTES (SETA BAIXO)");
      return;
    } else {
      const Item copy = *item;
      placed = target.placeAt(copy, targetContainer, grab.x, grab.y,
                              grab.rotated);
      if (placed) {
        source.removeFromSlot(grab.record, copy.quantity);
        if (copy.id == kItemPistol) pistolChangedHands(!grab.fromCrate);
      }
    }
    if (placed) {
      grab.active = false;
      cursorPanel = grab.container;
      cursorX = grab.x;
      cursorY = grab.y;
      syncCursor();
      setInventoryMessage("");
    } else {
      setInventoryMessage("NAO CABE AQUI");
    }
  }

  // Item whose footprint covers a grid cell, or -1 for an empty cell.
  static int cellItem(const Inventory& inventory, int container, int x,
                      int y) {
    for (int i = 0; i < kInventoryRecordCount; ++i) {
      const Item* item = inventory.slot(i);
      if (!item || item->id == 0 || item->container != container) continue;
      if (x >= item->gridX && x < item->gridX + item->footprintW() &&
          y >= item->gridY && y < item->gridY + item->footprintH())
        return i;
    }
    return -1;
  }

  static int clampInt(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
  }

  // Keeps the cell cursor valid and derives the selected entry from it. The
  // selection is -1 on an empty cell; equipment slots are always selectable.
  static void syncCursor() {
    if ((cursorPanel == 2 && crateActive < 0) ||
        (cursorPanel == 1 && !playerInventory.containerActive(1))) {
      cursorPanel = 0;
      cursorX = cursorY = 0;
    }
    if (cursorPanel == 3) {
      cursorY = clampInt(cursorY, 0, kEquipSlotCount - 1);
      playerInventorySelection = kInventoryRecordCount + cursorY;
    } else {
      const int container = cursorPanel == 1 ? 1 : 0;
      cursorX = clampInt(cursorX, 0, Inventory::containerWidth(container) - 1);
      cursorY = clampInt(cursorY, 0, Inventory::containerHeight(container) - 1);
      if (cursorPanel == 2 && crateActive >= 0)
        crateSelection = cellItem(crates[crateActive], 0, cursorX, cursorY);
      else
        playerInventorySelection =
            cellItem(playerInventory, container, cursorX, cursorY);
    }
    crateFocus = cursorPanel == 2;
  }

  // Walks the cursor one cell at a time. At a border it crosses to the
  // neighbouring panel: equipment | pockets (backpack below) | crate.
  static void navigate(Tyra::Pad& pad) {
    int dx = 0, dy = 0;
    if (inputClicked(pad, IA_ROLE_MENU_LEFT)) dx = -1;
    if (inputClicked(pad, IA_ROLE_MENU_RIGHT)) dx = 1;
    if (inputClicked(pad, IA_ROLE_MENU_UP)) dy = -1;
    if (inputClicked(pad, IA_ROLE_MENU_DOWN)) dy = 1;
    if (dx == 0 && dy == 0) return;

    constexpr int kCell = 28;
    const int pocketsW = Inventory::containerWidth(0);
    const int pocketsH = Inventory::containerHeight(0);
    const int backpackW = Inventory::containerWidth(1);
    const int backpackH = Inventory::containerHeight(1);
    const bool backpack = playerInventory.containerActive(1);
    const int backpackTop = pocketsH * kCell + 20;
    // Equipment boxes are 40 units tall; map a screen height to its row.
    auto equipRow = [](int screenY) { return clampInt(screenY / 40, 0, 5); };

    switch (cursorPanel) {
      case 3: {
        const int row = cursorY / 2;
        const int col = cursorY % 2;
        if (dy != 0) {
          cursorY = clampInt(row + dy, 0, 5) * 2 + col;
        } else if (dx < 0 && col == 1) {
          --cursorY;
        } else if (dx > 0 && col == 0) {
          ++cursorY;
        } else if (dx > 0) {
          const int screenY = row * 40 + 18;
          if (screenY < backpackTop || !backpack) {
            cursorPanel = 0;
            cursorX = 0;
            cursorY = clampInt(screenY / kCell, 0, pocketsH - 1);
          } else {
            cursorPanel = 1;
            cursorX = 0;
            cursorY = clampInt((screenY - backpackTop) / kCell, 0,
                               backpackH - 1);
          }
        }
        break;
      }
      case 0:
        if (dx < 0) {
          if (cursorX == 0) {
            cursorY = equipRow(cursorY * kCell + kCell / 2) * 2 + 1;
            cursorPanel = 3;
          } else {
            --cursorX;
          }
        } else if (dx > 0) {
          if (cursorX < pocketsW - 1) {
            ++cursorX;
          } else if (crateActive >= 0) {
            cursorPanel = 2;
            cursorX = 0;
          }
        } else if (dy < 0) {
          if (cursorY > 0) --cursorY;
        } else if (cursorY < pocketsH - 1) {
          ++cursorY;
        } else if (backpack) {
          cursorPanel = 1;
          cursorY = 0;
        }
        break;
      case 1:
        if (dx < 0) {
          if (cursorX == 0) {
            cursorY = equipRow(backpackTop + cursorY * kCell + kCell / 2) * 2 + 1;
            cursorPanel = 3;
          } else {
            --cursorX;
          }
        } else if (dx > 0) {
          if (cursorX < backpackW - 1) ++cursorX;
        } else if (dy < 0) {
          if (cursorY > 0) {
            --cursorY;
          } else {
            cursorPanel = 0;
            cursorY = pocketsH - 1;
          }
        } else if (cursorY < backpackH - 1) {
          ++cursorY;
        }
        break;
      case 2:
        if (dx < 0) {
          if (cursorX == 0) {
            cursorPanel = 0;
            cursorX = pocketsW - 1;
          } else {
            --cursorX;
          }
        } else if (dx > 0) {
          if (cursorX < pocketsW - 1) ++cursorX;
        } else if (dy < 0) {
          if (cursorY > 0) --cursorY;
        } else if (cursorY < pocketsH - 1) {
          ++cursorY;
        }
        break;
    }
    syncCursor();
  }

  // Cursor and buttons while a loot crate is open. The D-pad walks across both
  // panels; X sends the selected item across.
  void updateCrate(Tyra::Pad& pad) {
    Inventory& crate = crates[crateActive];
    Inventory& focused = crateFocus ? crate : playerInventory;

    navigate(pad);
    if (inputClicked(pad, IA_CONFIRM)) quickTransfer();
    if (pad.getClicked().L2) beginGrab();
    if (pad.getClicked().R2) {
      const int current = crateFocus ? crateSelection : playerInventorySelection;
      if (Inventory::isEquipmentEntry(current) ||
          !focused.rotateItem(current))
        setInventoryMessage("NAO GIRA AQUI");
    }
  }

  void quickTransfer() {
    Inventory& crate = crates[crateActive];
    Inventory& from = crateFocus ? crate : playerInventory;
    Inventory& to = crateFocus ? playerInventory : crate;
    const int index = crateFocus ? crateSelection : playerInventorySelection;
    if (Inventory::isEquipmentEntry(index)) {
      setInventoryMessage("DESEQUIPE PARA TRANSFERIR");
      return;
    }
    const Item* item = from.slot(index);
    if (!item || item->id == 0) {
      setInventoryMessage(crateFocus ? "CAIXA VAZIA" : "SLOT VAZIO");
      return;
    }
    if (item->id == kItemPistol && !crateFocus && survivalWeapon().equipped()) {
      setInventoryMessage("GUARDE A PISTOLA ANTES (SETA BAIXO)");
      return;
    }
    const Item copy = *item;
    if (to.place(copy) < 0) {
      setInventoryMessage(crateFocus ? "SEM ESPACO NO INVENTARIO"
                                     : "CAIXA CHEIA");
      return;
    }
    from.removeFromSlot(index, copy.quantity);
    if (copy.id == kItemPistol) pistolChangedHands(!crateFocus);
    snprintf(inventoryMessage, sizeof(inventoryMessage), "%s: %s x%d",
             crateFocus ? "PEGOU" : "GUARDOU", copy.name ? copy.name : "ITEM",
             copy.quantity);
    inventoryMessageTimer = 2.5F;
  }

  void closeCrate(ScriptContext& ctx) {
    if (crateActive < 0) return;
    const int open = crateActive;
    crateActive = -1;
    syncCursor();
    inventoryGrab.active = false;
    inventoryGrab.fromCrate = false;
    if (crateDimmed[open] || crateObject < 0 || crateObject >= ctx.objectCount)
      return;
    bool empty = true;
    for (int i = 0; i < kInventoryRecordCount; ++i)
      if (crates[open].slot(i)->id != 0) empty = false;
    if (!empty) return;
    RuntimeObject& object = ctx.objects[crateObject];
    object.data.color[0] *= 0.82F;
    object.data.color[1] *= 0.82F;
    object.data.color[2] *= 0.82F;
    object.dirty = true;
    crateDimmed[open] = true;
  }

  void collectPickup(ScriptContext& ctx) {
    if (ctx.usedObject < 0 || ctx.usedObject >= ctx.objectCount) return;
    if (ctx.scene < 0 || ctx.scene >= SCENE_COUNT ||
        ctx.scene >= static_cast<int>(sizeof(SCENE_OBJECT_ID_TABLES) /
                                     sizeof(SCENE_OBJECT_ID_TABLES[0])) ||
        ctx.usedObject >= SCENE_OBJECT_COUNTS[ctx.scene])
      return;
    const unsigned long long* ids = SCENE_OBJECT_ID_TABLES[ctx.scene];
    if (!ids) return;

    if (ids[ctx.usedObject] == kSupplyCrateAIdHash) {
      collectContainerLoot(ctx, 0, kSupplyCrateAIdHash);
      return;
    }
    if (ids[ctx.usedObject] == kSupplyCrateBIdHash) {
      collectContainerLoot(ctx, 1, kSupplyCrateBIdHash);
      return;
    }

    const PickupDefinition* pickup = nullptr;
    for (const PickupDefinition& definition : kPickups) {
      if (definition.idHash != 0 && ids[ctx.usedObject] == definition.idHash) {
        pickup = &definition;
        break;
      }
    }
    if (!pickup || !ctx.objects[ctx.usedObject].active ||
        !ctx.objects[ctx.usedObject].visible)
      return;

    const ItemDefinition* definition = itemDefinition(pickup->itemId);
    if (!definition) {
      TYRA_ERROR("Inventory: missing definition for item ", pickup->itemId);
      return;
    }

    const int droppedIndex = findDroppedItem(ctx.usedObject);
    const int quantity =
        droppedIndex >= 0 ? droppedItems[droppedIndex].quantity
                          : pickup->quantity;
    const int rounds =
        droppedIndex >= 0 ? droppedItems[droppedIndex].ammoCount : 0;

    const Inventory before = playerInventory;
    int added = 0;
    if (pickup->magazine) {
      added = playerInventory.addMagazine(rounds) ? 1 : 0;
    } else {
      added = playerInventory.add(*definition, quantity);
    }
    if (added != quantity) {
      playerInventory = before;
      setInventoryMessage("SEM ESPACO NO INVENTARIO");
      TYRA_LOG("Inventory: no free slot for ", definition->name);
      return;
    }

    ctx.objects[ctx.usedObject].visible = false;
    if (droppedIndex >= 0)
      droppedItems[droppedIndex] = droppedItems[--droppedItemCount];
    if (pickup->itemId == kItemPistol) survivalWeapon().acquire();
    snprintf(inventoryMessage, sizeof(inventoryMessage), "PEGOU: %s x%d",
             definition->name, quantity);
    inventoryMessageTimer = 2.5F;
    TYRA_LOG("Inventory: picked up ", definition->name, " x", quantity);
  }

  void dropSelectedItem(ScriptContext& ctx) {
    const Item* selected = playerInventory.slot(playerInventorySelection);
    if (!selected || selected->id == 0) {
      setInventoryMessage("SLOT VAZIO");
      return;
    }
    const int itemId = selected->id;
    if (Inventory::isEquipmentEntry(playerInventorySelection)) {
      setInventoryMessage("DESEQUIPE ANTES DE DROPAR");
      return;
    }
    if (itemId == kItemPistol && survivalWeapon().equipped()) {
      setInventoryMessage("GUARDE A PISTOLA ANTES DE DROPAR");
      return;
    }
    if (ctx.scene < 0 || ctx.scene >= SCENE_COUNT) {
      setInventoryMessage("SEM ESPACO NO CHAO");
      return;
    }
    const unsigned long long* ids = SCENE_OBJECT_ID_TABLES[ctx.scene];
    const int objectCount = ctx.objectCount < SCENE_OBJECT_COUNTS[ctx.scene]
                                ? ctx.objectCount
                                : SCENE_OBJECT_COUNTS[ctx.scene];
    int target = -1;
    // An item of the same type already dropped on the floor takes the new
    // amount (the pickup object is shared), wherever it lies.
    int mergeIndex = -1;
    for (int k = 0; ids && k < droppedItemCount && mergeIndex < 0; ++k) {
      const int object = droppedItems[k].object;
      if (object < 0 || object >= objectCount || !ctx.objects[object].visible)
        continue;
      for (const PickupDefinition& definition : kPickups) {
        if (definition.itemId == itemId && !definition.magazine &&
            definition.idHash != 0 && ids[object] == definition.idHash) {
          mergeIndex = k;
          target = object;
          break;
        }
      }
    }
    if (mergeIndex < 0 && droppedItemCount >= kMaxDroppedItems) {
      setInventoryMessage("SEM ESPACO NO CHAO");
      return;
    }
    for (int i = 0; ids && i < objectCount && target < 0; ++i) {
      if (ctx.objects[i].visible) continue;
      for (const PickupDefinition& definition : kPickups) {
        if (definition.itemId == itemId && definition.idHash != 0 &&
            ids[i] == definition.idHash) {
          target = i;
          break;
        }
      }
    }
    if (target < 0) {
      bool hasWorldObject = false;
      for (const PickupDefinition& definition : kPickups)
        if (definition.itemId == itemId) hasWorldObject = true;
      setInventoryMessage(hasWorldObject ? "SEM ESPACO NO CHAO"
                                         : "ESSE ITEM NAO PODE SER DROPADO");
      return;
    }

    const int amount =
        dropQuantity < selected->quantity ? dropQuantity : selected->quantity;
    const DroppedItem dropped = {target, amount, selected->ammoCount};
    const char* name = selected->name;
    if (!playerInventory.removeFromSlot(playerInventorySelection,
                                        dropped.quantity))
      return;
    if (itemId == kItemPistol) survivalWeapon().release();
    dropQuantity = 1;
    if (mergeIndex >= 0)
      droppedItems[mergeIndex].quantity += dropped.quantity;
    else
      droppedItems[droppedItemCount++] = dropped;

    float dirX = ctx.playerLook.x, dirZ = ctx.playerLook.z;
    const float length = sqrtf(dirX * dirX + dirZ * dirZ);
    if (length > 0.001F) {
      dirX /= length;
      dirZ /= length;
    } else {
      dirX = 0.0F;
      dirZ = 1.0F;
    }
    RuntimeObject& object = ctx.objects[target];
    object.data.position[0] = ctx.playerPosition.x + dirX * 0.9F;
    object.data.position[1] = ctx.playerPosition.y - EYE_HEIGHT + 0.25F;
    object.data.position[2] = ctx.playerPosition.z + dirZ * 0.9F;
    object.velocityX = object.velocityY = object.velocityZ = 0.0F;
    object.restFrames = 0;
    object.visible = true;
    object.dirty = true;
    snprintf(inventoryMessage, sizeof(inventoryMessage), "DROPOU: %s x%d",
             name ? name : "ITEM", dropped.quantity);
    inventoryMessageTimer = 2.5F;
  }

  // Using a crate opens its own grid next to the player's inventory. The
  // contents are rolled once per scene and stay until taken.
  void collectContainerLoot(ScriptContext& ctx, int container,
                            unsigned long long idHash) {
    RuntimeObject& object = ctx.objects[ctx.usedObject];
    if (!object.active || !object.visible) return;

    if (!crateRolled[container]) {
      crateRolled[container] = true;
      crates[container].reset();
      for (int i = 0; i < 3; ++i) {
        const LootDrop drop = rollContainerLoot(idHash, ctx.scene, i);
        const ItemDefinition* definition = itemDefinition(drop.itemId);
        if (!definition) {
          TYRA_ERROR("Inventory: missing container loot definition ",
                     drop.itemId);
          continue;
        }
        if (drop.magazine)
          crates[container].addMagazine(0);
        else
          crates[container].add(*definition, drop.quantity);
      }
    }

    crateActive = container;
    crateObject = ctx.usedObject;
    crateFocus = true;
    cursorPanel = 2;
    cursorX = cursorY = 0;
    syncCursor();
    inventoryGrab.active = false;
    inventoryOpen = true;
    setInventoryMessage("");
  }

  void loadSelectedMagazine(float deltaTime) {
    const Item* selected = playerInventory.slot(playerInventorySelection);
    if (!selected || selected->id != kItemPistolMagazine) {
      setInventoryMessage("SELECIONE UM CARREGADOR");
      loadTimer_ = 0.0F;
      return;
    }

    loadTimer_ -= deltaTime;
    if (loadTimer_ > 0.0F) return;
    loadTimer_ = 0.16F;
    if (selected->ammoCount >= selected->ammoCapacity) {
      setInventoryMessage("CARREGADOR CHEIO");
      loadTimer_ = 0.0F;
      return;
    }
    const int ammoSlot = playerInventory.firstAmmoSlot(selected->ammoType);
    if (ammoSlot < 0) {
      setInventoryMessage("SEM MUNICAO COMPATIVEL");
      return;
    }
    if (!playerInventory.loadMagazine(playerInventorySelection, ammoSlot)) {
      setInventoryMessage("CARREGADOR CHEIO");
      loadTimer_ = 0.0F;
      return;
    }
    const Item* magazine = playerInventory.slot(playerInventorySelection);
    snprintf(inventoryMessage, sizeof(inventoryMessage), "CARREGADOR: %d/%d",
             magazine ? magazine->ammoCount : 0,
             magazine ? magazine->ammoCapacity : 0);
    inventoryMessageTimer = 2.5F;
  }

  void useSelectedItem() {
    const Item* selected = playerInventory.slot(playerInventorySelection);
    if (!selected || selected->id == 0) return;

    const int itemId = selected->id;
    const char* itemName = selected->name;
    Player& player = survivalPlayer();
    bool equipmentToggled = false;
    bool used = false;
    if (itemId == kItemPistol) {
      Weapon& weapon = survivalWeapon();
      used = true;
      if (Inventory::isEquipmentEntry(playerInventorySelection)) {
        if (weapon.equipped()) {
          setInventoryMessage("GUARDE A PISTOLA (SETA BAIXO)");
        } else {
          playerInventory.toggleEquipment(playerInventorySelection);
          setInventoryMessage("PISTOLA FORA DO COLDRE");
        }
      } else if (playerInventory.holsterEquipped() &&
                 playerInventory.equipped(EquipSlot::WeaponSecondary) ==
                     nullptr) {
        weapon.holster();
        playerInventory.toggleEquipment(playerInventorySelection);
        setInventoryMessage("PISTOLA NO COLDRE");
      } else {
        weapon.toggleEquipped();
        setInventoryMessage(weapon.equipped() ? "PISTOLA EQUIPADA"
                                              : "PISTOLA GUARDADA");
      }
    } else if (itemId == kItemBandage) {
      used = player.heal(kBandageHealing) || player.bleeding();
      if (used) {
        player.stopBleeding();
        setInventoryMessage("BANDAGEM APLICADA");
      }
    } else if (itemId == kItemMedkit) {
      used = player.heal(kMedkitHealing) || player.bleeding();
      if (used) {
        player.stopBleeding();
        setInventoryMessage("KIT MEDICO USADO");
      }
    } else if (itemId == kItemCannedFood) {
      used = player.hunger() < 100.0F;
      if (used) {
        player.restoreHunger(kFoodRestore);
        setInventoryMessage("COMIDA CONSUMIDA");
      }
    } else if (itemId == kItemWater) {
      used = player.thirst() < 100.0F;
      if (used) {
        player.restoreThirst(kWaterRestore);
        setInventoryMessage("AGUA CONSUMIDA");
      }
    } else if (Inventory::isEquipmentEntry(playerInventorySelection) ||
               selected->slot != EquipSlot::None) {
      used = true;
      equipmentToggled = true;
      switch (playerInventory.toggleEquipment(playerInventorySelection)) {
        case EquipResult::Ok:
          setInventoryMessage("EQUIPAMENTO ALTERADO");
          break;
        case EquipResult::SlotOccupied:
          setInventoryMessage("SLOT JA OCUPADO");
          break;
        case EquipResult::NoSpace:
          setInventoryMessage("SEM ESPACO NO INVENTARIO");
          break;
        case EquipResult::BackpackNotEmpty:
          setInventoryMessage("ESVAZIE A MOCHILA ANTES DE REMOVER");
          break;
        case EquipResult::HolsterNotEmpty:
          setInventoryMessage("TIRE A PISTOLA DO COLDRE ANTES");
          break;
        case EquipResult::NeedsHolster:
          setInventoryMessage("PRECISA DO COLDRE");
          break;
        default:
          setInventoryMessage("ITEM NAO PODE SER EQUIPADO");
          break;
      }
    }

    if (used && !equipmentToggled && itemId != kItemPistol) {
      if (!playerInventory.removeFromSlot(playerInventorySelection, 1)) {
        TYRA_ERROR("Inventory: failed to consume ", itemName);
        return;
      }
      TYRA_LOG("Inventory: used ", itemName);
    } else if (!used) {
      setInventoryMessage("ITEM NAO PODE SER USADO AGORA");
    }
  }

  unsigned int sceneGeneration_ = 0;
  float loadTimer_ = 0.0F;
  bool initialized_ = false;
};

}  // namespace

Inventory& survivalInventory() { return playerInventory; }

bool g_survivalPadOwned = false;

int& survivalInventorySelection() { return playerInventorySelection; }

const InventoryGrab& survivalInventoryGrab() { return inventoryGrab; }

bool survivalInventoryOpen() { return inventoryOpen; }

const Inventory* survivalCrate() {
  return crateActive >= 0 ? &crates[crateActive] : nullptr;
}

int survivalCursorPanel() { return cursorPanel; }

int survivalCursorX() { return cursorX; }

int survivalCursorY() { return cursorY; }

bool survivalCrateFocused() { return crateFocus; }

int survivalCrateSelection() { return crateSelection; }

int survivalInventoryDropQuantity() { return dropQuantity; }

bool survivalInventoryDropMode() { return dropMode; }

const char* survivalInventoryMessage() { return inventoryMessage; }

bool survivalInventoryToastVisible() {
  return !inventoryOpen && inventoryMessageTimer > 0.0F &&
         inventoryMessage[0] != '\0';
}

}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalInventoryScript);
