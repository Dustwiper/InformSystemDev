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
	void runTests(void) const;

};

//Non-member functions
std::vector<int> parseSequence(std::string& lineToParse);
std::vector<int> generateIntegrals(const int& min, const int& max, const int& quantity);

template <typename T>
void output(std::vector<T>& sequence) {
	auto last = sequence.end();
	for (auto it = sequence.begin(); it < last - 1; ++it) {
		std::cout << *it << ", ";
	}
	std::cout << *(last - 1) << ";" << std::endl;
}

template <typename T>
void output(std::vector<T>& sequence, int size) {
	for (auto i{ 0 }; i < size; ++i) {
		std::cout << sequence[i] << ", ";
	}
	std::cout << sequence[size] << " ..." << std::endl;
}
