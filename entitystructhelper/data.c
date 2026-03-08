#include "data.h"

char name[64] = "player";
EntityList list = {0};

void Randomize(EntityList* list)
{
    if (list->E_arr != NULL)
    {
        free(list->E_arr);
    }

    list->lenght = rand() % 16 + 4;
    list->E_arr = malloc(sizeof(Entity) * list->lenght);

    for (int i = 0; i < list->lenght; i++)
    {
        list->E_arr[i].position.x = rand() % 1000;
        list->E_arr[i].position.y = rand() % 1000;
        list->E_arr[i].position.z = rand() % 1000;

        list->E_arr[i].rotation.w = rand() % 360;
        list->E_arr[i].rotation.x = rand() % 360;
        list->E_arr[i].rotation.y = rand() % 360;
        list->E_arr[i].rotation.z = rand() % 360;

        list->E_arr[i].health = rand() % 100;
        list->E_arr[i].level = rand() % 50;

        list->E_arr[i].id = i;

        if (i == 0)
        {
            snprintf(list->E_arr[i].name, 32, "%s", name);
        }
        else
        {
            snprintf(list->E_arr[i].name, 32, "garbage_entity_%d", i);
        }

        list->E_arr[i].inv.lenght = rand() % 10;

        for (int j = 0; j < 255; j++)
        {
            if (j < list->E_arr[i].inv.lenght)
            {
                list->E_arr[i].inv.item[j] = rand() % 256;
            }
            else
            {
                list->E_arr[i].inv.item[j] = ITEM_NONE;
            }
        }
    }
}