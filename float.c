#include "float.h"

static void* float_alloc() {
    float* value = malloc(sizeof(float));
    *value = 0.0f; // ноль по умочанию 
    return value;
}

static void float_free(void* element) {
    free(element);
}

static void float_print(const void* element) {
    printf("%f", *(const float*)element);
}

static void float_to_string(const void* element, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%f", *(const float*)element);
}

static void float_add(const void* a, const void* b, void* result) {
    *(float*)result = *(const float*)a + *(const float*)b;
}

static void float_multiply(const void* a, const void* b, void* result) {
    *(float*)result = *(const float*)a * *(const float*)b;
}

static void float_scalar_multiply(const void* scalar, void* element) {
    *(float*)element *= *(const float*)scalar;
}

static void float_negative(const void* element, void* result) {
    *(float*)result = -*(const float*)element;
}

static void float_division(const void* a, const void* b, void* result) {
    *(float*)result = *(const float*)a / *(const float*)b;
}

static int float_is_zero(const void* element) {
    return fabs(*(const float*)element) < 1e-6f;
}

static TypeInfo* FLOAT_TYPE_INFO = NULL;

static const float ZERO = 0.0f;
static const float ONE = 1.0f;

const TypeInfo* get_float_TypeInfo() {
    if (FLOAT_TYPE_INFO == NULL) {
        FLOAT_TYPE_INFO = malloc(sizeof(TypeInfo));
        FLOAT_TYPE_INFO->size = sizeof(float);
        FLOAT_TYPE_INFO->alloc = float_alloc;
        FLOAT_TYPE_INFO->free = float_free;
        FLOAT_TYPE_INFO->print = float_print;
        FLOAT_TYPE_INFO->toString = float_to_string;
        FLOAT_TYPE_INFO->add = float_add;
        FLOAT_TYPE_INFO->multiply = float_multiply;
        FLOAT_TYPE_INFO->scalarMultiply = float_scalar_multiply;
        FLOAT_TYPE_INFO->negative = float_negative;
        FLOAT_TYPE_INFO->division = float_division;
        FLOAT_TYPE_INFO->is_zero = float_is_zero;
        FLOAT_TYPE_INFO->zero = &ZERO;
        FLOAT_TYPE_INFO->one = &ONE; // можно убрать
    }
    return FLOAT_TYPE_INFO;
}