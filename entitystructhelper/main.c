#include <stdio.h>

#include "data.h"

int main()
{
    printf("Learn entitylist finding : tutorial goat program\n");
    getchar();
    printf("input your name\n");
    scanf("%63s",name);

    while (1)
    {
        for (int i = 0; i < list.lenght; i++)
        {
            printf("Entity %d\n", i);
            printf("Name: %s\n", list.E_arr[i].name);
            printf("ID: %d\n", list.E_arr[i].id);
            printf("Health: %d\n", list.E_arr[i].health);
            printf("Level: %d\n", list.E_arr[i].level);
        
            printf("Position: %.2f %.2f %.2f\n",
                list.E_arr[i].position.x,
                list.E_arr[i].position.y,
                list.E_arr[i].position.z);
            
            printf("Rotation: %.2f %.2f %.2f %.2f\n",
                list.E_arr[i].rotation.w,
                list.E_arr[i].rotation.x,
                list.E_arr[i].rotation.y,
                list.E_arr[i].rotation.z);
            
            printf("Inventory length: %d\n", list.E_arr[i].inv.lenght);
            
            for (int j = 0; j < list.E_arr[i].inv.lenght; j++)
            {
                printf("Slot %d: %d\n", j, list.E_arr[i].inv.item[j]);
            }
        
            printf("\n");
        }
        getchar();
        Randomize(&list);
    }
}