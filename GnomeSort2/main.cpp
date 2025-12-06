#include "Program.h"
#include <windows.h>

int main(int argc, char* argv[]) { 
	SetConsoleCP(65001);       // Ввод UTF-8
	SetConsoleOutputCP(65001);
	Program pr;
	pr.mainLoop();
}