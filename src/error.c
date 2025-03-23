#include "error.h"

static const char* ERROR_STRINGS[] = {
    "Хорошо",                        // MATRIX_OK
    "Обнаружен нулевой указатель",   // MATRIX_ERR_NULL_PTR
    "Некорректный размер матрицы",   // MATRIX_ERR_INVALID_SIZE
    "Индекс выходит за границы",     // MATRIX_ERR_INDEX_OOB
    "Ошибка выделения памяти",       // MATRIX_ERR_ALLOC
    "Несовпадение типов",            // MATRIX_ERR_TYPE_MISMATCH
    "Ошибка деления на ноль",        // MATRIX_ERR_DIVISION_BY_ZERO
    "Недопустимая операция",         // MATRIX_ERR_INVALID_OPERATION
};

const char* matrix_error_str(matrix_error_t err) {
    if (err < 0 || err > MATRIX_ERR_INVALID_OPERATION) {
        return "Неизвестная ошибка";
    }
    return ERROR_STRINGS[err];
}