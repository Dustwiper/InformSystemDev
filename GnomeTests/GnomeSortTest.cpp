/*****************************
 *Unit Tests Module by Catch2*
 * For proper test information run the programm with "-s" comand line argument
 *****************************/

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main()

#include "GnomeSortTest.h"
#include "GnomeSort.h"
#include "catch.hpp"

//Тестовый класс
class TestArray {
private:
	std::vector<int> testArray_; 
public:
	TestArray(const std::vector<int> initVec) : testArray_(initVec) {};

	// Метод сортировки массива средствами std для сравнения
	auto sortArray() -> const std::vector<int>& { 
		std::sort(this->testArray_.begin(), testArray_.end());
		return testArray_;
	}
	
	// Метод получения вектора
	const std::vector<int>& getArray() { return testArray_; }

	// Освобождаем память для вектора
	~TestArray() { 
	testArray_.clear();
	testArray_.shrink_to_fit();
	}
};

/*Секция тестов*/
SCENARIO("Basic array sorting tests") {

	GIVEN("Sorting function 'gnomeSort'") {

		WHEN("Argument is a vector of unordered numbers") {
			TestArray array1({ 73, 4, 91, 25, 66, 13, 88, 59});

			THEN("gnomeSort(array) = {4, 13, 25, 59, 66, 73, 88, 91}") {

				REQUIRE(gnomeSort(array1.getArray()) == array1.sortArray());
			}
		}
		WHEN("Argument is a vector of negative numbers") {
			TestArray array2({ -3, -17, -42, -8, -99, -56, -24, -71 });

			THEN("gnomeSort(array) = {-99, -71, -56, -42, -24, -17, -8, -3}") {

				REQUIRE(gnomeSort(array2.getArray()) == array2.sortArray());
			}
		}
		WHEN("Argument contains both negative and positive numbers") {
			TestArray array3({ -73, 42, 0, -18, 67, -95, 23, -4, 89, -31 });

			THEN("gnomeSort(array) = {-95, -73, -31, -18, -4, 0, 23, 42, 67, 89}") {

				REQUIRE(gnomeSort(array3.getArray()) == array3.sortArray());
			}
		}
	}		
}

int main(int argc, char* argv[]) {
	Catch::Session session;

	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0) {
		return returnCode; 
	}

	return session.run();
}