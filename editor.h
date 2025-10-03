#ifndef UI_H
#define UI_H
#include "core.h"
#include "entity.h"
#include "thirdparty/raylib-nuklear.h"
#include <dirent.h>

#define MAX_ASSETS 256
#define MAX_UI_WINDOWS 32

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
  String8 path;
  AssetType type;
  Texture2D preview;
  b32 has_preview;
  time_t modified_time;
  Model* model;
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
  struct nk_rect bounds;
  b32 active;
  String8 name;
} UIWindow;

typedef struct {
  UIWindow windows[MAX_UI_WINDOWS];
  u64 count;
} UIManager;

typedef struct {
  Vec2 position;
  Texture texture;
} Cursor;

typedef struct {
  Arena* arena;
  struct nk_context* nk_ctx;
  Cursor cursor;
  UIVisibility* vis;
  UIManager* uiman;
  AssetBrowser* browser;
  EntityStore* entity_store;
  EntityID selected_entity; 
  String8 selected_asset;
  EditorTool current_tool;
  struct {
    f32 scale;
    i32 scroll_speed;
    b32 about_to_place;
  } tool_params;
} EditorState;

void entity_array_push(Arena* arena, EntityArray* arr, Entity entity);

void assetbrowser_load(Arena* arena, AssetBrowser* browser, String8 path);
void assetbrowser_unload(AssetBrowser* browser);

void ui_manager_register_window(UIManager* man, struct nk_context* ctx, String8 name);
b32  ui_is_hovered(UIManager* man, struct nk_context* ctx);

void ui_assetbrowser(EditorState* ed);
void ui_toolbar(EditorState* ed);
void ui_hierarchy(EditorState* ed);
void ui_inspector(EditorState* ed);
void ui_update_editor(EditorState* ed);
b32  ui_any_window_hovered(struct nk_context* ctx);
b32  ui_is_capturing_input(struct nk_context* ctx);

#endif
