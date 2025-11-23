#pragma once
#include <cstdlib>
#include <functional>
#include <random>
#include <string>
#include <unordered_map> 


#include "CheckPoint.h"
#include "GnomeSort.h"
#include "GnomeSortTest.h"

class Program {
private:
	//Словарь[название комманды] : [функция]
	std::unordered_map<std::string, std::function<void()>> commands;
	bool isRunning;
	std::string userInput;
	std::vector<int> sortedSequence;
	std::vector<int> originalSequence;
public:
	Program();
	explicit Program(bool process) : isRunning {process} {}; // явный конструктор для запуска программмы
	void manualInput(void);
	void generateSequence(void);
	void fileInput(void);
	void writeFile(void);
	void mainLoop(void);
	void showMenu(void) const;
	void exitProgramm(void) const;
	void saveSequencesToFile(const std::filesystem::path& fileToWrite) const;
};

//Non-member functions
std::vector<int> parseSequence(std::string& lineToParse);
std::vector<int> generateIntegrals(const int& min, const int& max, const int& quantity);
void validateElementsQuantity(int& elements, int maxElements);
void clearVector(std::vector<int>& vec);
bool validateFileSequence(const std::string& line);

template <typename T>
void output(std::vector<T>& sequence) {
	auto last = sequence.end();
	for (auto it = sequence.begin(); it < last - 1; ++it) {
		std::cout << *it << ", ";
	}
	std::cout << *(last - 1) << ";" << std::endl;
}

