#pragma once
/***************************
 *Модуль различных проверок*
****************************/

#include <string>
#include <iostream>
#include <optional>
#include <filesystem>
#include <fstream>
#include "regex.h"


struct errorMessage {
	std::string NUM_ERROR{"Неверный формат ввода! Повторите попытку:"};
	std::string STR_ERROR{ "Неверный формат строки! Повторите попытку:" };
	std::string RANGE_ERROR{ "Неверные границы диапазона!" };
	std::string FILENF_ERROR{ "Файл не доступен или не найден!"};	
	std::string ARRAY_ERROR{ "Неверный формат последовательности! " };
	std::string COMMAND_ERROR{ "Комманда не найдена!" };

};

extern errorMessage message;

bool inline matchRegExpr(const std::regex& expr, const std::string& line);
void clearBuffer(void);
void assignString(std::string& strValue);

//функция проверки различного типа ввода
int validateNumeric();

std::filesystem::path validateFilePath(void);
auto getPermissions(const std::filesystem::path& filePath) -> std::filesystem::perms;
auto checkPermission(const std::filesystem::path& filePath, std::filesystem::perms reqOption) -> bool;