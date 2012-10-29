/*
 * map.h
 * TODO: use sorted list for faster search time
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef MAP_H_
#define MAP_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int (mapfnc2)(void*, void*);

typedef struct {
    char*   name;       //!< function name
    void*   fnc;        //!< function pointer
    size_t  offset;     //!< data offset;
} t_map_fnc;

typedef struct {
    char*   name;       //!< function name
    void*   rec;        //!< record pointer
    void*   fnc;        //!< function;
} t_map_set;

typedef struct {
    char*   name;       //!< function name
    void*   rec;        //!< record pointer
} t_map_rec;

#define MAP_FNC_NULL { 0, 0, 0 }
#define MAP_SET_NULL { 0, 0, 0 }
#define MAP_REC_NULL { 0, 0 }

#define map_offset(x, y)            ((typeof(x))((uint8_t*)x + y))
#define map_fncoff(f, x, y)         ((f)((uint8_t*)x + y))

const t_map_fnc* map_find_fnc(const t_map_fnc list[], char* str);
const t_map_set* map_find_set(const t_map_set list[], char* str);
const t_map_rec* map_find_rec(const t_map_rec list[], char* str);

int map_call_fnc(const t_map_fnc list[], char* str, void* handle, void* base);
void* map_get_rec(const t_map_rec list[], char* str);

#endif /* MAP_H_ */
