#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "thirdparty/raylib-nuklear.h"
#include "editor.h"

static Texture2D generate_model_thumbnail(const char* path) {
  Model model = LoadModel(path);
  if (model.meshCount == 0) {
    ERROR("failed to load model: %s", path);
    return (Texture){0};
  }

  Vec3 camera_starting_position = v3(2,2,2);
  // FIXME
  /* BoundingBox model_bbox = GetMeshBoundingBox(model.meshes[0]); */
  /* f32 model_size = glm_vec3_distance(model_bbox.max.v, model_bbox.min.v); */
  /* DEBUG("model_bbox.max: {%f, %f, %f}", */
  /*       model_bbox.max.x, */
  /*       model_bbox.max.y, */
  /*       model_bbox.max.z); */
  /* DEBUG("model_bbox.min: {%f, %f, %f}", */
  /*       model_bbox.min.x, */
  /*       model_bbox.min.y, */
  /*       model_bbox.min.z); */
  /* DEBUG("model size: %f", model_size); */

  /* // scale the model */
  /* DEBUG("scaling model"); */
  /* m4_scale(model.transform, (model_size/model_size)*2); */

  /* Vec3 size = { */
  /*   model_bbox.max.x - model_bbox.min.x, */
  /*   model_bbox.max.y - model_bbox.min.y, */
  /*   model_bbox.max.z - model_bbox.min.z, */
  /* }; */
  /* f32 max_dim = Max(size.x, Max(size.y, size.z)); */
  /* f32 scale = 2/max_dim; */

  /* for (u64 i = 0; i < model.meshes[0].vertexCount; i++) { */
  /*   model.meshes[0].vertices[i*3 + 0] *= scale; */
  /*   model.meshes[0].vertices[i*3 + 1] *= scale; */
  /*   model.meshes[0].vertices[i*3 + 2] *= scale; */
  /* } */
  /* UploadMesh(&model.meshes[0], false); */

  /* model_bbox = GetMeshBoundingBox(model.meshes[0]); */
  /* model_size = glm_vec3_distance(model_bbox.max.v, model_bbox.min.v); */
  /* DEBUG("model_bbox.max: {%f, %f, %f}", */
  /*       model_bbox.max.x, */
  /*       model_bbox.max.y, */
  /*       model_bbox.max.z); */
  /* DEBUG("model_bbox.min: {%f, %f, %f}", */
  /*       model_bbox.min.x, */
  /*       model_bbox.min.y, */
  /*       model_bbox.min.z); */
  /* DEBUG("model size: %f", model_size); */

  /* f32 distance_to_max = glm_vec3_distance(camera_starting_transform.translation.v, model_bbox.max.v); */
  /* f32 distance_to_min = glm_vec3_distance(camera_starting_transform.translation.v, model_bbox.min.v); */
  /* f32 closest_distance = Min(distance_to_max, distance_to_min); */
  /* DEBUG("distance to max: %f", distance_to_max); */
  /* DEBUG("distance to min: %f", distance_to_min); */
  /* DEBUG("closest distance: %f", closest_distance); */

  /* f32 normalized_distance = model_size/closest_distance; */
  /* DEBUG("normalized_distance: %f", normalized_distance); */

  f32 camera_x = camera_starting_position.x + 0;
  f32 camera_y = camera_starting_position.y + 2;
  f32 camera_z = camera_starting_position.z + 0;
  DEBUG("camera {%f, %f, %f}", camera_x, camera_y, camera_z);
  Camera3D cam = {
    .position   = {camera_x, camera_y, camera_z},
    .target     = {0.0, 0.5, 0.0},
    .up         = {0.0, 1.0, 0.0},
    .fovy       = 60.0,
    .projection = CAMERA_PERSPECTIVE
  };

  const u8 thumb_size = 128;
  RenderTexture2D target = LoadRenderTexture(thumb_size, thumb_size);

  DEBUG("generating model thumbnail");
  { // TEXTURE MODE =================================
    BeginTextureMode(target);

    ClearBackground((Color){30,30,30,255});
    { // 3D MODE ====================================
      BeginMode3D(cam);

      DrawModel(model, v3(0,0,0), 1.0, WHITE);

      EndMode3D();
    } // 3D MODE ====================================

    EndTextureMode();
  } // TEXTURE MODE =================================
  UnloadModel(model);

  // Make a copy of the texture from the target
  DEBUG("making copy of texture: %dx%d",
        target.texture.width,
        target.texture.height);
  Image img = LoadImageFromTexture((target.texture));
  ImageFlipVertical(&img);
  Texture tex = LoadTextureFromImage(img);

  UnloadImage(img);
  UnloadRenderTexture(target);

  return tex;
}

void assetbrowser_load(Arena* arena, AssetBrowser* browser, String8 path) {
  String8 realpath = path;
  if (!str8_ends_with(path, str8_lit("/"), 0)) {
    realpath = str8_cat(arena, path, str8_lit("/"));
  }
  browser->count = 0;
  browser->selected = -1;

  DEBUG("opening directory: %s", realpath.str);
  DIR* dir = opendir((char*)realpath.str);
  if (!dir) {
    ERROR("failed to open asset dir: %s", realpath.str);
    return;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) && browser->count < MAX_ASSETS) {
    if (entry->d_type == DT_REG) { // only accept regular files
      AssetEntry* e = &browser->entries[browser->count];
      e->name = str8_copy(arena, str8_cstring(entry->d_name));

      e->type = ASSET_OTHER;
      e->has_preview = false;

      if (str8_ends_with(e->name, str8_lit(".png"), 0)
      ||  str8_ends_with(e->name, str8_lit(".jpg"), 0)) {
        DEBUG("loading image");
        String8 full_path = str8_cat(arena, realpath, e->name);
        Image img = LoadImage((char*)full_path.str);
        if (img.data) {
          ImageResize(&img, 128, 128);
          ImageFlipVertical(&img);
          e->preview = LoadTextureFromImage(img);
          e->has_preview = true;
          e->path = full_path;
          UnloadImage(img);
        }
      } else if (str8_ends_with(e->name, str8_lit(".obj"), 0)
             ||  str8_ends_with(e->name, str8_lit(".glb"), 0)) {
        DEBUG("loading model");
        String8 full_path = str8_cat(arena, realpath, e->name);
        e->preview = generate_model_thumbnail((char*)full_path.str);
        e->path = full_path;
        e->has_preview = (e->preview.id > 0);
        e->type = ASSET_MODEL;
      }
      browser->count++;
    }
  }
  closedir(dir);
}

void assetbrowser_unload(AssetBrowser* browser) {
  // FIXME
  DEBUG("AssetBrowser: About to unload textures");
  for (usize i = 0; i < browser->count; i++) {
    DEBUG("idx %lu", i);
    if (browser->entries[i].has_preview) {
      DEBUG("idx %lu has preview of size %dx%d",
            i, browser->entries[i].preview.width, browser->entries[i].preview.height);
      UnloadTexture(browser->entries[i].preview);
      browser->entries[i].has_preview = false;
    }
  }
  INFO("AssetBrowser: Unloaded");
}

void ui_menubar(EditorState* ed) {
  struct nk_context* ctx = ed->nk_ctx;
  if (nk_begin(ctx, "MenuBar", nk_rect(0, 0, GetScreenWidth(), 25),
               NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER)) {
    ui_manager_register_window(ed->uiman, ctx, str8_lit("MenuBar"));

    nk_menubar_begin(ctx);

    nk_layout_row_begin(ctx, NK_STATIC, 15, 3);

    // ----- File Menu -----
    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 20, 1);
      if (nk_menu_item_label(ctx, "New", NK_TEXT_LEFT)) {
        // TODO: New scene logic
      }
      if (nk_menu_item_label(ctx, "Open...", NK_TEXT_LEFT)) {
        // TODO: Open scene
      }
      if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)) {
        // TODO: Save scene
      }
      nk_menu_end(ctx);
    }

    // ----- Edit Menu -----
    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200))) {
      nk_layout_row_dynamic(ctx, 20, 1);
      if (nk_menu_item_label(ctx, "Undo", NK_TEXT_LEFT)) {
      }
      if (nk_menu_item_label(ctx, "Redo", NK_TEXT_LEFT)) {
      }
      nk_menu_end(ctx);
    }

    // ----- View Menu -----
    nk_layout_row_push(ctx, 60);
    if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(150, 200))) {
      nk_layout_row_dynamic(ctx, 20, 1);
      if (nk_menu_item_label(ctx, ed->vis->show_toolbar
                             ? "[x] Toolbar"
                             : "[ ] Toolbar", NK_TEXT_LEFT))
        ed->vis->show_toolbar = !ed->vis->show_toolbar;
      if (nk_menu_item_label(ctx, ed->vis->show_hierarchy
                             ? "[x] Hierarchy"
                             : "[ ] Hierarchy", NK_TEXT_LEFT))
        ed->vis->show_hierarchy = !ed->vis->show_hierarchy;
      if (nk_menu_item_label(ctx, ed->vis->show_inspector
                             ? "[x] Inspector"
                             : "[ ] Inspector", NK_TEXT_LEFT))
        ed->vis->show_inspector = !ed->vis->show_inspector;
      if (nk_menu_item_label(ctx, ed->vis->show_assetbrowser
                             ? "[x] Assets Browser"
                             : "[ ] Assets Browser", NK_TEXT_LEFT))
        ed->vis->show_assetbrowser = !ed->vis->show_assetbrowser;
      nk_menu_end(ctx);
    }

    nk_layout_row_end(ctx);
    nk_menubar_end(ctx);
  }
  nk_end(ctx);
}

void ui_toolbar(EditorState* ed) {
  struct nk_context* ctx = ed->nk_ctx;
  struct nk_rect rect = nk_rect(GetScreenWidth()-80-10, 10+25, 80, GetScreenHeight()-25-20);
  nk_flags flags = NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR;
  nk_begin(ctx, "Toolbar", rect, flags);
  ui_manager_register_window(ed->uiman, ctx, str8_lit("Toolbar"));

  nk_layout_row_dynamic(ctx, 30, 1);
  if (nk_button_label(ctx, "Place")) {
    ed->current_tool = TOOL_PLACE;
    // TODO: implement placing
  }
  if (nk_button_label(ctx, "Transform")) {
    ed->current_tool = TOOL_TRANSFORM;
    // TODO: implement transforming
  }

  nk_end(ed->nk_ctx);
}

void ui_hierarchy(EditorState* ed) {
  struct nk_context* ctx = ed->nk_ctx;
  nk_begin(ctx, "Hierarchy", nk_rect(150, 10, 200, 400),
           NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_SCALABLE);
  ui_manager_register_window(ed->uiman, ctx, str8_lit("Hierarchy"));

  if (ed->entity_store->list_count > 0) {
    Entity* ent;
    entity_foreach(ed->entity_store, ent) {
      if (!ent->alive) continue;
      nk_layout_row_dynamic(ctx, 25, 1);
      if (nk_select_label(ctx, (char *)ent->name.str, NK_TEXT_LEFT, ed->selected_entity == ent->id)) {
        ed->selected_entity = ent->id;
      }
    }
  }

  nk_end(ctx);
}

void ui_inspector(EditorState* ed) {
  struct nk_context* ctx = ed->nk_ctx;
  nk_begin(ctx, "Inspector", nk_rect(370, 10, 250, 400),
           NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_SCALABLE);
  ui_manager_register_window(ed->uiman, ctx, str8_lit("Inspector"));

  Entity* ent = entity_store_find(ed->entity_store, ed->selected_entity);
  if (ent) {
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_label(ctx, (char*)ent->name.str, NK_TEXT_LEFT);

    nk_property_float(ctx, "Pos X", -1000, &ent->transform.translation.x, 1000, 0.1, 0.05);
    nk_property_float(ctx, "Pos Y", -1000, &ent->transform.translation.y, 1000, 0.1, 0.05);
    nk_property_float(ctx, "Pos Z", -1000, &ent->transform.translation.z, 1000, 0.1, 0.05);

    nk_property_float(ctx, "Rot X", -360, &ent->transform.rotation.x, 360, 1.0, 0.1);
    nk_property_float(ctx, "Rot Y", -360, &ent->transform.rotation.y, 360, 1.0, 0.1);
    nk_property_float(ctx, "Rot Z", -360, &ent->transform.rotation.z, 360, 1.0, 0.1);

    nk_property_float(ctx, "Scale X", 0.01, &ent->transform.scale.x, 100, 0.1, 0.05);
    nk_property_float(ctx, "Scale Y", 0.01, &ent->transform.scale.y, 100, 0.1, 0.05);
    nk_property_float(ctx, "Scale Z", 0.01, &ent->transform.scale.z, 100, 0.1, 0.05);
  }

  nk_end(ctx);
}

void ui_assetbrowser(EditorState* ed) {
  struct nk_context* ctx = ed->nk_ctx;
  AssetBrowser* browser = ed->browser;
  if (nk_begin(ctx, "AssetsBrowser", nk_rect(50, 400, 500, 250),
               NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE
              |NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
    ui_manager_register_window(ed->uiman, ctx, str8_lit("AssetsBrowser"));

    nk_layout_row_dynamic(ctx, 80, 4); // 4 thumbnails per row

    for (usize i = 0; i < browser->count; i++) {
      AssetEntry* e = &browser->entries[i];
      /* struct nk_rect bounds = nk_widget_bounds(ctx); */
      if (e->has_preview) {
        // draw texture
        // FIXME: aspect ratio
        /* Rectangle src = {0, 0, (float)e->preview.width, (float)e->preview.height}; */
        /* Rectangle dst = {bounds.x, bounds.y, bounds.w, bounds.h - 20}; */
        /* DrawTexturePro(e->preview, src, dst, v2(0,0), 0, WHITE); */
        /* bool is_selected = browser->selected == i; */
        struct nk_image img = TextureToNuklear(e->preview);
        if (nk_button_image(ctx, img)) {
          // NOTE: let's manage assets better
          browser->selected = i;
          DEBUG("browser selected idx %li", browser->selected);
          ed->selected_asset = e->path;
          DEBUG("Selected asset: %s", (char *)e->name.str);
          DEBUG("Selected path: %s", (char *)e->path.str);

          switch (e->type) {
          case ASSET_TEXTURE:
            break;
          case ASSET_MODEL:
            if (!e->model) {
              DEBUG("allocating asset entry model");
              e->model = push_one(ed->arena, Model);
            }
            DEBUG("loading model and copying into asset entry's model slot");
            *e->model = LoadModel((char*)e->path.str);
            break;
          default:
            break;
          }
        }
      } else {
        // TODO: cleanup
        bool is_selected = browser->selected == i;
        if (nk_selectable_label(ctx, "?", NK_TEXT_ALIGN_MIDDLE, &is_selected)) {
          browser->selected = i;
          ed->selected_asset = e->path;
          DEBUG("Selected asset: %s", (char *)e->name.str);
        }
        /* DrawRectangle(bounds.x, bounds.y, bounds.w, bounds.h - 20, DARKGRAY); */
        /* DrawText("?", bounds.x + bounds.w/2-4, bounds.y + bounds.h/2 - 8, 12, RAYWHITE); */
      }
      /* struct nk_image img = TextureToNuklear(e->preview); */
      /* if (nk_button_image(ctx, img)) { */
      /*   browser->selected = i; */
      /*   DEBUG("Selected asset: %s", (char *)e->name.str); */
      /* } */
      /* bool is_selected = browser->selected == i; */
      /* if (nk_selectable_label(ctx, (char *)e->name.str, NK_TEXT_CENTERED, &is_selected)) { */
      /* } */
    }
  }
  nk_end(ctx);
}

void ui_update_editor(EditorState* ed) {
  ui_menubar(ed);
  ed->uiman->count = 0;
  if (ed->vis->show_toolbar)      ui_toolbar(ed);
  if (ed->vis->show_hierarchy)    ui_hierarchy(ed);
  if (ed->vis->show_inspector)    ui_inspector(ed);
  if (ed->vis->show_assetbrowser) ui_assetbrowser(ed);
}

void ui_manager_register_window(UIManager* man, struct nk_context* ctx, String8 name) {
  if (man->count >= MAX_UI_WINDOWS) return;

  UIWindow* w = &man->windows[man->count++];
  w->bounds   = nk_window_get_bounds(ctx);
  w->active   = true;
  w->name     = name;
}

b32 ui_is_hovered(UIManager* man, struct nk_context* ctx) {
  const struct nk_input* input = &ctx->input;

  for (u64 i = 0; i < man->count; i++) {
    UIWindow* w = &man->windows[i];
    if (!w->active) continue;
    if (nk_input_is_mouse_hovering_rect(input, w->bounds)) {
      return true;
    }
  }
  return false;
}
