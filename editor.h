#ifndef UI_H
#define UI_H
#include "core.h"
#include "entity.h"
#include "thirdparty/raylib-nuklear.h"
#include <dirent.h>

#define MAX_ASSETS 256

typedef enum {
  TOOL_PLACE,
  TOOL_TRANSFORM,
  TOOL_COUNT
} EditorTool;

typedef enum {
  ASSET_TEXTURE,
  ASSET_MODEL,
  ASSET_OTHER,
} AssetType;

typedef struct {
  String8 name;
  AssetType type;
  Texture2D preview;
  b32 has_preview;
  time_t modified_time;
} AssetEntry;

typedef struct {
  AssetEntry entries[MAX_ASSETS];
  usize count;
  isize selected;
} AssetBrowser;

typedef struct {
  Entity* v;
  u64 count;
  u64 capacity;
} EntityArray;

typedef struct {
  b32 show_toolbar;
  b32 show_hierarchy;
  b32 show_inspector;
  b32 show_assetbrowser;
} UIVisibility;

typedef struct {
  Arena* arena;
  HashTable* ht;
  struct nk_context* nk_ctx;
  UIVisibility* vis;
  AssetBrowser* browser;
  EntityStore* entity_store;
  EntityID selected_entity; 
  EditorTool current_tool;
} EditorState;

void entity_array_push(Arena* arena, EntityArray* arr, Entity entity);

void assetbrowser_load(Arena* arena, AssetBrowser* browser, String8 path);
void assetbrowser_unload(AssetBrowser* browser);

void ui_assetbrowser(EditorState* ed);
void ui_toolbar(EditorState* ed);
void ui_hierarchy(EditorState* ed);
void ui_inspector(EditorState* ed);
void ui_update_editor(EditorState* ed);
#endif
