#include "double.h"

static void* double_alloc() {
    double* value = malloc(sizeof(double));
    *value = 0.0; // ноль по умолчанию 
    return value;
}

static void double_free(void* element) {
    free(element);
}

static void double_print(const void* element) {
    printf("%lf", *(const double*)element);
}

static void double_to_string(const void* element, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%lf", *(const double*)element);
}

static void double_add(const void* a, const void* b, void* result) {
    *(double*)result = *(const double*)a + *(const double*)b;
}

static void double_multiply(const void* a, const void* b, void* result) {
    *(double*)result = *(const double*)a * *(const double*)b;
}

static void double_scalar_multiply(const void* scalar, void* element) {
    *(double*)element *= *(const double*)scalar;
}

static void double_negative(const void* element, void* result) {
    *(double*)result = -*(const double*)element;
}

static void double_division(const void* a, const void* b, void* result) {
    *(double*)result = *(const double*)a / *(const double*)b;
}

static int double_is_zero(const void* element) {
    return fabs(*(const double*)element) < 1e-10;
}

static TypeInfo* DOUBLE_TYPE_INFO = NULL;

static const double ZERO = 0.0;
static const double ONE = 1.0;

const TypeInfo* get_double_TypeInfo() {
    if (DOUBLE_TYPE_INFO == NULL) {
        DOUBLE_TYPE_INFO = malloc(sizeof(TypeInfo));
        DOUBLE_TYPE_INFO->size = sizeof(double);
        DOUBLE_TYPE_INFO->alloc = double_alloc;
        DOUBLE_TYPE_INFO->free = double_free;
        DOUBLE_TYPE_INFO->print = double_print;
        DOUBLE_TYPE_INFO->toString = double_to_string;
        DOUBLE_TYPE_INFO->add = double_add;
        DOUBLE_TYPE_INFO->multiply = double_multiply;
        DOUBLE_TYPE_INFO->scalarMultiply = double_scalar_multiply;
        DOUBLE_TYPE_INFO->negative = double_negative;
        DOUBLE_TYPE_INFO->division = double_division;
        DOUBLE_TYPE_INFO->is_zero = double_is_zero;
        DOUBLE_TYPE_INFO->zero = &ZERO;
        DOUBLE_TYPE_INFO->one = &ONE;
    }
    return DOUBLE_TYPE_INFO;
}