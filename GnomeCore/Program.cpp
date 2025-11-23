#include "Program.h"

Program::Program(){
	commands.emplace("type", [this]() {this -> manualInput(); });
	commands.emplace("genr", [this]() {this -> generateSequence(); });
	commands.emplace("file", [this]() {this -> fileInput(); });
	commands.emplace("exit", [this]() {this -> exitProgramm(); });
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
	originalSequence = parseSequence(userInput);
	//Сортируем
	sortedSequence = gnomeSort(originalSequence);
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
	auto allowedElements{ 150 };

	validateElementsQuantity(elements, allowedElements);

	while (true) {
		std::cout << "Введите нижнюю границу:" << std::endl;
		validateNumeric(low);
		std::cout << "Введите верхнюю границу:" << std::endl;
		validateNumeric(high);
		
		if (high < low) { std::cout << message.RANGE_ERROR << std::endl; }
		else { break; }
	}

	originalSequence = generateIntegrals(low, high, elements);
	sortedSequence = gnomeSort(originalSequence);
	std::cout << "\n~СГЕНЕРИРОВАННЫЙ МАССИВ:" << std::endl;
	output(originalSequence);
	std::cout << "---------------------" << std::endl;
	std::cout << "~РЕЗУЛЬТАТ СОРТИРОВКИ:\n" << std::endl;
	output(sortedSequence);
	clearBuffer();
}

//Файловый ввод программы
void Program::fileInput(void){
	std::string fileSequence;
	std::cout << "Введите путь для загрузки файла:" << std::endl;

	while (true) {
		std::string filePath = validateFilePath().string();
		std::ifstream fileIn(filePath);
		std::getline(fileIn, fileSequence);
		fileIn.close();

		originalSequence = parseSequence(fileSequence);
		if (originalSequence.empty()) {
			std::cout << message.ARRAY_ERROR << "Введите путь к файлу:" << std::endl;
			continue;
		}
		break;
	}

	sortedSequence = gnomeSort(originalSequence);

	std::cout << "\n~ИСХОДНЫЙ МАССИВ:" << std::endl;
	std::cout << fileSequence << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << "~РЕЗУЛЬТАТ СОРТИРОВКИ:" << std::endl;
	output(sortedSequence);

	clearBuffer();
}

void Program::writeFile(void){
	if (sortedSequence.empty()) {
		std::cout << "Массив чисел пуст!" << std::endl;
		return;
	}

	std::cout << "Сохранить результаты в файл? (Y/N):" << std::endl;
	std::getline(std::cin, userInput);

	if (userInput != "Y" && userInput != "y") {
		return;
	}

	std::filesystem::path fileToWrite;

	while (true) {
		std::cout << "Введите путь к файлу:" << std::endl;
		fileToWrite = validateFilePath();

		if (std::filesystem::exists(fileToWrite) &&
			std::filesystem::file_size(fileToWrite) > 0)
		{
			if (!checkPermission(fileToWrite, std::filesystem::perms::owner_write)) {
				std::cout << message.FILENF_ERROR << std::endl;
				continue;
			}

			std::cout << "Перезаписать файл? (Y/N):" << std::endl;
			std::getline(std::cin, userInput);

			if (userInput == "Y" || userInput == "y") {
				break;
			}
			else {
				continue;
			}
		}

		break;
	}

	saveSequencesToFile(fileToWrite);
	clearVector(originalSequence);
	clearVector(sortedSequence);

	std::cout << "==============================\n";
	std::cout << "   Сохранение завершено!      \n";
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
			writeFile();
		}

		else {
			std::cout << message.COMMAND_ERROR << std::endl;
		};
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
	std::cout << "  exit  - Выход из программы\n";;
	std::cout << "==============================\n";
}

void Program::exitProgramm(void) const{
	std::cout << "\n╔══════════════════════════════════════╗\n";
	std::cout <<   "║       👋 Заверешение работы!         ║ \n";
	std::cout <<   "╚══════════════════════════════════════╝\n";
	std::exit(EXIT_SUCCESS);  // корректное завершение с кодом 0
}

void Program::saveSequencesToFile(const std::filesystem::path& fileToWrite) const{
	std::stringstream buffer;

	auto writeVector = [&buffer](std::string_view title,
		const std::vector<int>& vec)
		{
			buffer << title << '\n';
			for (std::size_t i = 0; i < vec.size(); ++i) {
				buffer << vec[i];
				if (i + 1 < vec.size()) {
					buffer << ", ";
				}
			}
			buffer << "\n\n";
		};

	writeVector("~ИСХОДНЫЙ МАССИВ:", originalSequence);
	writeVector("~РЕЗУЛЬТАТ СОРТИРОВКИ:", sortedSequence);

	std::ofstream fileout{ fileToWrite, std::ios::trunc };
	fileout << buffer.str();
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

void validateElementsQuantity(int& elements, int maxElements){
	while (true) {
		std::cout << "Введите количество элементов (max 150):" << std::endl;
		validateNumeric(elements);

		if (elements <= 0 || elements > maxElements) {
			std::cout << "Повторите попытку:"<< std::endl;
		}
		else {
			break;
		}
	}
}

void clearVector(std::vector<int>& vec) {
	vec.clear();
	vec.shrink_to_fit();
}


