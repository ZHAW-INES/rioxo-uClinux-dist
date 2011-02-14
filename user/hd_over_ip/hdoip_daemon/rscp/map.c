/*
 * map.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */
#include "map.h"
#include <string.h>

const t_map_fnc* map_find_fnc(const t_map_fnc list[], char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

const t_map_set* map_find_set(const t_map_set list[], char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

const t_map_rec* map_find_rec(const t_map_rec list[], char* str)
{
    while (list->name) {
        if (strcmp(list->name, str) == 0) return list;
        list++;
    }
    return 0;
}

int map_call_fnc(const t_map_fnc list[], char* str, void* handle, void* base)
{
    const t_map_fnc *map = map_find_fnc(list, str);

    if (map) {
        mapfnc2 *f = (mapfnc2*)map->fnc;
        if (f) {
            return f(handle, map_offset(base, map->offset));
        }
    }

    return -1;
}

void* map_get_rec(const t_map_rec list[], char* str)
{
    const t_map_rec *r = map_find_rec(list, str);
    if (r) {
        return r->rec;
    }
    return 0;
}
