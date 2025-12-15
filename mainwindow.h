#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SortWindow;

// Главное окно: регистрация/авторизация.
// После успешного входа открывается окно SortWindow.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleClearButton();     // Очистить поля логина/пароля
    void handleEnterButton();     // Войти (авторизация)
    void handleRegisterButton();  // Регистрация

private:
    Ui::MainWindow *ui = nullptr;        // Сгенерированный класс UI из mainwindow.ui
    QPointer<SortWindow> m_sortWindow;   // Окно сортировки (после входа)

    bool notEmptyFields() const;         // Проверка заполненности полей
};

#endif // MAINWINDOW_H
