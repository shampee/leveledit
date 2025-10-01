#include "core.h"
#include "editor.h"
#include "thirdparty/raylib.h"

const i32 WIDTH = 1280;
const i32 HEIGHT = 720;
const i32 FONT_SIZE = 10;

static void handle_keyboard(EditorState* ed) {
    if (IsKeyPressed(KEY_ONE))   ed->vis->show_toolbar      = !ed->vis->show_toolbar;
    if (IsKeyPressed(KEY_TWO))   ed->vis->show_hierarchy    = !ed->vis->show_hierarchy;
    if (IsKeyPressed(KEY_THREE)) ed->vis->show_inspector    = !ed->vis->show_inspector;
    if (IsKeyPressed(KEY_FOUR))  ed->vis->show_assetbrowser = !ed->vis->show_assetbrowser;
}

i32 main(i32 argc, char* argv[]) {
  Arena arena = {0};
  Arena entity_arena = {0};
  HashTable ht = {0};
  EditorState ed = {0};
  AssetBrowser browser = {0};
  EntityStore store = {0};
  UIVisibility vis = {
      .show_toolbar = true,
      .show_hierarchy = false,
      .show_inspector = false,
      .show_assetbrowser = false,
  };

  InitWindow(WIDTH, HEIGHT, "leveledit");
  ed.nk_ctx = InitNuklear(FONT_SIZE);

  arena_init(&arena, MB(1));
  arena_init(&entity_arena, MB(1));
  entity_store_init(&entity_arena, &store, KB(512), KB(512));
  ed.entity_store = &store;
  ed.arena = &arena;
  ed.ht = &ht;
  ed.vis = &vis;

  /* LoadTextureFromImage(()) */
  Image arrow = LoadImage("resources/cursors/Dark/Arrows/Arrow2.png");
  ImageResize(&arrow, arrow.width/2, arrow.height/2);
  ed.cursor.texture = LoadTextureFromImage(arrow);


  assetbrowser_load(ed.arena, &browser, str8_lit("resources/assets"));
  ed.browser = &browser;

  struct nk_context* nk = ed.nk_ctx;

  Camera3D camera = {
      .position   = {20.0, 20.0, 10.0},
      .target     = {0.0, 8.0, 0.0},
      .up         = {0.0, 1.0, 0.0},
      .fovy       = 45.0,
      .projection = CAMERA_PERSPECTIVE
  };

  Ray ray = {0};

  // Ground quad
  Vec3 g0 = (Vec3){-50.0f, 0.0f, -50.0f};
  Vec3 g1 = (Vec3){-50.0f, 0.0f,  50.0f};
  Vec3 g2 = (Vec3){ 50.0f, 0.0f,  50.0f};
  Vec3 g3 = (Vec3){ 50.0f, 0.0f, -50.0f};

  // Test triangle
  Vec3 ta = (Vec3){-25.0f, 0.5f, 0.0f};
  Vec3 tb = (Vec3){ -4.0f, 2.5f, 1.0f};
  Vec3 tc = (Vec3){ -8.0f, 6.5f, 0.0f};

  Vec3 bary = {0.0f, 0.0f, 0.0f};

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    if (IsCursorHidden()) UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    // Mouse -------------------------------------
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      if (IsCursorHidden()) EnableCursor();
      else DisableCursor();
    }
    ed.cursor.position = GetMousePosition();
    // Keyboard -----------------------------------
    handle_keyboard(&ed);

    // Display information about closest hit
    RayCollision collision = {0};
    const char* hit_object_name = "None";
    collision.distance = FLT_MAX;
    collision.hit = false;
    Color cursorColor = WHITE;

    // Get ray and test against objects
    ray = GetScreenToWorldRay(GetMousePosition(), camera);

    // Check ray collision against ground quad
    RayCollision ground_hit_info = GetRayCollisionQuad(ray, g0, g1, g2, g3);

    if ((ground_hit_info.hit) && (ground_hit_info.distance < collision.distance)) {
      collision = ground_hit_info;
      cursorColor = GREEN;
      hit_object_name = "Ground";
    }

    // Check ray collision against test triangle
    RayCollision tri_hit_info = GetRayCollisionTriangle(ray, ta, tb, tc);

    if ((tri_hit_info.hit) && (tri_hit_info.distance < collision.distance)) {
      collision = tri_hit_info;
      cursorColor = PURPLE;
      hit_object_name = "Triangle";

      bary = v3_bary(collision.point, ta, tb, tc);
    }


    UpdateNuklear(nk);
    ui_update_editor(&ed);

    { // Draw ----------------------------------------------
      BeginDrawing();

      ClearBackground(RAYWHITE);

      BeginMode3D(camera);


      // Draw the test triangle
      DrawLine3D(ta, tb, PURPLE);
      DrawLine3D(tb, tc, PURPLE);
      DrawLine3D(tc, ta, PURPLE);


      // If we hit something, draw the cursor at the hit point
      if (collision.hit) {
        DrawCube(collision.point, 0.3f, 0.3f, 0.3f, cursorColor);
        DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);

        Vec3 normalEnd;
        normalEnd.x = collision.point.x + collision.normal.x;
        normalEnd.y = collision.point.y + collision.normal.y;
        normalEnd.z = collision.point.z + collision.normal.z;

        DrawLine3D(collision.point, normalEnd, RED);

        switch (ed.current_tool) {
        case TOOL_PLACE:
          if (ed.selected_entity > 0) {
            DEBUG("list count: %lu", ed.entity_store->list_count);
            DEBUG("list next id: %lu", ed.entity_store->next_id);
            Entity* ent = entity_store_find(ed.entity_store, ed.selected_entity);
            if (ent->model) {
              DEBUG("found model, drawing");
              DrawModel(*ent->model, collision.point, 1, WHITE);
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
              // TODO: placing
              // this would need serialization
            }
          }
          break;
        case TOOL_TRANSFORM:
          DrawText("TRANSFORM MODE", 10, GetScreenHeight()-40, 10, BLACK);
          break;
        default:
          break;
        }
      }

      DrawRay(ray, MAROON);

      DrawGrid(10, 10.0f);

      EndMode3D();

      // Draw some debug GUI text
      DrawText(TextFormat("Hit Object: %s", hit_object_name), 10, 50, 10, BLACK);

      if (collision.hit) {
        int ypos = 70;
        DrawText(TextFormat("Distance: %3.2f", collision.distance), 10, ypos, 10, BLACK);
        DrawText(TextFormat("Hit Pos: %3.2f %3.2f %3.2f",
                            collision.point.x, collision.point.y, collision.point.z),
                 10, ypos + 15, 10, BLACK);

        DrawText(TextFormat("Hit Norm: %3.2f %3.2f %3.2f",
                            collision.normal.x, collision.normal.y, collision.normal.z),
                 10, ypos + 30, 10, BLACK);

        if (tri_hit_info.hit && TextIsEqual(hit_object_name, "Triangle")) {
          DrawText(TextFormat("Barycenter: %3.2f %3.2f %3.2f",
                              bary.x, bary.y, bary.z),
                   10, ypos + 45, 10, BLACK);
        }
      }
      switch (ed.current_tool) {
      case TOOL_PLACE:
          DrawText("PLACE MODE", 10, GetScreenHeight()-40, 10, BLACK);
          break;
      case TOOL_TRANSFORM:
          DrawText("TRANSFORM MODE", 10, GetScreenHeight()-40, 10, BLACK);
        break;
      default:
        break;
      }

      DrawFPS(10, GetScreenHeight()-20);

      DrawNuklear(nk);
      DrawTexture(ed.cursor.texture, ed.cursor.position.x, ed.cursor.position.y, BLACK);
      EndDrawing();
    } // Draw ---------------------------------------------
  }

  CloseWindow();

  assetbrowser_unload(&browser);
  UnloadNuklear(nk);
  arena_free(&entity_arena);
  arena_free(&arena);
  return 0;
}
