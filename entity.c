#include "entity.h"
#include "base/base_arena.h"

void entity_store_init(Arena* arena, EntityStore* store, usize list_capacity, usize id_map_capacity) {
  store->arena = arena;
  store->next_id = 1;

  store->list_capacity = list_capacity;
  store->list_count = 0;
  store->list = push_array(arena, Entity*, list_capacity);

  store->id_map_capacity = id_map_capacity;
  store->id_to_index = push_array(arena, EntityID, id_map_capacity);
  MemorySet(store->id_to_index, 0, id_map_capacity);

  store->free_capacity = list_capacity / 4 + 16;
  store->free_count = 0;
  store->free_list = push_array(arena, Entity*, store->free_capacity);
}

Entity* entity_store_add(EntityStore* store) {
  Entity* ent;
  // optionally reuse
  if (store->free_count > 0) {
    ent = store->free_list[--store->free_count];
  } else {
    ent = push_one(store->arena, Entity);

    // optionally grow
    if (store->list_count >= store->list_capacity) {
      usize new_cap = store->list_capacity * 2;
      Entity** new_list = grow_array(store->arena, Entity*, store->list, store->list_count, new_cap);
      store->list = new_list;
      store->list_capacity = new_cap;
    }
    store->list[store->list_count] = ent;
  }

  ent->id = store->next_id++;
  ent->alive = true;

  // update id_to_index map
  if (ent->id >= store->id_map_capacity) {
    usize new_cap = store->id_map_capacity * 2;
    if (new_cap <= ent->id) new_cap = ent->id + 16;
    EntityID* new_map = grow_array(store->arena, EntityID, store->id_to_index, store->id_map_capacity, new_cap);
    /* for (usize i = 0; i < store->id_map_capacity; i++) new_map[i] = store->id_to_index[i]; */
    /* memset((u8 *)new_map + store->id_map_capacity * sizeof(EntityID), 0, */
    /*        (new_cap - store->id_map_capacity) * sizeof(EntityID)); */
    for (usize i = store->id_map_capacity; i < new_cap; i++) new_map[i] = 0;
    store->id_to_index = new_map;
    store->id_map_capacity = new_cap;
  }
  store->id_to_index[ent->id] = store->list_count++;
  return ent;
}

Entity* entity_store_find(EntityStore* store, EntityID id) {
  if (id == 0 || id >= store->id_map_capacity) return NULL;
  usize idx = store->id_to_index[id];
  Entity* ent =store->list[idx];
  return (ent && ent->alive) ? ent : NULL;
}

void entity_store_remove(EntityStore* store, EntityID id) {
  if (id == 0 || id >= store->id_map_capacity) return;

  usize idx = store->id_to_index[id];
  Entity* ent = store->list[idx];
  if (!ent || !ent->alive) return;

  ent->alive = false;

  // add to free list
  if (store->free_count >= store->free_capacity) {
    usize new_cap = store->free_capacity * 2;
    Entity** new_free = grow_array(store->arena, Entity*, store->free_list, store->free_count, new_cap);
    /* for (usize i = 0; i < store->free_count; i++) new_free[i] = store->free_list[i]; */
    store->free_list = new_free;
    store->free_capacity = new_cap;
  }
  store->free_list[store->free_count++] = ent;
}

void entity_store_compact(EntityStore* store) {
  usize write_idx = 0;

  for (usize read_idx = 0; read_idx < store->list_count; read_idx++) {
    Entity* ent = store->list[read_idx];
    if (!ent->alive) continue;
    if (write_idx != read_idx) {
      store->list[write_idx] = ent;
      store->id_to_index[ent->id] = write_idx;
    }
    write_idx++;
  }
  store->list_count = write_idx;
  store->list_count = 0;
}
