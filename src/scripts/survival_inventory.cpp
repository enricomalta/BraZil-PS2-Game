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
bool inventoryOpen = false;
char inventoryMessage[64] = "INVENTARIO FECHADO";
float inventoryMessageTimer = 0.0F;
int containerLootTaken[2] = {0, 0};
int dropQuantity = 1;
int dropQuantitySlot = -1;
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
  switch ((state >> 16) % 8U) {
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
      inventoryOpen = false;
      loadTimer_ = 0.0F;
      containerLootTaken[0] = 0;
      containerLootTaken[1] = 0;
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
      inventoryOpen = false;
      setInventoryMessage("");
    }

    if (!inventoryOpen) return;
    const int availableSlots = playerInventory.slotCount();
    if (playerInventorySelection >= availableSlots)
      playerInventorySelection = availableSlots - 1;
    if (playerInventorySelection < 0) playerInventorySelection = 0;

    if (inputClicked(pad, IA_ROLE_MENU_UP))
      playerInventorySelection =
          playerInventorySelection > 0 ? playerInventorySelection - 1
                                       : availableSlots - 1;
    if (inputClicked(pad, IA_ROLE_MENU_DOWN))
      playerInventorySelection =
          (playerInventorySelection + 1) % availableSlots;

    if (inputClicked(pad, IA_CONFIRM)) useSelectedItem();

    // Left/right choose how many of the selected stack a drop will take.
    const Item* current = playerInventory.slot(playerInventorySelection);
    const int stack = current ? current->quantity : 0;
    if (dropQuantitySlot != playerInventorySelection) {
      dropQuantitySlot = playerInventorySelection;
      dropQuantity = 1;
    }
    if (inputClicked(pad, IA_ROLE_MENU_LEFT) && stack > 0)
      dropQuantity = dropQuantity > 1 ? dropQuantity - 1 : stack;
    if (inputClicked(pad, IA_ROLE_MENU_RIGHT) && stack > 0)
      dropQuantity = dropQuantity < stack ? dropQuantity + 1 : 1;
    if (dropQuantity > stack) dropQuantity = stack;
    if (dropQuantity < 1) dropQuantity = 1;
    // Circle (the open button) drops; it is a click, so the press that opened
    // the inventory cannot also drop.
    if (!openedThisFrame && inputClicked(pad, IA_ROLE_ALT))
      dropSelectedItem(ctx);
    if (pad.getPressed().Square)
      loadSelectedMagazine(g_frameDt);
    else
      loadTimer_ = 0.0F;
  }

 private:
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
    if (itemId == kItemPistol && survivalWeapon().equipped()) {
      setInventoryMessage("GUARDE A PISTOLA ANTES DE DROPAR");
      return;
    }
    if ((itemId == kItemBackpack && playerInventory.backpackEquipped()) ||
        (itemId == kItemVest && playerInventory.vestEquipped()) ||
        (itemId == kItemHelmet && playerInventory.helmetEquipped())) {
      setInventoryMessage("ITEM NAO PODE SER DROPADO");
      return;
    }
    if (droppedItemCount >= kMaxDroppedItems ||
        ctx.scene < 0 || ctx.scene >= SCENE_COUNT) {
      setInventoryMessage("SEM ESPACO NO CHAO");
      return;
    }
    const unsigned long long* ids = SCENE_OBJECT_ID_TABLES[ctx.scene];
    const int objectCount = ctx.objectCount < SCENE_OBJECT_COUNTS[ctx.scene]
                                ? ctx.objectCount
                                : SCENE_OBJECT_COUNTS[ctx.scene];
    int target = -1;
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
      setInventoryMessage("SEM ESPACO NO CHAO");
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

  void collectContainerLoot(ScriptContext& ctx, int container,
                            unsigned long long idHash) {
    RuntimeObject& object = ctx.objects[ctx.usedObject];
    if (!object.active || !object.visible) return;
    if (containerLootTaken[container] >= 3) {
      setInventoryMessage("CAIXA VAZIA");
      return;
    }

    const LootDrop drop =
        rollContainerLoot(idHash, ctx.scene, containerLootTaken[container]);
    const ItemDefinition* definition = itemDefinition(drop.itemId);
    if (!definition) {
      TYRA_ERROR("Inventory: missing container loot definition ", drop.itemId);
      return;
    }

    const Inventory before = playerInventory;
    const int added = drop.magazine
                          ? (playerInventory.addMagazine(0) ? 1 : 0)
                          : playerInventory.add(*definition, drop.quantity);
    if (added != drop.quantity) {
      playerInventory = before;
      setInventoryMessage("SEM ESPACO PARA O LOOT");
      return;
    }

    ++containerLootTaken[container];
    object.data.color[0] *= 0.82F;
    object.data.color[1] *= 0.82F;
    object.data.color[2] *= 0.82F;
    object.dirty = true;
    snprintf(inventoryMessage, sizeof(inventoryMessage), "CAIXA: %s x%d",
             definition->name, drop.quantity);
    inventoryMessageTimer = 2.5F;
    TYRA_LOG("Inventory: container ", container + 1, " yielded ",
             definition->name, " x", drop.quantity, " (",
             3 - containerLootTaken[container], " remaining)");
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
    bool used = false;
    if (itemId == kItemPistol) {
      survivalWeapon().toggleEquipped();
      used = true;
      setInventoryMessage(survivalWeapon().equipped() ? "PISTOLA EQUIPADA"
                                                     : "PISTOLA GUARDADA");
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
    } else if (itemId == kItemBackpack || itemId == kItemVest ||
               itemId == kItemHelmet) {
      const bool toggled = playerInventory.toggleEquipment(itemId);
      used = true;
      if (!toggled) {
        setInventoryMessage(itemId == kItemBackpack
                                ? "ESVAZIE SLOTS EXTRAS ANTES DE REMOVER"
                                : "EQUIPAMENTO ALTERADO");
      } else {
        setInventoryMessage("EQUIPAMENTO ALTERADO");
      }
    }

    if (used && itemId != kItemPistol &&
        itemId != kItemBackpack && itemId != kItemVest &&
        itemId != kItemHelmet) {
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

int& survivalInventorySelection() { return playerInventorySelection; }

bool survivalInventoryOpen() { return inventoryOpen; }

int survivalInventoryDropQuantity() { return dropQuantity; }

const char* survivalInventoryMessage() { return inventoryMessage; }

bool survivalInventoryToastVisible() {
  return !inventoryOpen && inventoryMessageTimer > 0.0F &&
         inventoryMessage[0] != '\0';
}

}  // namespace Deep_forest

TYRA_SCRIPT(Deep_forest::SurvivalInventoryScript);
