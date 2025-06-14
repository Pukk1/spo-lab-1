//
// Created by Ivan on 09.05.2025.
//

#include "list_util.h"

const int START_ARRAY_SIZE = 8;

// добавление в массив с возможным динамическим расширением
void addToList(List *currentArray, void *element) {
    void **nodes;
    if (currentArray->capacity != currentArray->size) {
        nodes = currentArray->elements;
    } else {
        nodes = malloc(sizeof(void *) * 2 * currentArray->size);
        for (int i = 0; i < currentArray->size; ++i) {
            nodes[i] = currentArray->elements[i];
        }
        free(currentArray->elements);
        currentArray->elements = nodes;
    }
    nodes[currentArray->size] = element;
    currentArray->size += 1;
}

List *mallocEmptyList() {
    void** nodes = malloc(sizeof(void *) * START_ARRAY_SIZE);
    List *list = malloc(sizeof(List));
    list->size = 0;
    list->capacity = START_ARRAY_SIZE;
    list->elements = nodes;
    return list;
}
