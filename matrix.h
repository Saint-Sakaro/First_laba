#pragma once
#include <string.h>
#include "type.h"
#include "error.h"
#include "int.h"
#include "float.h"
#include "double.h"
#include "complex.h"

typedef struct matrix {
    size_t size;
    void* data;
    const TypeInfo* typeInfo;
} matrix_t;

matrix_t* create_matrix(int size, const TypeInfo* typeInfo, matrix_error_t* err);
void free_matrix(matrix_t* matrix, matrix_error_t* err);
void set_element(matrix_t* matrix, int row, int col, const void* value, const TypeInfo* valueType, matrix_error_t* err);
void get_element(const matrix_t* matrix, int row, int col, void* value, const TypeInfo* valueType, matrix_error_t* err);
void add_matrices(const matrix_t* a, const matrix_t* b, matrix_t* result, matrix_error_t* err);
void multiply_matrices(const matrix_t* a, const matrix_t* b, matrix_t* result, matrix_error_t* err);
void scalar_multiply_matrix(const matrix_t* matrix, const void* scalar, const TypeInfo* valueType, matrix_t* result, matrix_error_t* err);
void print_matrix(const matrix_t* matrix, matrix_error_t* err);
void add_linear_combination(matrix_t* matrix, int row, int count_rows, const int* rows, const void* coeffs, const TypeInfo* coeffsType, matrix_error_t* err);
void make_LU_decomposition(const matrix_t* matrix, matrix_t* L, matrix_t* U, matrix_error_t* err);
char* matrix_to_string(const matrix_t* matrix, matrix_error_t* err);