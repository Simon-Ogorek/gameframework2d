#ifndef __GF2D_MAP_C__
#define __GF2D_MAP_C__

#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "gfc_list.h"

typedef struct Map_Tile
{
    Sprite *sprite;
    GFC_Vector2D pos;
};

typedef struct Map_Chunk
{
    GFC_List tiles;
    GFC_Vector2D pos;
};

void gf2d_map_init(char *map_file);

void gf2d_map_draw();
void map_update();
void map_think();



#endif