#ifndef ENTITY_H
#define ENTITY_H
#include "base/base_inc.h"

typedef u32 EntityID;

typedef struct Entity Entity;
struct Entity {
  EntityID id;
  String8 name;
  struct {
    Vec3f32 translation; // Translation
    Quaternion rotation; // Rotation
    Vec3f32 scale;       // Scale
  } transform;
  b32 alive;
};

typedef struct EntityStore EntityStore;
struct EntityStore {
  Arena* arena;

  Entity** list;
  usize list_count;
  usize list_capacity;

  EntityID* id_to_index;
  usize id_map_capacity;

  Entity** free_list;
  usize free_count;
  usize free_capacity;

  EntityID next_id;
};

#define entity_foreach(store, ent)                                       \
  for (usize _i = 0;                                                           \
       _i < (store)->list_count && ((ent = (store)->list[_i]) || true); _i++)

void    entity_store_init(Arena* arena, EntityStore* store, usize list_capacity, usize id_map_capacity);
Entity* entity_store_add(EntityStore* store);
Entity* entity_store_find(EntityStore* store, EntityID id);
void    entity_store_remove(EntityStore* store, EntityID id);
void    entity_store_compact(EntityStore* store);

#endif
