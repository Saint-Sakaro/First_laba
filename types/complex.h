#pragma once
#include "type.h"

const TypeInfo* get_complex_TypeInfo();

typedef struct complex {
    double re;
    double im;
} complex_t;