#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct TypeInfo {
    size_t size;
    void* (*alloc)();
    void (*free)(void*);
    void (*print)(const void*);
    void (*toString)(const void*, char*, size_t);
    void (*add)(const void*, const void*, void*);
    void (*multiply)(const void*, const void*, void*);
    void (*scalarMultiply)(const void*, void*);
    void (*negative)(const void*, void*);
    void (*division)(const void*, const void*, void*);
    int (*is_zero)(const void*);
    const void* zero;
    const void* one; // если захочу компексные - (не забыть) 
} TypeInfo;



