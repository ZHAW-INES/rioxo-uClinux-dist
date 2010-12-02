/*
 * map.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */
#include "map.h"
#include <string.h>

t_map_fnc* map_find_fnc(t_map_fnc* list, char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

t_map_set* map_find_set(t_map_set* list, char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

t_map_rec* map_find_rec(t_map_rec* list, char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

int map_call_fnc(t_map_fnc* list, char* str, void* handle, void* base)
{
    t_map_fnc *map = map_find_fnc(list, str);

    if (map) {
        mapfnc2 *f = (mapfnc2*)map->fnc;
        if (f) {
            return f(handle, map_offset(base, map->offset));
        }
    }

    return -1;
}

void* map_get_rec(t_map_rec* list, char* str)
{
    t_map_rec *r = map_find_rec(list, str);
    if (r) {
        return r->rec;
    }
    return 0;
}
