#include "complex.h"

static void* complex_alloc() {
    complex_t* value = malloc(sizeof(complex_t));
    value->re = 0.0;  // ноль по умолчанию 
    value->im = 0.0;  // ноль по умолчанию 
    return value;
}

static void complex_free(void* element) {
    free(element);
}

static void complex_print(const void* element) {
    const complex_t* c = (const complex_t*)element;
    printf("(%lf + %lf i)", c->re, c->im);
}

static void complex_to_string(const void* element, char* buffer, size_t buffer_size) {
    const complex_t* c = (const complex_t*)element;
    char sign = (c->im >= 0) ? '+' : '-';
    snprintf(buffer, buffer_size, "(%lf %c %lf i)", c->re, sign, fabs(c->im));
}

static void complex_add(const void* a, const void* b, void* result) {
    const complex_t* c_a = (const complex_t*)a;
    const complex_t* c_b = (const complex_t*)b;
    complex_t* res = (complex_t*)result;
    res->re = c_a->re + c_b->re;
    res->im = c_a->im + c_b->im;
}

static void complex_multiply(const void* a, const void* b, void* result) {
    const complex_t* c_a = (const complex_t*)a;
    const complex_t* c_b = (const complex_t*)b;
    complex_t* res = (complex_t*)result;
    res->re = c_a->re * c_b->re - c_a->im * c_b->im;
    res->im = c_a->re * c_b->im + c_a->im * c_b->re;
}

static void complex_scalar_multiply(const void* scalar, void* element) {
    const complex_t* c_s = (const complex_t*)scalar;
    complex_t* c = (complex_t*)element;
    c->re = c->re * c_s->re - c->im * c_s->im;
    c->im = c->re * c_s->im + c->im * c_s->re;
}

static void complex_negative(const void* element, void* result) {
    const complex_t* c = (const complex_t*)element;
    complex_t* res = (complex_t*)result;
    res->re = -c->re;
    res->im = -c->im;
}

static void complex_division(const void* a, const void* b, void* result) {
    const complex_t* ca = (const complex_t*)a;
    const complex_t* cb = (const complex_t*)b;
    complex_t* res = (complex_t*)result;
    double division = cb->re * cb->re + cb->im * cb->im;
    res->re = (ca->re * cb->re + ca->im * cb->im) / division;
    res->im = (ca->im * cb->re - ca->re * cb->im) / division;
}

static int complex_is_zero(const void* element) {
    const complex_t* c = (const complex_t*)element;
    return fabs(c->re) < 1e-8 && fabs(c->im) < 1e-8;
}

static const complex_t COMPLEX_ZERO = {0.0, 0.0};
static const complex_t COMPLEX_ONE = {1.0, 0.0};

static TypeInfo* COMPLEX_TYPE_INFO = NULL;

const TypeInfo* get_complex_TypeInfo() {
    if (COMPLEX_TYPE_INFO == NULL) {
        COMPLEX_TYPE_INFO = malloc(sizeof(TypeInfo));
        COMPLEX_TYPE_INFO->size = sizeof(complex_t);
        COMPLEX_TYPE_INFO->alloc = complex_alloc;
        COMPLEX_TYPE_INFO->free = complex_free;
        COMPLEX_TYPE_INFO->print = complex_print;
        COMPLEX_TYPE_INFO->toString = complex_to_string;
        COMPLEX_TYPE_INFO->add = complex_add;
        COMPLEX_TYPE_INFO->multiply = complex_multiply;
        COMPLEX_TYPE_INFO->scalarMultiply = complex_scalar_multiply;
        COMPLEX_TYPE_INFO->negative = complex_negative;
        COMPLEX_TYPE_INFO->division = complex_division;
        COMPLEX_TYPE_INFO->is_zero = complex_is_zero;
        COMPLEX_TYPE_INFO->zero = &COMPLEX_ZERO;
        COMPLEX_TYPE_INFO->one = &COMPLEX_ONE;
    }
    return COMPLEX_TYPE_INFO;
}