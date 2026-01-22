#ifndef __GF2D_ENTITY_C__
#define __GF2D_ENTITY_C__

#include <SDL.h>

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gf2d_sprite.h"

#include "gfc_list.h"

enum Entity_Status
{
    Active,
    Inactive
};

typedef struct Entity_S
{
    Sprite *sprite;
    GFC_Vector2D pos;
    float rotation;

    enum Entity_Status status;

    char* name;

    GFC_Vector2D image_size;
    GFC_Vector2D working_size;

    float animation_frame;
    float animation_speed;
}Entity;

void gf2d_entity_init(int count, char* config_filepath);

Entity * gf2d_create_entity(char *name);

void gf2d_delete_entity(Entity *ent);

void gf2d_think_all();
void gf2d_update_all();
void gf2d_draw_all();

void gf2d_entity_manager_slog();

void gf2d_get_entity(char * name);
#endif