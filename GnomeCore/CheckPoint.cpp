#include "CheckPoint.h"


errorMessage message;

//Функция проверки формата различного ввода
bool inline matchRegExpr(const std::regex& expr, const std::string& line){
	return std::regex_match(line, expr);
}

//Цикл обработки некорректного формата строки
void assignString(std::string& strValue) {
	std::getline(std::cin, strValue);
	while (matchRegExpr(regExpr::strPattern, strValue) == false) {
		std::cout << message.STR_ERROR << std::endl;
		std::getline(std::cin, strValue);
	}
}

//Отчистка буфера
void clearBuffer(void){
	if (std::cin.rdbuf()->in_avail() > 0) {  // есть ли что-то во входном буфере
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	std::cin.clear(); 
}

// Проверка корректности введёного пути
std::filesystem::path validateFilePath(void) {
	std::string filePath;
	std::getline(std::cin, filePath);
	std::filesystem::directory_entry fileObject{ filePath };

	if (!fileObject.exists()) {
		std::ofstream newFile(filePath);
		newFile.close();
	}

	// Проверка бита на запрета на чтение и существование файла
	while (!fileObject.is_regular_file())
	{
		std::cout << message.FILENF_ERROR << " Повторите попытку: " << std::endl;
		std::getline(std::cin, filePath);
		fileObject.assign(filePath);
	}

	return filePath;
}

int validateNumeric() {
	std::string variable;

	while (true) {
		std::cin >> variable;

		if (matchRegExpr(regExpr::intPattern, variable)) {
			clearBuffer();
			return std::stoi(variable);
		}

		std::cout << message.NUM_ERROR << std::endl;
		clearBuffer();

	}
}

auto getPermissions(const std::filesystem::path& filePath) -> std::filesystem::perms{
	std::filesystem::file_status fs{std::filesystem::status(filePath)};
	return fs.permissions();
}

auto checkPermission(const std::filesystem::path& filePath, std::filesystem::perms reqOption) -> bool{
	return ((getPermissions(filePath) & reqOption) != std::filesystem::perms::none);
}




