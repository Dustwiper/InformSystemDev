/*******************
 *Gnome Sort Module*
 *******************/

#include "GnomeSort.h"

/*******************************************************
 *Precondition: Корректно составленный массив чисел.   *
 *Postcondition: Корректно отсортированный массив чисел*
 *******************************************************/ 

std::vector<int> gnomeSort(std::vector<int> array) {
	auto size = array.size(); // получаем размер массива
	auto current_index = 1; // начинаем проверять со второго элемента
	while (current_index != size) {
		//увеличиваем индекс, если элементы стоят в правильном порядке
		if (array[current_index] >= array[current_index - 1]) ++current_index; 

		else {
			//меняем местами элементы
			std::swap(array[current_index - 1], array[current_index]);
			// отходим на шаг назад
			if (current_index > 1) current_index -= 1;
		}
	}
	return array;
}
