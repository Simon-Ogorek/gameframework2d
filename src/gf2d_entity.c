#include "gf2d_entity.h"
#include "simple_logger.h"
#include "simple_json.h"
#include <SDL_image.h>
#include "gfc_hashmap.h"
#include "gfc_config_def.h"

static struct Entity_Manager
{
    Entity *all_ents;
    int count;
} entityManager;


void gf2d_preload_sprites(char* config_filepath)
{
    int i;
    
    SJson * imagePathJSON;
    char *imagePath;

    SJson *spriteJSON = sj_load("sprites/sprite.json");

    SJson *spriteInfoJSON = sj_object_get_value(spriteJSON, "sprites");

    SJson *spritePropertiesJSON;

    for (i = 0; i < sj_array_get_count(spriteInfoJSON); i++)
    {
        spritePropertiesJSON = sj_array_get_nth(spriteInfoJSON, i);
        if (!spritePropertiesJSON)
        {
            slog("Bad JSON in %s at entry %i for sprites", config_filepath, i);
            continue;
        }
        imagePathJSON = sj_object_get_value(spritePropertiesJSON, "filepath");
        if (!imagePathJSON)
        {
            slog("Bad JSON in %s at entry %i for sprites", config_filepath, i);
            continue;
        }
        imagePath = sj_get_string_value(imagePathJSON);
        if (!imagePath)
        {
            slog("Bad JSON in %s at entry %i for image path in sprites", config_filepath, i);
            continue;
        }
        SDL_Surface *temp = IMG_Load(imagePath);
        if (!temp)
        {
            slog("Bad Image in %s at entry %i for sprites", config_filepath, i);
            continue;
        }

        int frameWidth = 0;
        int frameHeight = 0;
        if (!sj_get_integer_value(sj_object_get_value(spritePropertiesJSON, "sheet_width"), &frameWidth) ||
            !sj_get_integer_value(sj_object_get_value(spritePropertiesJSON, "sheet_height"), &frameHeight))
        {
            slog("Bad Height/ Width in %s at entry %i for sprites", config_filepath, i);
            continue;
        }

        gf2d_sprite_load_all(imagePath, 
            temp->w / frameWidth,
            temp->h / frameHeight,
            frameWidth,
            false);



    }
    gfc_config_def_init();
    gfc_config_def_load(config_filepath);

}


void gf2d_entity_init(int count, char* config_filepath)
{
    int i;

    entityManager.all_ents = (Entity *)gfc_allocate_array(sizeof(Entity), count);
    entityManager.count = count;

    for (i = 0; i < count; i++)
    {
        entityManager.all_ents[i].status = Inactive;

    }

    gf2d_preload_sprites("sprites/sprite.json");

}

void gf2d_draw_entity(Entity *ent)
{
    gf2d_sprite_draw( ent->sprite, 
        ent->pos,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        (int)ent->animation_frame );
    }

void gf2d_think_entity(Entity *ent)
{
    if (ent->animation_frame == 0)
    {
        ent->pos = gfc_vector2d(rand()%1200, rand()%700);
    }
}

void gf2d_update_entity(Entity *ent)
{
    ent->animation_frame += ent->animation_speed;
    ent->animation_frame = (ent->animation_frame >= ent->sprite->total_frames-1) ? 0 : ent->animation_frame;
    slog("current frame of %s is %f at %f", ent->name, ent->animation_frame, ent->animation_speed);
}

Entity * gf2d_create_entity(char* name)
{
    Entity *ent;
    int i;

    for (i = 0; i < entityManager.count; i++)
    {
        ent = &entityManager.all_ents[i];

        if (ent->status == Inactive)
        {
            ent->status = Active;

            ent->name = (name ? name : "default");
            SJson *ent_info = gfc_config_def_get_by_name("sprites", name);
            char *filepath = sj_get_string_value(sj_object_get_value(ent_info, "filepath"));
            ent->sprite = gf2d_sprite_load_image(filepath);
            int code = sj_get_float_value(sj_object_get_value(ent_info, "animation_speed"), &ent->animation_speed);
            slog("%f speed %i", ent->animation_speed, code);
            return ent;
        }
    }

    slog("No inactive ent found");
    return NULL;

}
void gf2d_delete_entity(Entity *ent)
{
    free(ent->sprite);
    ent->status = Inactive;
}
void gf2d_think_all()
{
    Entity *ent;
    int i;

    for (i = 0; i < entityManager.count; i++)
    {
        ent = &entityManager.all_ents[i];

        if (ent->status == Active)
        {
            gf2d_think_entity(ent);
        }
    }
}

void gf2d_update_all()
{
    Entity *ent;
    int i;

    for (i = 0; i < entityManager.count; i++)
    {
        ent = &entityManager.all_ents[i];

        if (ent->status == Active)
        {
            gf2d_update_entity(ent);
        }
    }
}

void gf2d_draw_all()
{
    Entity *ent;
    int i;

    for (i = 0; i < entityManager.count; i++)
    {
        ent = &entityManager.all_ents[i];

        if (ent->status == Active)
        {
            gf2d_draw_entity(ent);
        }
    }
}

void gf2d_entity_manager_slog()
{
    Entity *ent;
    int i;
    
    for (i = 0; i < entityManager.count; i++)
    {
        ent = &entityManager.all_ents[i];

        if (ent->status == Active)
        {
            slog("%s | %i | %f, %f", ent->name, i, ent->pos.x, ent->pos.y);
        }
    }
}
