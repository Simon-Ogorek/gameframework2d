#include "simple_json.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gf2d_map.h"
#include "gfc_hashmap.h"
#include "gfc_types.h"

#include <SDL_image.h>

static struct Map_Manager
{
    GFC_List *chunks;
    GFC_HashMap *tiles;

    SJson *map_info_JSON;
    int tile_size;
}map_manager;

typedef struct Tile_Definition
{
    char* tileset_file;
    int frame;

}Tile_Definition;

Tile_Definition *fetch_tile(char* name)
{
    return (Tile_Definition *)gfc_hashmap_get(map_manager.tiles, name);
}

void gf2d_map_init(char *map_file)
{
    map_manager.chunks = gfc_list_new();
    map_manager.tiles = gfc_hashmap_new();

    SJson *map_info_JSON = sj_load(map_file);

    SJson *map_tilesets_JSON = sj_object_get_value(map_info_JSON, "tilesets");

    int tile_size;
    SJson *map_tile_size_JSON =  sj_object_get_value(map_info_JSON, "sizeInPixels");
    sj_get_integer_value(map_tile_size_JSON, &tile_size);
    map_manager.tile_size = tile_size;
    map_manager.map_info_JSON = map_info_JSON;
    sj_get_integer_value(map_tile_size_JSON, &tile_size);

    for (int i = 0; i < sj_array_get_count(map_tilesets_JSON); i++)
    {

        #pragma region Tileset
        SJson *tileset_JSON = sj_array_get_nth(map_tilesets_JSON, i);

        if (!tileset_JSON)
        {
            slog("Bad JSON in %s at entry %i for map", map_file, i);
            continue;
        }

        SJson *tileset_name_JSON = sj_object_get_value(tileset_JSON, "name");
        if (!tileset_name_JSON)
        {
            slog("Bad JSON in %s at entry %i for map", map_file, i);
            continue;
        }
        char* tileset_name = sj_get_string_value(tileset_name_JSON);

        SJson *tileset_file_JSON = sj_object_get_value(tileset_JSON, "filepath");
        if (!tileset_file_JSON)
        {
            slog("Bad JSON in %s at entry %i for map", map_file, i);
            continue;
        }
        char* tileset_file = sj_get_string_value(tileset_file_JSON);
        
        SDL_Surface* temp_image = IMG_Load(tileset_file);

        int image_height = temp_image->h;
        int image_width = temp_image->w;

        SDL_FreeSurface(temp_image);

        SJson* sheet_width_JSON = sj_object_get_value(tileset_JSON, "sheet_width");
        int sheet_width;
        sj_get_integer_value(sheet_width_JSON, &sheet_width);

        SJson* sheet_height_JSON = sj_object_get_value(tileset_JSON, "sheet_height");
        int sheet_height;
        sj_get_integer_value(sheet_height_JSON, &sheet_height);

        if (!sheet_height_JSON || !sheet_width_JSON)
        {
            slog("Bad JSON in %s at entry %i for map", map_file, i);
            continue;
        }

        slog("DEBUG ( iw : %i | ih : %i | sw : %i | sh : %i )", image_width, image_height, sheet_width, sheet_height);

        slog_sync();
        Sprite* tileset_sprite = gf2d_sprite_load_all(
            tileset_file,
            image_width / sheet_width,
            image_height / sheet_height,
            sheet_width,
            NULL
        );

        #pragma endregion

        #pragma region Tile Defintion

        SJson *tiles_JSON = sj_object_get_value(tileset_JSON, "tiles");

        if (!tiles_JSON)
        {
            slog("Bad JSON in %s at entry %i for map", map_file, i);
            continue;
        }

        for (int j = 0; j < sj_array_get_count(tiles_JSON); j++)
        {
            SJson *tile_info_JSON = sj_array_get_nth(tiles_JSON, j);
            if (!tile_info_JSON)
            {
                slog("Bad JSON in %s at entry %i for map", map_file, j);
                continue;
            }
            Tile_Definition *tile = (Tile_Definition *)malloc(sizeof(Tile_Definition));
        
            tile->tileset_file = tileset_file;

            int tile_x, tile_y;
            
            SJson *tile_x_JSON = sj_object_get_value(tile_info_JSON, "x");
            sj_get_integer_value(tile_x_JSON, &tile_x);

            SJson *tile_y_JSON = sj_object_get_value(tile_info_JSON, "y");
            sj_get_integer_value(tile_x_JSON, &tile_y);

            tile->frame = (sheet_width * tile_y) + tile_x;

            SJson *tile_name_JSON = sj_object_get_value(tile_info_JSON, "name");
            char *tile_name = sj_get_string_value(tile_name_JSON);

            if (!tile_name_JSON || !tile_y_JSON || !tile_x_JSON)
            {
                slog("Bad JSON in %s at entry %i for map", map_file, j);
                continue;
            }

            gfc_hashmap_insert(map_manager.tiles, tile_name, tile);
        }
    }

    #pragma endregion
}

void gf2d_map_draw()
{
    #pragma region Map_Spawning

    SJson *map_layout_JSON = sj_object_get_value(map_manager.map_info_JSON, "map_layout");

    if (!map_layout_JSON)
    {
        slog("Bad JSON in draw");
        return;
    }
    for (int i = 0; i < sj_array_get_count(map_layout_JSON); i++)
    {
        SJson *tile_JSON = sj_array_get_nth(map_layout_JSON, i);

        if (!tile_JSON)
        {
            slog("Bad tile JSON at entry %i for map", i);
            continue;
        }

        SJson *tile_name_JSON = sj_object_get_value(tile_JSON, "tile");
        SJson *tile_x_JSON = sj_object_get_value(tile_JSON, "x");
        SJson *tile_y_JSON = sj_object_get_value(tile_JSON, "y");
        SJson *tile_z_JSON = sj_object_get_value(tile_JSON, "z");

        if (!tile_name_JSON || !tile_x_JSON || !tile_y_JSON || !tile_z_JSON)
        {
            slog("bad tile info at %i", i);
            continue;
        }

        char *tile_name = sj_get_string_value(tile_name_JSON);
        int tile_x, tile_y, tile_z;

        sj_get_integer_value(tile_x_JSON, &tile_x);
        sj_get_integer_value(tile_y_JSON, &tile_y);
        sj_get_integer_value(tile_z_JSON, &tile_z);

        Tile_Definition* tile_DEF = fetch_tile(tile_name);
        int tile_size = map_manager.tile_size;

        GFC_Vector2D pos = gfc_vector2d(tile_x * tile_size, tile_y * tile_size);
        //slog("drawing map tile at %f, %f", pos.x, pos.y);
        gf2d_sprite_render(
            gf2d_sprite_load_image(tile_DEF->tileset_file),
            pos,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            tile_DEF->frame
        );
    }
    #pragma endregion
}   
void map_update();
void map_think();



