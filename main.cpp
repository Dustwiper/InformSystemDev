#include "mainwindow.h"

#include <QApplication>

// Точка входа в приложение.
// QApplication управляет главным циклом Qt, обработкой событий, стилями и т.д.
int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Создаём объект приложения Qt (обязателен для GUI)

    MainWindow w;               // Создаём главное окно (логин/регистрация)
    w.show();                   // Показываем окно пользователю

    return a.exec();            // Запускаем цикл обработки событий
}
