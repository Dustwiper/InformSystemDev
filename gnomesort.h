#ifndef GNOMESORT_H
#define GNOMESORT_H

#include <vector>

// Гномья сортировка (Gnome Sort).
// ВАЖНО: функция принимает вектор "по значению" (копию) и возвращает новый отсортированный вектор.
// Это удобно: исходный массив не меняется, пока ты сам не присвоишь результат.
std::vector<int> gnomeSort(std::vector<int> input);

#endif // GNOMESORT_H
