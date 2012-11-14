/*
 * rtsp_map.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_MAP_H_
#define RTSP_MAP_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int (mapfnc2)(void*, void*);

typedef struct {
    char*   name;         //!< function name
    void*   fnc;          //!< function pointer
    size_t  offset;       //!< data offset;
} t_map_fnc;

typedef struct {
    char*   name;         //!< function name
    void*   rec;          //!< record pointer
    void*   fnc;          //!< function;
    bool    in_session;   //!< whether the method is only valid in a session
    int     states;       //!< states in which this method is valid
    size_t  impl_offset;  //!< offset of the implementation in t_rtsp_media
} t_map_set;

typedef struct {
    char*   name;         //!< function name
    void*   rec;          //!< record pointer
} t_map_rec;

#define MAP_FNC_NULL { 0, 0, 0 }
#define MAP_SET_NULL { 0, 0, 0, false, 0, 0 }
#define MAP_REC_NULL { 0, 0 }

#define map_offset(x, y)            ((typeof(x))((uint8_t*)x + y))
#define map_fncoff(f, x, y)         ((f)((uint8_t*)x + y))

const t_map_fnc* map_find_fnc(const t_map_fnc list[], char* str);
const t_map_set* map_find_set(const t_map_set list[], char* str);
const t_map_rec* map_find_rec(const t_map_rec list[], char* str);

int map_call_fnc(const t_map_fnc list[], char* str, void* handle, void* base);
void* map_get_rec(const t_map_rec list[], char* str);

#endif /* RTSP_MAP_H_ */
