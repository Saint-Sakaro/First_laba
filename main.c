#include "matrix.h"

#define MAX_MATRICES 10
#define MAX_LINE 256

matrix_t* matrices[MAX_MATRICES] = {NULL};

const TypeInfo* get_type_info(const char* type) {
    if (strcmp(type, "int") == 0) return get_integer_TypeInfo();
    if (strcmp(type, "double") == 0) return get_double_TypeInfo();
    if (strcmp(type, "complex") == 0) return get_complex_TypeInfo();
    return NULL;
}

void process_command(char* command, FILE* output, int* total, int* passed, int test_number) {
    char* token = strtok(command, " ");
    matrix_error_t err = MATRIX_OK;
    (*total)++;

    if (strcmp(token, "CREATE") == 0) {
        int size = atoi(strtok(NULL, " "));
        char* type = strtok(NULL, " ");
        int id = atoi(strtok(NULL, " "));
        
        matrices[id] = create_matrix(size, get_type_info(type), &err);
        if (err == MATRIX_OK) {
            (*passed)++;
        } else {
            printf("[FAIL] Test %d: CREATE %d | %s\n", test_number, id, matrix_error_str(err));
        }
    }
    else if (strcmp(token, "SET") == 0) {
        int id = atoi(strtok(NULL, " "));
        int row = atoi(strtok(NULL, " "));
        int col = atoi(strtok(NULL, " "));
        char* value = strtok(NULL, " ");

        if (!matrices[id]) {
            printf("[FAIL] Test %d: SET %d | Матрица не создана\n", test_number, id);
            return;
        }

        void* element = matrices[id]->typeInfo->alloc();
        if (matrices[id]->typeInfo == get_complex_TypeInfo()) {
            sscanf(value, "(%lf%lfi)", &((complex_t*)element)->re, &((complex_t*)element)->im);
        } else {
            sscanf(value, "%lf", (double*)element);
        }

        set_element(matrices[id], row, col, element, matrices[id]->typeInfo, &err);
        matrices[id]->typeInfo->free(element);
        
        if (err == MATRIX_OK) {
            (*passed)++;
        } else {
            printf("[FAIL] Test %d: SET %d | %s\n", test_number, id, matrix_error_str(err));
        }
    }
    else if (strcmp(token, "GET") == 0) {
        int id = atoi(strtok(NULL, " "));
        int row = atoi(strtok(NULL, " "));
        int col = atoi(strtok(NULL, " "));
        char buffer[64];
        
        if (!matrices[id]) {
            printf("[FAIL] Test %d: GET %d | Матрица не создана\n", test_number, id);
            return;
        }

        void* element = matrices[id]->typeInfo->alloc();
        get_element(matrices[id], row, col, element, matrices[id]->typeInfo, &err);
        
        if (err == MATRIX_OK) {
            matrices[id]->typeInfo->toString(element, buffer, sizeof(buffer));
            char expected[MAX_LINE];
            fgets(expected, sizeof(expected), output);
            expected[strcspn(expected, "\n")] = 0;
            
            if (strcmp(buffer, expected) == 0) {
                (*passed)++;
            } else {
                printf("[FAIL] Test %d: GET %d | Должно было быть '%s',  Получено '%s'\n", 
                      test_number, id, expected, buffer);
            }
        } else {
            printf("[FAIL] Test %d: GET %d | %s\n", test_number, id, matrix_error_str(err));
        }
        matrices[id]->typeInfo->free(element);
    }
    else if (strcmp(token, "ADD") == 0) {
        int a = atoi(strtok(NULL, " "));
        int b = atoi(strtok(NULL, " "));
        int res = atoi(strtok(NULL, " "));
        
        if (!matrices[a] || !matrices[b] || !matrices[res]) {
            printf("[FAIL] Test %d: ADD | Матрица не создана\n", test_number);
            return;
        }

        add_matrices(matrices[a], matrices[b], matrices[res], &err);
        if (err == MATRIX_OK) {
            (*passed)++;
        } else {
            printf("[FAIL] Test %d: ADD %d+%d | %s\n", 
                  test_number, a, b, matrix_error_str(err));
        }
    }
    else if (strcmp(token, "ADD_LINEAR_COMBINATION") == 0) {
        int matrix_id = atoi(strtok(NULL, " "));
        int target_row = atoi(strtok(NULL, " "));
        int count_rows = atoi(strtok(NULL, " "));
        
        int* rows = malloc(count_rows * sizeof(int));
        for(int i=0; i<count_rows; i++) {
            rows[i] = atoi(strtok(NULL, " "));
        }
        
        void* coeffs = malloc(matrices[matrix_id]->typeInfo->size * count_rows);
        char* value;
        for(int i=0; i<count_rows; i++) {
            value = strtok(NULL, " ");
            sscanf(value, "%lf", (double*)((char*)coeffs + i*matrices[matrix_id]->typeInfo->size));
        }
        
        const TypeInfo* coeffsType = get_type_info(strtok(NULL, " "));
        
        add_linear_combination(matrices[matrix_id], target_row, count_rows, rows, coeffs, coeffsType, &err);
        
        free(rows);
        free(coeffs);
        
        if(err == MATRIX_OK) (*passed)++;
        else printf("[FAIL] Test %d: ADD_LINEAR_COMBINATION | %s\n", test_number, matrix_error_str(err));
    }

    else if(strcmp(token, "SCALAR_MULTIPLY") == 0) {
        int src_id = atoi(strtok(NULL, " "));
        char* scalar_str = strtok(NULL, " ");
        char* type_str = strtok(NULL, " ");
        int res_id = atoi(strtok(NULL, " "));
        
        const TypeInfo* type = get_type_info(type_str);
        void* scalar = malloc(type->size);
        
        if (type == get_complex_TypeInfo()) {
            complex_t* c = (complex_t*)scalar;
            // Исправленный парсинг: (re+imi)
            if (sscanf(scalar_str, "(%lf+%lfi)", &c->re, &c->im) != 2) {
                printf("[FAIL] Неправильный формат комплексного скаляра\n");
                free(scalar);
                return;
            }
        } else {
            sscanf(scalar_str, "%lf", (double*)scalar);
        }
        
        scalar_multiply_matrix(matrices[src_id], scalar, type, matrices[res_id], &err);
        free(scalar);

        if(err == MATRIX_OK) (*passed)++;
        else printf("[FAIL] Test %d: SCALAR_MULTIPLY | %s\n", test_number, matrix_error_str(err));
    }
    else if(strcmp(token, "LU_DECOMPOSE") == 0) {
        int matrix_id = atoi(strtok(NULL, " "));
        int L_id = atoi(strtok(NULL, " "));
        int U_id = atoi(strtok(NULL, " "));
        
        make_LU_decomposition(matrices[matrix_id], matrices[L_id], matrices[U_id], &err);
        
        if(err == MATRIX_OK) (*passed)++;
        else printf("[FAIL] Test %d: LU_DECOMPOSE | %s\n", test_number, matrix_error_str(err));
    }
}

int main() {
    FILE* input = fopen("input.txt", "r");
    FILE* output = fopen("output.txt", "r");
    int total = 0, passed = 0, test_number = 0;
    char line[MAX_LINE];
    matrix_error_t err;

    if (!input || !output) {
        printf("Невозможно открыть файл\n");
        return 1;
    }

    while (fgets(line, sizeof(line), input)) {
        test_number++;
        line[strcspn(line, "\n")] = 0;
        process_command(line, output, &total, &passed, test_number);
    }

    // Cleanup
    for (int i = 0; i < MAX_MATRICES; i++) {
        if (matrices[i]) free_matrix(matrices[i], &err);
    }

    printf("\nРезультаты тестирования: %d/%d пройдено\n", passed, total);
    fclose(input);
    fclose(output);
    return 0;
}



// #include "matrix.h"

// void check_error(matrix_error_t received, matrix_error_t expected, const char* msg) {
//     if (received != expected) {
//         printf("[FAIL] %s | Получено: %s | Ожидалось: %s\n",
//               msg,
//               matrix_error_str(received),
//               matrix_error_str(expected));
//     } else {
//         printf("[OK] %s\n", msg);
//     }
// }

// void print_test_header(const char* title) {
//     printf("\n=== %s ===\n", title);
// }

// void test_add_matrices() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     const TypeInfo* floatType = get_float_TypeInfo();

//     print_test_header("Тест сложения матриц");

//     // 1. Корректное сложение
//     matrix_t* a = create_matrix(2, intType, &err);
//     matrix_t* b = create_matrix(2, intType, &err);
//     matrix_t* result = create_matrix(2, intType, &err);

//     int valuesA[4] = {1, 2, 3, 4};
//     int valuesB[4] = {5, 6, 7, 8};
//     for (int i = 0; i < 4; i++) {
//         set_element(a, i/2, i%2, &valuesA[i], intType, &err);
//         set_element(b, i/2, i%2, &valuesB[i], intType, &err);
//     }

//     add_matrices(a, b, result, &err);
//     check_error(err, MATRIX_OK, "Сложение корректных матриц");

//     // Проверка результата
//     int expected[4] = {6, 8, 10, 12};
//     for (int i = 0; i < 4; i++) {
//         int val;
//         get_element(result, i/2, i%2, &val, intType, &err);
//         if (val != expected[i]) {
//             printf("[FAIL] Ожидалось %d, получено %d в (%d,%d)\n", expected[i], val, i/2, i%2);
//         }
//     }

//     // 2. Несовпадение размеров
//     matrix_t* invalid_size = create_matrix(3, intType, &err);
//     add_matrices(a, invalid_size, result, &err);
//     check_error(err, MATRIX_ERR_INVALID_SIZE, "Сложение матриц разного размера");
//     free_matrix(invalid_size, &err);

//     // 3. Несовпадение типов
//     matrix_t* float_matrix = create_matrix(2, floatType, &err);
//     add_matrices(a, float_matrix, result, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Сложение int и float матриц");
//     free_matrix(float_matrix, &err);

//     // 4. NULL-матрицы
//     add_matrices(NULL, b, result, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "Сложение с NULL матрицей");

//     free_matrix(a, &err);
//     free_matrix(b, &err);
//     free_matrix(result, &err);
// }

// void test_multiply_matrices() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     const TypeInfo* floatType = get_float_TypeInfo();

//     print_test_header("Тест умножения матриц");

//     // 1. Корректное умножение
//     matrix_t* a = create_matrix(2, intType, &err);
//     matrix_t* b = create_matrix(2, intType, &err);
//     matrix_t* result = create_matrix(2, intType, &err);

//     int valuesA[4] = {1, 2, 3, 4}; // [1 2; 3 4]
//     int valuesB[4] = {5, 6, 7, 8}; // [5 6; 7 8]
//     for (int i = 0; i < 4; i++) {
//         set_element(a, i/2, i%2, &valuesA[i], intType, &err);
//         set_element(b, i/2, i%2, &valuesB[i], intType, &err);
//     }

//     multiply_matrices(a, b, result, &err);
//     check_error(err, MATRIX_OK, "Умножение корректных матриц");

//     // Проверка результата (ожидаемый результат: [19 22; 43 50])
//     int expected[4] = {19, 22, 43, 50};
//     for (int i = 0; i < 4; i++) {
//         int val;
//         get_element(result, i/2, i%2, &val, intType, &err);
//         if (val != expected[i]) {
//             printf("[FAIL] Ожидалось %d, получено %d в (%d,%d)\n", expected[i], val, i/2, i%2);
//         }
//     }

//     // 2. Несовпадение размеров
//     matrix_t* invalid_size = create_matrix(3, intType, &err);
//     multiply_matrices(a, invalid_size, result, &err);
//     check_error(err, MATRIX_ERR_INVALID_SIZE, "Умножение матриц разного размера");
//     free_matrix(invalid_size, &err);

//     // 3. Несовпадение типов
//     matrix_t* float_matrix = create_matrix(2, floatType, &err);
//     multiply_matrices(a, float_matrix, result, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Умножение int и float матриц");
//     free_matrix(float_matrix, &err);

//     // 4. NULL-матрицы
//     multiply_matrices(NULL, b, result, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "Умножение с NULL матрицей");

//     // 5. Ошибка аллокации (имитация через большую матрицу)
//     matrix_t* huge_a = create_matrix(1000, intType, &err);
//     matrix_t* huge_b = create_matrix(1000, intType, &err);
//     matrix_t* huge_result = create_matrix(1000, intType, &err);
//     if (huge_a && huge_b && huge_result) {
//         multiply_matrices(huge_a, huge_b, huge_result, &err);
//         if (err == MATRIX_OK) {
//             printf("[OK] Ошибка аллокации обработана, но моя вычеслительная способность проходит ее\n");
//         } else {
//             printf("[FAIL] Ожидалась ошибка аллокации\n");
//         }
//     }
//     free_matrix(huge_a, &err);
//     free_matrix(huge_b, &err);
//     free_matrix(huge_result, &err);

//     free_matrix(a, &err);
//     free_matrix(b, &err);
//     free_matrix(result, &err);
// }

// void test_scalar_multiply() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     const TypeInfo* floatType = get_float_TypeInfo();

//     print_test_header("Тест scalar_multiply_matrix");

//     // 1. Корректное умножение
//     matrix_t* matrix = create_matrix(2, intType, &err);
//     matrix_t* result = create_matrix(2, intType, &err);
//     int scalar = 3;

//     // Заполняем матрицу значениями [1, 2; 3, 4]
//     int values[4] = {1, 2, 3, 4};
//     for (int i = 0; i < 4; i++) {
//         set_element(matrix, i/2, i%2, &values[i], intType, &err);
//     }

//     scalar_multiply_matrix(matrix, &scalar, intType, result, &err);
//     check_error(err, MATRIX_OK, "Корректное умножение (int)");

//     // Проверяем результат: [3, 6; 9, 12]
//     int expected[4] = {3, 6, 9, 12};
//     for (int i = 0; i < 4; i++) {
//         int val;
//         get_element(result, i/2, i%2, &val, intType, &err);
//         if (val != expected[i]) {
//             printf("[FAIL] Ожидалось %d, получено %d в (%d,%d)\n", expected[i], val, i/2, i%2);
//         }
//     }

//     // 2. NULL-аргументы
//     scalar_multiply_matrix(NULL, &scalar, intType, result, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL matrix");
//     scalar_multiply_matrix(matrix, NULL, intType, result, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL scalar");
//     scalar_multiply_matrix(matrix, &scalar, NULL, result, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL valueType");
//     scalar_multiply_matrix(matrix, &scalar, intType, NULL, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL result");

//     // 3. Несовпадение размеров матриц
//     matrix_t* invalid_size = create_matrix(3, intType, &err);
//     scalar_multiply_matrix(matrix, &scalar, intType, invalid_size, &err);
//     check_error(err, MATRIX_ERR_INVALID_SIZE, "Несовпадение размеров");
//     free_matrix(invalid_size, &err);

//     // 4. Несовпадение типов (valueType != matrix->typeInfo)
//     scalar_multiply_matrix(matrix, &scalar, floatType, result, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Несовпадение valueType и типа матрицы");

//     // 5. Несовпадение типов матриц (matrix->typeInfo != result->typeInfo)
//     matrix_t* float_result = create_matrix(2, floatType, &err);
//     scalar_multiply_matrix(matrix, &scalar, intType, float_result, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Несовпадение типов матриц");
//     free_matrix(float_result, &err);

//     // 6. Умножение на ноль
//     scalar = 0;
//     scalar_multiply_matrix(matrix, &scalar, intType, result, &err);
//     check_error(err, MATRIX_OK, "Умножение на ноль");
//     for (int i = 0; i < 4; i++) {
//         int val;
//         get_element(result, i/2, i%2, &val, intType, &err);
//         if (val != 0) {
//             printf("[FAIL] Ожидался 0, получено %d в (%d,%d)\n", val, i/2, i%2);
//         }
//     }

//     free_matrix(matrix, &err);
//     free_matrix(result, &err);
// }

// void test_print_matrix() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();

//     print_test_header("Тест print_matrix");

//     // 1. Корректная матрица
//     matrix_t* matrix = create_matrix(2, intType, &err);
//     int values[4] = {1, 2, 3, 4};
//     for (int i = 0; i < 4; i++) {
//         set_element(matrix, i/2, i%2, &values[i], intType, &err);
//     }
//     printf("Ожидаемый вывод:\n[ 1 2 ]\n[ 3 4 ]\n\n");
//     printf("Фактический вывод:\n");
//     print_matrix(matrix, &err);
//     check_error(err, MATRIX_OK, "Корректная матрица");

//     // 2. NULL-матрица
//     print_matrix(NULL, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL matrix");

//     // 3. Матрица с некорректным размером
//     matrix_t* invalid_size = create_matrix(0, intType, &err); // Создаст ошибку
//     if (invalid_size) {
//         print_matrix(invalid_size, &err);
//         check_error(err, MATRIX_ERR_INVALID_SIZE, "Некорректный размер");
//         free_matrix(invalid_size, &err);
//     }

//     free_matrix(matrix, &err);
// }

// void test_add_linear_combination() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();

//     print_test_header("Тест add_linear_combination");

//     // 1. Корректный вызов
//     matrix_t* matrix = create_matrix(3, intType, &err);
//     int values[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//     for (int i = 0; i < 9; i++) {
//         set_element(matrix, i/3, i%3, &values[i], intType, &err);
//     }

//     int rows[] = {0, 1};
//     int coeffs[] = {2, -1}; // row2 = row2 + 2*row0 - 1*row1

//     add_linear_combination(matrix, 2, 2, rows, coeffs, intType, &err);
//     check_error(err, MATRIX_OK, "Корректный вызов");

//     // Проверяем строку 2:
//     // Исходная: [7, 8, 9]
//     // После: 7 + 2*1 -1*4 = 5; 8 + 2*2 -1*5 = 7; 9 + 2*3 -1*6 = 9
//     int expected[3] = {5, 7, 9};
//     for (int j = 0; j < 3; j++) {
//         int val;
//         get_element(matrix, 2, j, &val, intType, &err);
//         if (val != expected[j]) {
//             printf("[FAIL] Ожидалось %d, получено %d в (2,%d)\n", expected[j], val, j);
//         }
//     }

//     // 2. NULL-аргументы
//     add_linear_combination(NULL, 0, 2, rows, coeffs, intType, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL matrix");
//     add_linear_combination(matrix, 0, 2, NULL, coeffs, intType, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL rows");
//     add_linear_combination(matrix, 0, 2, rows, NULL, intType, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL coeffs");

//     // 3. Некорректные индексы
//     int invalid_row = 5;
//     add_linear_combination(matrix, invalid_row, 2, rows, coeffs, intType, &err);
//     check_error(err, MATRIX_ERR_INDEX_OOB, "Некорректный row");

//     int invalid_rows[] = {0, 10};
//     add_linear_combination(matrix, 2, 2, invalid_rows, coeffs, intType, &err);
//     check_error(err, MATRIX_ERR_INDEX_OOB, "Некорректный индекс в rows");

//     // 4. Несовпадение типов коэффициентов
//     const TypeInfo* floatType = get_float_TypeInfo();
//     add_linear_combination(matrix, 2, 2, rows, coeffs, floatType, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Несовпадение типов коэффициентов");

//     free_matrix(matrix, &err);
// }

// void test_matrix_to_string() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     const TypeInfo* floatType = get_float_TypeInfo();

//     print_test_header("Тест matrix_to_string");

//     // 1. Корректная матрица (int)
//     matrix_t* int_matrix = create_matrix(2, intType, &err);
//     int int_values[4] = {42, -7, 100, 0};
//     for (int i = 0; i < 4; i++) {
//         set_element(int_matrix, i / 2, i % 2, &int_values[i], intType, &err);
//     }

//     char* int_str = matrix_to_string(int_matrix, &err);
//     check_error(err, MATRIX_OK, "Корректная матрица (int)");
//     if (int_str) {
//         printf("Результат (int):\n%s", int_str);
//         free(int_str);
//     }

//     // 2. Корректная матрица (float)
//     matrix_t* float_matrix = create_matrix(2, floatType, &err);
//     float float_values[4] = {3.14f, -2.71f, 0.0f, 1.0f};
//     for (int i = 0; i < 4; i++) {
//         set_element(float_matrix, i / 2, i % 2, &float_values[i], floatType, &err);
//     }

//     char* float_str = matrix_to_string(float_matrix, &err);
//     check_error(err, MATRIX_OK, "Корректная матрица (float)");
//     if (float_str) {
//         printf("Результат (float):\n%s", float_str);
//         free(float_str);
//     }

//     // 3. NULL-матрица
//     char* null_str = matrix_to_string(NULL, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL matrix");
//     if (null_str) {
//         free(null_str);
//     }

//     // 4. Матрица с некорректным размером
//     matrix_t* invalid_size_matrix = create_matrix(0, intType, &err); // Создаст ошибку
//     if (invalid_size_matrix) {
//         char* invalid_size_str = matrix_to_string(invalid_size_matrix, &err);
//         check_error(err, MATRIX_ERR_INVALID_SIZE, "Некорректный размер");
//         if (invalid_size_str) {
//             free(invalid_size_str);
//         }
//         free_matrix(invalid_size_matrix, &err);
//     }

//     // 5. Матрица с большими числами (проверка выравнивания)
//     matrix_t* big_numbers_matrix = create_matrix(2, intType, &err);
//     int big_numbers[4] = {123456789, -987654321, 0, 42};
//     for (int i = 0; i < 4; i++) {
//         set_element(big_numbers_matrix, i / 2, i % 2, &big_numbers[i], intType, &err);
//     }

//     char* big_numbers_str = matrix_to_string(big_numbers_matrix, &err);
//     check_error(err, MATRIX_OK, "Матрица с большими числами");
//     if (big_numbers_str) {
//         printf("Результат (большие числа):\n%s", big_numbers_str);
//         free(big_numbers_str);
//     }

//     free_matrix(int_matrix, &err);
//     free_matrix(float_matrix, &err);
//     free_matrix(big_numbers_matrix, &err);
// }

// void test_LU_decomposition() {
//     matrix_error_t err;
//     const TypeInfo* doubleType = get_double_TypeInfo();

//     print_test_header("Тест LU-декомпозиции");

//     // 1. Корректное разложение
//     matrix_t* A = create_matrix(3, doubleType, &err);
//     matrix_t* L = create_matrix(3, doubleType, &err);
//     matrix_t* U = create_matrix(3, doubleType, &err);
//     matrix_t* product = create_matrix(3, doubleType, &err);

//     double dataA[9] = {2, -1, -2, 
//                       -4, 6, 3, 
//                       -4, -2, 8};
//     for (int i = 0; i < 9; i++) {
//         set_element(A, i/3, i%3, &dataA[i], doubleType, &err);
//     }

//     make_LU_decomposition(A, L, U, &err);
//     check_error(err, MATRIX_OK, "Корректное разложение");


//     // Проверка L (нижняя треугольная с единицами на диагонали)
//     double expected_L[9] = {1, 0, 0,
//                            -2, 1, 0,
//                            -2, -1, 1};
//     for (int i = 0; i < 9; i++) {
//         double val;
//         get_element(L, i/3, i%3, &val, doubleType, &err);
//         if (fabs(val - expected_L[i]) > 1e-6) {
//             printf("[FAIL] L[%d][%d] Ожидалось %f, получено %f\n", i/3, i%3, expected_L[i], val);
//         }
//     }

//     // Проверка U (верхняя треугольная)
//     double expected_U[9] = {2, -1, -2,
//                             0, 4, -1,
//                             0, 0, 3};
//     for (int i = 0; i < 9; i++) {
//         double val;
//         get_element(U, i/3, i%3, &val, doubleType, &err);
//         if (fabs(val - expected_U[i]) > 1e-6) {
//             printf("[FAIL] U[%d][%d] Ожидалось %f, получено %f\n", i/3, i%3, expected_U[i], val);
//         }
//     }

//     // Проверка L * U = A
//     multiply_matrices(L, U, product, &err);
//     for (int i = 0; i < 9; i++) {
//         double val;
//         get_element(product, i/3, i%3, &val, doubleType, &err);
//         if (fabs(val - dataA[i]) > 1e-6) {
//             printf("[FAIL] L*U[%d][%d] Ожидалось %f, получено %f\n", i/3, i%3, dataA[i], val);
//         }
//     }

//     // 2. Некорректные аргументы
//     make_LU_decomposition(NULL, L, U, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL matrix");
//     make_LU_decomposition(A, NULL, U, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL L");
//     make_LU_decomposition(A, L, NULL, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "NULL U");

//     // 3. Несовпадение размеров
//     matrix_t* invalid_size = create_matrix(2, doubleType, &err);
//     make_LU_decomposition(A, invalid_size, U, &err);
//     check_error(err, MATRIX_ERR_INVALID_SIZE, "Несовпадение размеров");
//     free_matrix(invalid_size, &err);

//     // 4. Несовпадение типов
//     const TypeInfo* intType = get_integer_TypeInfo();
//     matrix_t* int_matrix = create_matrix(3, intType, &err);
//     make_LU_decomposition(A, int_matrix, U, &err);
//     check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Несовпадение типов");
//     free_matrix(int_matrix, &err);

//     // 5. Деление на ноль (вырожденная матрица)
//     double singular_data[9] = {1, 2, 3, 
//                               2, 4, 6, 
//                               7, 8, 9};
//     matrix_t* singular_A = create_matrix(3, doubleType, &err);
//     for (int i = 0; i < 9; i++) {
//         set_element(singular_A, i/3, i%3, &singular_data[i], doubleType, &err);
//     }
//     make_LU_decomposition(singular_A, L, U, &err);
//     check_error(err, MATRIX_ERR_DIVISION_BY_ZERO, "Деление на ноль");
//     free_matrix(singular_A, &err);
//     free_matrix(A, &err);
//     free_matrix(L, &err);
//     free_matrix(U, &err);
//     free_matrix(product, &err);
// }

// void test_complex_operations() {
//     matrix_error_t err;
//     const TypeInfo* complexType = get_complex_TypeInfo();

//     print_test_header("Тесты для комплексных чисел");

//     // 1. Создание и освобождение комплексной матрицы
//     matrix_t* complex_matrix = create_matrix(2, complexType, &err);
//     check_error(err, MATRIX_OK, "Создание комплексной матрицы 2x2");
//     if (complex_matrix) {
//         free_matrix(complex_matrix, &err);
//         check_error(err, MATRIX_OK, "Освобождение комплексной матрицы");
//     }

//     // 2. Запись и чтение комплексных элементов
//     complex_t c1 = {3.0, 4.0}; // 3 + 4i
//     complex_t c2 = {1.0, -2.0}; // 1 - 2i
    
//     matrix_t* m = create_matrix(2, complexType, &err);
//     set_element(m, 0, 0, &c1, complexType, &err);
//     set_element(m, 0, 1, &c2, complexType, &err);
//     check_error(err, MATRIX_OK, "Запись комплексных элементов");

//     complex_t read_val;
//     get_element(m, 0, 0, &read_val, complexType, &err);
//     if (fabs(read_val.re - 3.0) < 1e-6 && fabs(read_val.im - 4.0) < 1e-6) {
//         printf("[OK] Корректное чтение (3+4i)\n");
//     } else {
//         printf("[FAIL] Ожидалось (3+4i), получено (%lf + %lfi)\n", read_val.re, read_val.im);
//     }

//     // 3. Сложение комплексных матриц
//     matrix_t* a = create_matrix(2, complexType, &err);
//     matrix_t* b = create_matrix(2, complexType, &err);
//     matrix_t* result = create_matrix(2, complexType, &err);

//     complex_t a_data[4] = {{1,2}, {3,4}, {5,6}, {7,8}};
//     complex_t b_data[4] = {{8,7}, {6,5}, {4,3}, {2,1}};
    
//     for (int i = 0; i < 4; i++) {
//         set_element(a, i/2, i%2, &a_data[i], complexType, &err);
//         set_element(b, i/2, i%2, &b_data[i], complexType, &err);
//     }

//     add_matrices(a, b, result, &err);
//     check_error(err, MATRIX_OK, "Сложение комплексных матриц");

//     // Проверка результата (9+9i, 9+9i, 9+9i, 9+9i)
//     complex_t expected_add[4] = {{9,9}, {9,9}, {9,9}, {9,9}};
//     for (int i = 0; i < 4; i++) {
//         complex_t val;
//         get_element(result, i/2, i%2, &val, complexType, &err);
//         if (fabs(val.re - expected_add[i].re) > 1e-6 || fabs(val.im - expected_add[i].im) > 1e-6) {
//             printf("[FAIL] Ожидалось (%lf + %lfi), получено (%lf + %lfi) в (%d,%d)\n",
//                 expected_add[i].re, expected_add[i].im, val.re, val.im, i/2, i%2);
//         }
//     }

//     // 4. Умножение комплексных матриц
//     multiply_matrices(a, b, result, &err);
//     check_error(err, MATRIX_OK, "Умножение комплексных матриц");

//     // 5. LU-декомпозиция для комплексных матриц
//     matrix_t* L = create_matrix(2, complexType, &err);
//     matrix_t* U = create_matrix(2, complexType, &err);
//     make_LU_decomposition(a, L, U, &err);
//     check_error(err, MATRIX_OK, "LU-разложение для комплексных матриц");

//     // 6. Вывод матрицы
//     printf("Комплексная матрица:\n");
//     print_matrix(a, &err);
//     char* str = matrix_to_string(a, &err);
//     if (str) {
//         printf("Строковое представление:\n%s", str);
//         free(str);
//     }

//     // Освобождение ресурсов
//     free_matrix(m, &err);
//     free_matrix(a, &err);
//     free_matrix(b, &err);
//     free_matrix(result, &err);
//     free_matrix(L, &err);
//     free_matrix(U, &err);
// }

// int main() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     const TypeInfo* floatType = get_float_TypeInfo();

//     // 1. Тест создания и освобождения
//     print_test_header("Тест создания/освобождения");
//     matrix_t* m = create_matrix(3, intType, &err);
//     check_error(err, MATRIX_OK, "Создание матрицы 3x3 int");
//     if (m) {
//         free_matrix(m, &err);
//         check_error(err, MATRIX_OK, "Освобождение матрицы");
//     }

//     // 2. Тест set_element/get_element с корректным типом
//     print_test_header("Тест корректных операций");
//     m = create_matrix(2, intType, &err);
//     if (m) {
//         int val = 42;
        
//         // Корректная запись
//         set_element(m, 0, 0, &val, intType, &err);
//         check_error(err, MATRIX_OK, "Запись int в (0,0)");

//         // Корректное чтение
//         int result;
//         get_element(m, 0, 0, &result, intType, &err);
//         check_error(err, MATRIX_OK, "Чтение int из (0,0)");
//         if (result == 42) {
//             printf("[OK] Значение корректно: %d\n", result);
//         } else {
//             printf("[FAIL] Ожидалось 42, получено %d\n", result);
//         }

//         free_matrix(m, &err);
//     }

//     // 3. Тест несоответствия типов
//     print_test_header("Тест несоответствия типов");
//     m = create_matrix(2, intType, &err);
//     if (m) {
//         float float_val = 3.14f;
        
//         // Попытка записать float в int-матрицу
//         set_element(m, 1, 1, &float_val, floatType, &err);
//         check_error(err, MATRIX_ERR_TYPE_MISMATCH, "Запись float в int-матрицу");

//         free_matrix(m, &err);
//     }

//     // 4. Тест некорректных индексов
//     print_test_header("Тест некорректных индексов");
//     m = create_matrix(2, intType, &err);
//     if (m) {
//         int val = 100;
        
//         // Выход за границы
//         set_element(m, 5, 5, &val, intType, &err);
//         check_error(err, MATRIX_ERR_INDEX_OOB, "set_element(5,5)");

//         get_element(m, -1, 0, &val, intType, &err);
//         check_error(err, MATRIX_ERR_INDEX_OOB, "get_element(-1,0)");

//         free_matrix(m, &err);
//     }

//     // 5. Тест нулевых указателей
//     print_test_header("Тест нулевых указателей");
//     set_element(NULL, 0, 0, &(int){1}, intType, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "set_element(NULL matrix)");

//     int val;
//     get_element(NULL, 0, 0, &val, intType, &err);
//     check_error(err, MATRIX_ERR_NULL_PTR, "get_element(NULL matrix)");

//     // 6. Сложение матриц
//     test_add_matrices();

//     // 7. Умножение матриц
//     test_multiply_matrices();

//     // 8. Умножение на скаляр
//     test_scalar_multiply();

//     // 9. Умножение на скаляр
//     test_print_matrix();

//     // 10. Линейная комбинация
//     test_add_linear_combination();

//     // 11. Линейная комбинация
//     test_matrix_to_string();

//     // 12. LU разложение 
//     test_LU_decomposition();

//     // 13. Тесты для комплексных чисел
//     test_complex_operations();

//     return 0;
// }










































// int main() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     matrix_t* a = create_matrix(2, intType, &err);
//     matrix_t* b = create_matrix(2, intType, &err);
//     matrix_t* result = create_matrix(2, intType, &err);

//     int valuesA[4] = {1, 2, 3, 4};
//     int valuesB[4] = {5, 6, 7, 8};
//     for (int i = 0; i < 2; i++) {
//         for (int j = 0; j < 2; j++) {
//             set_element(a, i, j, &valuesA[i * 2 + j]);
//             set_element(b, i, j, &valuesB[i * 2 + j]);
//         }
//     }

//     printf("Матрица A:\n");
//     print_matrix(a);
//     printf("Матрица B:\n");
//     print_matrix(b);

//     add_matrices(a, b, result);
//     printf("A + B:\n");
//     print_matrix(result);

//     multiply_matrices(a, b, result);
//     printf("A * B:\n");
//     print_matrix(result);

//     int scalar = 2;
//     scalar_multiply_matrix(a, &scalar, result);
//     printf("A * 2:\n");
//     print_matrix(result);

//     int value;
//     get_element(a, 1, 1, &value);
//     printf("Элемент a[1][1]: %d\n", value);

//     const int source_rows[] = {0};
//     const int coeffs[] = {2};

//     add_linear_combination(a, 1, 1, source_rows, coeffs);
//     print_matrix(a);

    
//     int el = 400000000;
//     set_element(a, 0, 0, &el);
//     set_element(a, 0, 1, &el);

//     char* str = matrix_to_string(a);
//     printf("Матрица A:\n%s", str);




//     free_matrix(a, &err);
//     free_matrix(b, &err);
//     free_matrix(result, &err);

//     return 0;
// }






// Определение MAX_MATRIX_SIZE должно совпадать с тем, что в matrix.c
// #define MAX_MATRIX_SIZE 1024

// void test_valid_creation() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();
//     matrix_t* matrix = create_matrix(2, intType, &err);

//     if (matrix && err == MATRIX_OK) {
//         printf("[OK] Матрица 2x2 создана успешно.\n");
//         free_matrix(matrix);
//     } else {
//         printf("[FAIL] Ошибка при создании матрицы 2x2: %s\n", matrix_error_str(err));
//     }
// }

// void test_invalid_size() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();

//     // Отрицательный размер
//     matrix_t* matrix = create_matrix(-5, intType, &err);
//     if (!matrix && err == MATRIX_ERR_INVALID_SIZE) {
//         printf("[OK] Некорректный размер (-5) обработан правильно.\n");
//     } else {
//         printf("[FAIL] Ожидалась ошибка MATRIX_ERR_INVALID_SIZE.\n");
//     }

//     // Слишком большой размер
//     matrix = create_matrix(MAX_MATRIX_SIZE + 1, intType, &err);
//     if (!matrix && err == MATRIX_ERR_INVALID_SIZE) {
//         printf("[OK] Превышение MAX_MATRIX_SIZE обработано правильно.\n");
//     } else {
//         printf("[FAIL] Ожидалась ошибка MATRIX_ERR_INVALID_SIZE.\n");
//     }
// }

// void test_null_typeinfo() {
//     matrix_error_t err;
//     matrix_t* matrix = create_matrix(2, NULL, &err);

//     if (!matrix && err == MATRIX_ERR_NULL_PTR) {
//         printf("[OK] Передача NULL в typeInfo обработана правильно.\n");
//     } else {
//         printf("[FAIL] Ожидалась ошибка MATRIX_ERR_NULL_PTR.\n");
//     }
// }

// void test_memory_allocation_failure() {
//     matrix_error_t err;
//     const TypeInfo* intType = get_integer_TypeInfo();

//     // Пытаемся создать матрицу 10000x10000 (более реалистичный тест)
//     matrix_t* matrix = create_matrix(1000, intType, &err);

//     if (!matrix && err == MATRIX_ERR_ALLOC) {
//         printf("[OK] Ошибка выделения памяти обработана правильно.\n");
//     } else {
//         printf("[FAIL] Ожидалась ошибка MATRIX_ERR_ALLOC. Получено: %s\n", matrix_error_str(err));
//         if (matrix) free_matrix(matrix);
//     }
// }

// int main() {
//     printf("=== Тест создания матрицы ===\n");
//     test_valid_creation();
//     test_invalid_size();
//     test_null_typeinfo();
//     test_memory_allocation_failure();

//     return 0;
// }

