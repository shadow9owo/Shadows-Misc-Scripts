#pragma once

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

extern char name[64];

typedef enum items
{
    ITEM_NONE = 0,

    ITEM_APPLE = 1,
    ITEM_BREAD = 2,
    ITEM_WATER_BOTTLE = 3,

    ITEM_RUSTY_SWORD = 10,
    ITEM_IRON_SWORD = 11,
    ITEM_GOLD_SWORD = 12,

    ITEM_SMALL_KEY = 20,
    ITEM_RED_KEY = 21,
    ITEM_BLUE_KEY = 22,

    ITEM_TRASH_BAG = 37,
    ITEM_BROKEN_CUP = 38,
    ITEM_OLD_BATTERY = 39,

    ITEM_POTION_HP_SMALL = 50,
    ITEM_POTION_HP_BIG = 51,
    ITEM_POTION_MP_SMALL = 52,

    ITEM_DEBUG_CUBE = 99,

    ITEM_UNKNOWN_1 = 133,
    ITEM_UNKNOWN_2 = 134,

    ITEM_DEV_TEST = 255

} items;

typedef struct Inventory
{
    int lenght;
    items item[255];
} Inventory;

typedef struct Vector3
{
    float x;
    float y;
    float z;
} Vector3;

typedef struct Vector4
{
    float w;
    float x;
    float y;
    float z;
} Vector4;

typedef struct Entity
{
    Vector3 position;
    Vector4 rotation;
    int health;
    int level;
    Inventory inv;
    int id;
    char name[32];
} Entity;

typedef struct EntityList
{
    int lenght;
    Entity* E_arr;
} EntityList;

inline void popat(Inventory* inv, int value)
{
    if (value < 0 || value >= inv->lenght)
    {
        return;
    }

    for (int i = value; i < inv->lenght - 1; i++)
    {
        inv->item[i] = inv->item[i + 1];
    }

    inv->item[inv->lenght - 1] = ITEM_NONE;
    inv->lenght--;
}

extern void Randomize(EntityList* list);

extern EntityList list;