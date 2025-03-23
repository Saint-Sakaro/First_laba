#pragma once

typedef enum {
    MATRIX_OK,                        // Хорошо
    MATRIX_ERR_NULL_PTR,              // Нулевой указатель
    MATRIX_ERR_INVALID_SIZE,          // Неправильный размер
    MATRIX_ERR_INDEX_OOB,             // Выход за границы (out of bounds)
    MATRIX_ERR_ALLOC,                 // Ошибка выделения памяти
    MATRIX_ERR_TYPE_MISMATCH,         // Разные типы
    MATRIX_ERR_DIVISION_BY_ZERO,      // Деление на ноль
    MATRIX_ERR_INVALID_OPERATION,     // Ошибка операция 
} matrix_error_t;

const char* matrix_error_str(matrix_error_t err);