#include "int.h"

static void* int_alloc() {
    int* value = malloc(sizeof(int));
    *value = 0; // ноль по умолчанию 
    return value;
}

static void int_free(void* element) {
    free(element);
}

static void int_print(const void* element) {
    printf("%d", *(const int*)element);
}

static void int_to_string(const void* element, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%d", *(const int*)element);
}


static void int_add(const void* a, const void* b, void* result) {
    *(int*)result = *(const int*)a + *(const int*)b;
}

static void int_multiply(const void* a, const void* b, void* result) {
    *(int*)result = *(const int*)a * *(const int*)b;
}

static void int_scalar_multiply(const void* scalar, void* element) {
    *(int*)element *= *(const int*)scalar;
}

static void int_negative(const void* element, void* result) {
    *(int*)result = -*(const int*)element;
}

static void int_division(const void* a, const void* b, void* result) {
    *(int*)result = *(const int*)a / *(const int*)b;
}

static int int_is_zero(const void* element) {
    return *(const int*)element == 0;
}

static TypeInfo* INT_TYPE_INFO = NULL;

static const int ZERO = 0;
static const int ONE = 1;

const TypeInfo* get_integer_TypeInfo() {
    if (INT_TYPE_INFO == NULL) {
        INT_TYPE_INFO = malloc(sizeof(TypeInfo));
        INT_TYPE_INFO->size = sizeof(int);
        INT_TYPE_INFO->alloc = int_alloc;
        INT_TYPE_INFO->free = int_free;
        INT_TYPE_INFO->print = int_print;
        INT_TYPE_INFO->toString = int_to_string;
        INT_TYPE_INFO->add = int_add;
        INT_TYPE_INFO->multiply = int_multiply;
        INT_TYPE_INFO->scalarMultiply = int_scalar_multiply;
        INT_TYPE_INFO->negative = int_negative;
        INT_TYPE_INFO->division = int_division;
        INT_TYPE_INFO->is_zero = int_is_zero;
        INT_TYPE_INFO->zero = &ZERO;
        INT_TYPE_INFO->one = &ONE; // можно убрать
    }
    return INT_TYPE_INFO;
}