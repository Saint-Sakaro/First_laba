#include "matrix.h"

#define MAX_MATRIX_SIZE 1024

matrix_t* create_matrix(int size, const TypeInfo* typeInfo, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (size <= 0 || size > MAX_MATRIX_SIZE) {
        if (err) *err = MATRIX_ERR_INVALID_SIZE;
        return NULL;
    }

    if (!typeInfo) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return NULL;
    }

    matrix_t* matrix = malloc(sizeof(matrix_t));
    if (!matrix) {
        if (err) *err = MATRIX_ERR_ALLOC;
        return NULL;
    }

    matrix->size = size;
    matrix->typeInfo = typeInfo;
    matrix->data = malloc(size * size * sizeof(void*));
    if (!matrix->data) {
        free(matrix);
        if (err) *err = MATRIX_ERR_ALLOC;
        return NULL;
    }

    for (int i = 0; i < size * size; i++) {
        ((void**)matrix->data)[i] = typeInfo->alloc();

        if (!((void**)matrix->data)[i]) {
            for (int j = 0; j < i; j++) {
                typeInfo->free(((void**)matrix->data)[j]);
            }
            free(matrix->data);
            free(matrix);
            if (err) *err = MATRIX_ERR_ALLOC;
            return NULL;
        }
    }
    return matrix;
}

void free_matrix(matrix_t* matrix, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }


    for (int i = 0; i < matrix->size * matrix->size; i++) {
        matrix->typeInfo->free(((void**)matrix->data)[i]);
    }
    free(matrix->data);
    free(matrix);
}

void set_element(matrix_t* matrix, int row, int col, const void* value, const TypeInfo* valueType, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix || !value || !valueType) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (row < 0 || row >= matrix->size || col < 0 || col >= matrix->size) {
        if (err) *err = MATRIX_ERR_INDEX_OOB;
        return;
    }

    if (matrix->typeInfo != valueType) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    int index = row * matrix->size + col;
    // matrix->typeInfo->free(((void**)matrix->data)[index]);
    // ((void**)matrix->data)[index] = matrix->typeInfo->alloc();   --- идеальный варик если бы не ошибка алока, из-за которой придеться сначала алокать, а лишь потом копировать
    // memcpy(((void**)matrix->data)[index], value, matrix->typeInfo->size);
    void* old_element = ((void**)matrix->data)[index];
    void* now_element = matrix->typeInfo->alloc();

    if (!now_element) {
        if (err) *err = MATRIX_ERR_ALLOC;
        return;
    }

    memcpy(now_element, value, matrix->typeInfo->size);
    ((void**)matrix->data)[index] = now_element;
    matrix->typeInfo->free(old_element);
}

void get_element(const matrix_t* matrix, int row, int col, void* value, const TypeInfo* valueType, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix || !value || !valueType) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (row < 0 || row >= matrix->size || col < 0 || col >= matrix->size) {
        if (err) *err = MATRIX_ERR_INDEX_OOB;
        return;
    }

    if (matrix->typeInfo != valueType) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    int index = row * matrix->size + col;
    memcpy(value, ((void**)matrix->data)[index], matrix->typeInfo->size);
}

void add_matrices(const matrix_t* a, const matrix_t* b, matrix_t* result, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!a || !b || !result) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (a->size != b->size || a->size != result->size) {
        if (err) *err = MATRIX_ERR_INVALID_SIZE;
        return;
    }

    if (a->typeInfo != b->typeInfo || a->typeInfo != result->typeInfo) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    for (int i = 0; i < a->size * a->size; i++) {
        a->typeInfo->add(((void**)a->data)[i], ((void**)b->data)[i], ((void**)result->data)[i]);
    }
}

void multiply_matrices(const matrix_t* a, const matrix_t* b, matrix_t* result, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!a || !b || !result) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (a->size != b->size || a->size != result->size) {
        if (err) *err = MATRIX_ERR_INVALID_SIZE;
        return;
    }

    if (a->typeInfo != b->typeInfo || a->typeInfo != result->typeInfo) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    void* now_value = a->typeInfo->alloc();
    void* now_multiply = a->typeInfo->alloc();

    if (!now_value || !now_multiply) {
        a->typeInfo->free(now_value);
        a->typeInfo->free(now_multiply);
        if (err) *err = MATRIX_ERR_ALLOC;
        return;
    }

    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < a->size; j++) {
            for (int k = 0; k < a->size; k++) {
                a->typeInfo->multiply(((void**)a->data)[i * a->size + k], ((void**)b->data)[k * b->size + j], now_multiply);
                a->typeInfo->add(now_value, now_multiply, now_value);
            }
            set_element(result, i, j, now_value, result->typeInfo, err);
            memcpy(now_value, a->typeInfo->zero, result->typeInfo->size);

            if (err && *err != MATRIX_OK) {  // любимая segmentation fault
                a->typeInfo->free(now_value);
                a->typeInfo->free(now_multiply);
                return;
            }
        }
    }
    a->typeInfo->free(now_value);
    a->typeInfo->free(now_multiply);
}

void scalar_multiply_matrix(const matrix_t* matrix, const void* scalar, const TypeInfo* valueType, matrix_t* result, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix || !scalar || !result || !valueType) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (matrix->size != result->size) {
        if (err) *err = MATRIX_ERR_INVALID_SIZE;
        return;
    }

    if (matrix->typeInfo != result->typeInfo || valueType != matrix->typeInfo) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    for (int i = 0; i < matrix->size * matrix->size; i++) {
        memcpy(((void**)result->data)[i], ((void**)matrix->data)[i], matrix->typeInfo->size);
        matrix->typeInfo->scalarMultiply(scalar, ((void**)result->data)[i]);
    }
}


void print_matrix(const matrix_t* matrix, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;
    
    if (!matrix) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    for (int i = 0; i < matrix->size; i++) {
        printf("[ ");
        for (int j = 0; j < matrix->size; j++) {
            matrix->typeInfo->print(((void**)matrix->data)[i * matrix->size + j]);
            printf(" ");
        }
        printf("]\n");
    }
    printf("\n");
}

void add_linear_combination(matrix_t* matrix, int row, int count_rows, const int* rows, const void* coeffs, const TypeInfo* coeffsType, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix || !rows || !coeffs || !coeffsType) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (row < 0 || row >= matrix->size || count_rows <= 0 || count_rows > matrix->size) {
        if (err) *err = MATRIX_ERR_INDEX_OOB;
        return;
    }

    if (matrix->typeInfo != coeffsType) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    void* row_value = matrix->typeInfo->alloc();
    void* sum_of_koef_mul_with_row = matrix->typeInfo->alloc();
    void* now_value = matrix->typeInfo->alloc();
    void* koef_mul_with_row = matrix->typeInfo->alloc();

    if (!row_value || !sum_of_koef_mul_with_row || !now_value || !koef_mul_with_row) {
        matrix->typeInfo->free(row_value);
        matrix->typeInfo->free(sum_of_koef_mul_with_row);
        matrix->typeInfo->free(now_value);
        matrix->typeInfo->free(koef_mul_with_row);
        if (err) *err = MATRIX_ERR_ALLOC;
        return;
    }

    for (int j = 0; j < matrix->size; j++) {
        memcpy(sum_of_koef_mul_with_row, matrix->typeInfo->zero, matrix->typeInfo->size);

        for (int k = 0; k < count_rows; k++) {
            if (rows[k] < 0 || rows[k] >= matrix->size) {
                if (err) *err = MATRIX_ERR_INDEX_OOB;
                matrix->typeInfo->free(row_value);
                matrix->typeInfo->free(sum_of_koef_mul_with_row);
                matrix->typeInfo->free(now_value);
                matrix->typeInfo->free(koef_mul_with_row);
                return;
            }

            get_element(matrix, rows[k], j, now_value, matrix->typeInfo, err);

            if (err && *err != MATRIX_OK) {
                matrix->typeInfo->free(row_value);
                matrix->typeInfo->free(sum_of_koef_mul_with_row);
                matrix->typeInfo->free(now_value);
                matrix->typeInfo->free(koef_mul_with_row);
                return;
            }

            matrix->typeInfo->multiply(now_value, (const char*)coeffs + k * matrix->typeInfo->size, koef_mul_with_row);
            matrix->typeInfo->add(sum_of_koef_mul_with_row, koef_mul_with_row, sum_of_koef_mul_with_row);
        }

        get_element(matrix, row, j, row_value, matrix->typeInfo, err);
        if (err && *err != MATRIX_OK) {
            matrix->typeInfo->free(row_value);
            matrix->typeInfo->free(sum_of_koef_mul_with_row);
            matrix->typeInfo->free(now_value);
            matrix->typeInfo->free(koef_mul_with_row);
            return;
        }

        matrix->typeInfo->add(row_value, sum_of_koef_mul_with_row, row_value);

        set_element(matrix, row, j, row_value, matrix->typeInfo, err);
        if (err && *err != MATRIX_OK) {
            matrix->typeInfo->free(row_value);
            matrix->typeInfo->free(sum_of_koef_mul_with_row);
            matrix->typeInfo->free(now_value);
            matrix->typeInfo->free(koef_mul_with_row);
            return;
        }
    }

    matrix->typeInfo->free(now_value);
    matrix->typeInfo->free(koef_mul_with_row);
    matrix->typeInfo->free(row_value);
    matrix->typeInfo->free(sum_of_koef_mul_with_row);
}


char* matrix_to_string(const matrix_t* matrix, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;
    
    if (!matrix || !matrix->data) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return NULL;
    }

    size_t size = matrix->size;
    size_t max_width = 0;
    char element_buffer[64];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            void* element = ((void**)matrix->data)[i * size + j];
            matrix->typeInfo->toString(element, element_buffer, sizeof(element_buffer));
            size_t len = strlen(element_buffer);
            if (len > max_width) {
                max_width = len;
            }
        }
    }

    size_t buffer_size = (size * size * (max_width + 3)) + (size * 3) + 1; // (сами элементв + пробелы + |) + (| /n) + (0)
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        if (err) *err = MATRIX_ERR_ALLOC;
        return NULL;
    }

    size_t move = 0;

    for (int i = 0; i < size; i++) {
        move += snprintf(buffer + move, buffer_size - move, "| ");
        for (int j = 0; j < size; j++) {
            void* element = ((void**)matrix->data)[i * size + j];
            matrix->typeInfo->toString(element, element_buffer, sizeof(element_buffer));
            size_t len = strlen(element_buffer);
            int left_spaces = (max_width - len) / 2; // можно и size_t, но ловить предупреждение о int для %*
            int right_spaces = max_width - len - left_spaces;
            // move += snprintf(buffer + move, buffer_size - move, "%s ", element_buffer);
            move += snprintf(buffer + move, buffer_size - move, "%*s%s%*s | ", left_spaces, "", element_buffer, right_spaces, "");
        }
        move += snprintf(buffer + move, buffer_size - move, "\n");
    }
    return buffer;
}

void make_LU_decomposition(const matrix_t* matrix, matrix_t* L, matrix_t* U, matrix_error_t* err) {
    if (err) *err = MATRIX_OK;

    if (!matrix || !L || !U) {
        if (err) *err = MATRIX_ERR_NULL_PTR;
        return;
    }

    if (matrix->size != L->size || matrix->size != U->size) {
        if (err) *err = MATRIX_ERR_INVALID_SIZE;
        return;
    }

    if (matrix->typeInfo != L->typeInfo || matrix->typeInfo != U->typeInfo) {
        if (err) *err = MATRIX_ERR_TYPE_MISMATCH;
        return;
    }

    void* sum_for_U = matrix->typeInfo->alloc();
    void* sum_for_L = matrix->typeInfo->alloc();
    void* L_elem_mul_with_U_elem = matrix->typeInfo->alloc();
    void* negtive_sum_for_U = matrix->typeInfo->alloc();
    void* negtive_sum_for_L = matrix->typeInfo->alloc();
    void* division_for_L = matrix->typeInfo->alloc();

    if (!sum_for_U || !sum_for_L || !L_elem_mul_with_U_elem || !negtive_sum_for_U || !negtive_sum_for_L || !division_for_L) {
        if (err) *err = MATRIX_ERR_ALLOC;
        matrix->typeInfo->free(sum_for_U);
        matrix->typeInfo->free(sum_for_L);
        matrix->typeInfo->free(L_elem_mul_with_U_elem);
        matrix->typeInfo->free(negtive_sum_for_U);
        matrix->typeInfo->free(negtive_sum_for_L);
        matrix->typeInfo->free(division_for_L);
        return;
    }

    for (int i = 0; i < matrix->size; i++) {
        for (int j = 0; j < matrix->size; j++) {
            memcpy(sum_for_U, matrix->typeInfo->zero, matrix->typeInfo->size);
            memcpy(sum_for_L, matrix->typeInfo->zero, matrix->typeInfo->size);

            if (i <= j) {
                if (i == j)  {
                    set_element(L, i, i, matrix->typeInfo->one, matrix->typeInfo, err);
                    if (err && *err != MATRIX_OK) {
                        matrix->typeInfo->free(sum_for_U);
                        matrix->typeInfo->free(sum_for_L);
                        matrix->typeInfo->free(L_elem_mul_with_U_elem);
                        matrix->typeInfo->free(negtive_sum_for_U);
                        matrix->typeInfo->free(negtive_sum_for_L);
                        matrix->typeInfo->free(division_for_L);
                        return;
                    }
                }
                for (int k = 0; k < i; k++) {
                    matrix->typeInfo->multiply(((void**)L->data)[i * L->size + k], ((void**)U->data)[k * U->size + j], L_elem_mul_with_U_elem);
                    matrix->typeInfo->add(sum_for_U, L_elem_mul_with_U_elem, sum_for_U);
                }
                matrix->typeInfo->negative(sum_for_U, negtive_sum_for_U);
                matrix->typeInfo->add(negtive_sum_for_U, ((void**)matrix->data)[i * matrix->size + j],  negtive_sum_for_U);
                set_element(U, i, j, negtive_sum_for_U, matrix->typeInfo, err);
                if (err && *err != MATRIX_OK) {
                    matrix->typeInfo->free(sum_for_U);
                    matrix->typeInfo->free(sum_for_L);
                    matrix->typeInfo->free(L_elem_mul_with_U_elem);
                    matrix->typeInfo->free(negtive_sum_for_U);
                    matrix->typeInfo->free(negtive_sum_for_L);
                    matrix->typeInfo->free(division_for_L);
                    return;
                }
            }

            else {
                void* ujj = ((void**)U->data)[j * U->size + j];
                for (int k = 0; k < j; k++) {
                    matrix->typeInfo->multiply(((void**)L->data)[i * L->size + k], ((void**)U->data)[k * U->size + j], L_elem_mul_with_U_elem);
                    matrix->typeInfo->add(sum_for_L, L_elem_mul_with_U_elem, sum_for_L);
                }
                matrix->typeInfo->negative(sum_for_L, negtive_sum_for_L);
                matrix->typeInfo->add(negtive_sum_for_L, ((void**)matrix->data)[i * matrix->size + j],  negtive_sum_for_L);
                // printf("%d", U->typeInfo->is_zero(((void**)U->data)[j * matrix->size + j]));
                if (U->typeInfo->is_zero(((void**)U->data)[j * matrix->size + j])) {
                    if (err) *err = MATRIX_ERR_DIVISION_BY_ZERO;
                    matrix->typeInfo->free(sum_for_U);
                    matrix->typeInfo->free(sum_for_L);
                    matrix->typeInfo->free(L_elem_mul_with_U_elem);
                    matrix->typeInfo->free(negtive_sum_for_U);
                    matrix->typeInfo->free(negtive_sum_for_L);
                    matrix->typeInfo->free(division_for_L);
                    return;
                }
                matrix->typeInfo->division(negtive_sum_for_L, ((void**)U->data)[j * matrix->size + j], division_for_L);
                set_element(L, i, j, division_for_L, matrix->typeInfo, err);
                if (err && *err != MATRIX_OK) {
                    matrix->typeInfo->free(sum_for_U);
                    matrix->typeInfo->free(sum_for_L);
                    matrix->typeInfo->free(L_elem_mul_with_U_elem);
                    matrix->typeInfo->free(negtive_sum_for_U);
                    matrix->typeInfo->free(negtive_sum_for_L);
                    matrix->typeInfo->free(division_for_L);
                    return;
                }
            }
        }
    }
    matrix->typeInfo->free(sum_for_U);
    matrix->typeInfo->free(sum_for_L);
    matrix->typeInfo->free(L_elem_mul_with_U_elem);
    matrix->typeInfo->free(negtive_sum_for_U);
    matrix->typeInfo->free(negtive_sum_for_L);
    matrix->typeInfo->free(division_for_L);
}