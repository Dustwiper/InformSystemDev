#include "Program.h"


Program::Program(){
	commands.emplace("type", [this]() {this -> manualInput(); });
	commands.emplace("genr", [this]() {this -> generateSequence(); });
	commands.emplace("file", [this]() {this -> fileInput(); });
	commands.emplace("exit", [this]() {this -> exitProgramm(); });
	commands.emplace("test", [this]() {this -> runTests(); });
	isRunning = true;
}

// Ввод с клавиатуры
void Program::manualInput(void){
	std::cout << "Введите последовательность чисел для сортировки через запятую и пробел:" << std::endl;
	std::getline(std::cin, userInput);

	while (!matchRegExpr(regExpr::arrayPattern, userInput)) {
		std::cout << message.ARRAY_ERROR << " Повторите попытку:" << std::endl;
		std::getline(std::cin, userInput);
	}
	//Формируем вектор чисел
	sortedSequence = parseSequence(userInput);
	//Сортируем
	sortedSequence = gnomeSort(sortedSequence);
	std::cout << "---------------------" << std::endl;
	std::cout << "~РЕЗУЛЬТАТ СОРТИРОВКИ:" << std::endl;
	output(sortedSequence);
	std::cout << '\n';

	clearBuffer();
	return;
}

// Генерация последовательности
void Program::generateSequence(void) {
	auto low{ 0 }; auto high{ 0 };
	auto elements{ 0 };
	auto allowedElements{ 15 };

	std::cout << "Введите количество элементов:" << std::endl;
	validateNumeric(elements);

	while (true) {
		std::cout << "Введите нижнюю границу:" << std::endl;
		validateNumeric(low);
		std::cout << "Введите верхнюю границу:" << std::endl;
		validateNumeric(high);
		
		if (high < low) { std::cout << message.RANGE_ERROR << std::endl; }
		else { break; }

	}

	std::vector genericSequence = generateIntegrals(low, high, elements);
	sortedSequence = gnomeSort(genericSequence);
	std::cout << "\n~СГЕНЕРИРОВАННЫЙ МАССИВ:" << std::endl;
	(elements > allowedElements) ? output(genericSequence, allowedElements) : output(genericSequence);
	std::cout << "---------------------" << std::endl;
	std::cout << "~РЕЗУЛЬТАТ СОРТИРОВКИ:\n" << std::endl;
	(elements > allowedElements) ? output(sortedSequence) : output(sortedSequence);

	clearBuffer();
}

//Файловый ввод программы
void Program::fileInput(void){
	std::string fileSequence;
	std::cout << "Введите путь для загрузки файла:" << std::endl;

	std::string filePath = validateFilePath().string();
	std::ifstream fileIn(filePath);
	std::getline(fileIn, fileSequence);
	fileIn.close();

	while (!matchRegExpr(regExpr::arrayPattern, fileSequence)) {
		std::cout << message.ARRAY_ERROR << "Введите путь к файлу:" << std::endl;
		filePath = validateFilePath().string();
		fileIn.open(filePath);
		std::getline(fileIn, fileSequence);
		fileIn.close();
	}
	
	sortedSequence = parseSequence(fileSequence);
	sortedSequence = gnomeSort(sortedSequence);

	std::cout << "\n~ИСХОДНЫЙ МАССИВ:" << std::endl;
	std::cout << fileSequence << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << "~РЕЗУЛЬТАТ СОРТИРОВКИ:\n" << std::endl;
	output(sortedSequence);

	clearBuffer();

	
}

void Program::writeFile(void){
	std::cout << "Сохранить результаты в файл? (Y/N):" << std::endl;
	std::getline(std::cin, userInput);
	
	if (sortedSequence.empty()) { std::cout << "Массив чисел пуст!" << std::endl; return; }

	if (userInput == "Y" || userInput == "y") {
		std::cout << "Введите путь к файлу:" << std::endl;
		std::filesystem::path fileToWrite = validateFilePath();


		while (std::filesystem::file_size(fileToWrite) > 0) {

			if (!checkPermission(fileToWrite,
				std::filesystem::perms::owner_write)) {
				std::cout << message.FILENF_ERROR << std::endl;
				std::cout << "Введите путь к файлу:" << std::endl;
				fileToWrite = validateFilePath();
			}

			std::cout << "Перезаписать файл? (Y/N):" << std::endl;
			std::getline(std::cin, userInput);
			if (userInput == "Y" || userInput == "y") { break; }

			else {
				std::cout << "Введите путь к файлу:" << std::endl;
				fileToWrite = validateFilePath();
			}
		}

		std::ofstream fileout{ fileToWrite };

		for (auto it = sortedSequence.begin(); it != sortedSequence.end(); ++it) {
			if (it == (sortedSequence.end() - 1)) {
				fileout << *it;
				break;
			}
			fileout << *it << ", ";
		}

		fileout.close();
	}

	else { return;}

	std::cout << "==============================\n";
	std::cout << "   ✅ Сохранение завершено!   \n";
	std::cout << "==============================\n";

}


// Основной цикл программы
void Program::mainLoop(void){
	showMenu();
	while (isRunning) {
		std::cout << "Введите команду: " << std::endl;
		assignString(userInput);
		if (commands.contains(userInput)) {
			commands.at(userInput)();
			if(userInput == "test"){}
			else {
				writeFile();
			}
		}

		else std::cout << message.COMMAND_ERROR << std::endl;
		}

}

void Program::showMenu(void) const{
	std::cout << "\n==============================\n";
	std::cout << "        МЕНЮ ПРОГРАММЫ        \n";
	std::cout << "==============================\n";
	std::cout << "Выберите действие:\n\n";
	std::cout << "  type  - Ручной ввод чисел\n";
	std::cout << "  genr  - Сгенерировать случайную последовательность\n";
	std::cout << "  file  - Загрузить числа из файла\n";
	std::cout << "  exit  - Выход из программы\n";
	std::cout << "  test  - Запуск тестов\n";
	std::cout << "==============================\n";
}

void Program::exitProgramm(void) const{
	std::cout << "\n╔══════════════════════════════════════╗\n";
	std::cout <<   "║       👋 Заверешение работы!         ║ \n";
	std::cout <<   "╚══════════════════════════════════════╝\n";
	std::exit(EXIT_SUCCESS);  // корректное завершение с кодом 0
}

void Program::runTests(void) const{
	runAllTests();
}




//Non-member functions:
std::vector<int> parseSequence(std::string& lineToParse) {
	std::vector<int> valuesToSort; // Вектор со значениями для сортировки
	int parsedValue{ 0 }; // Переменная для временного хранения взятого значения
	auto end = std::sregex_iterator{}; // Конструктор по умолчанию создаёт итератор на конец последовательности
	// Цикл по всей строке 
	for (auto it = std::sregex_iterator{ std::begin(lineToParse),
										 std::end(lineToParse), regExpr::intPattern }; it != end; ++it) {
		parsedValue = std::stoi((*it)[0]);
		valuesToSort.push_back(parsedValue);
	}
	return valuesToSort;
}

std::vector<int> generateIntegrals(const int& min, const int& max, const int& quantity){
	std::vector<int> vec;
	std::random_device rd{};//Генератор псевдо-случайных недетерминированных чисел; генерирует одно случайное 
	// которое будет использовано как seeed
	// Инициализируем генератор начальным числом из rd
	auto mtgen = std::mt19937{ rd() };//Генератор чисел, использующий Вихрь Мерсенна. Самый эффективны ГПCЧ ->
	// Производим равномерно распределённое целое число на заданном интервале.
	auto ud = std::uniform_int_distribution<>(min, max);

	for (auto i = 0; i < quantity; ++i) {
		vec.push_back(ud(mtgen));
	}

	return vec;
}


