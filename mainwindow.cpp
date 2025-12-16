#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "sortwindow.h"
#include "userdb.h"

#include <QMessageBox>

// Конструктор главного окна (логин/регистрация)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Создаём элементы интерфейса из .ui

    // Подключаем кнопки к обработчикам
    connect(ui->clearButton, &QPushButton::released, this, &MainWindow::handleClearButton);
    connect(ui->registerButton, &QPushButton::released, this, &MainWindow::handleRegisterButton);
    connect(ui->enterButton, &QPushButton::released, this, &MainWindow::handleEnterButton);

    // Если пользователь начинает вводить, убираем красную рамку ошибок
    connect(ui->loginLine, &QLineEdit::textChanged, this, [this](){
        ui->loginLine->setStyleSheet(""); // Сброс стиля (убираем красный бордер)
    });
    connect(ui->passwordLine, &QLineEdit::textChanged, this, [this](){
        ui->passwordLine->setStyleSheet(""); // Сброс стиля
    });

    // Инициализируем БД при старте, чтобы сразу увидеть проблему подключения
    QString dbErr;
    if (!initDb(&dbErr)) {
        QMessageBox::warning(this, "Ошибка открытия базы данных!", dbErr);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Проверка: логин и пароль не пустые
bool MainWindow::notEmptyFields() const
{
    return !ui->loginLine->text().trimmed().isEmpty() && !ui->passwordLine->text().isEmpty();
}

// Очистить поля
void MainWindow::handleClearButton()
{
    ui->loginLine->clear();      // Удаляем текст логина
    ui->passwordLine->clear();   // Удаляем текст пароля
    ui->loginLine->setStyleSheet("");
    ui->passwordLine->setStyleSheet("");
}

// Регистрация
void MainWindow::handleRegisterButton()
{
    if (!notEmptyFields()) {
        // Подсветим пустые поля
        if (ui->loginLine->text().trimmed().isEmpty()) ui->loginLine->setStyleSheet("border: 2px solid red;");
        if (ui->passwordLine->text().isEmpty()){
            ui->passwordLine->setStyleSheet("border: 2px solid red;");
        }
        return;
    }

    QString dbErr;
    if (!initDb(&dbErr)) {
        QMessageBox::warning(this, "Ошибка подключения к БД!", dbErr);
        return;
    }

    const QString login = ui->loginLine->text().trimmed();
    const QString pass  = ui->passwordLine->text();

    long long newUserId = -1;
    QString err;
    if (!registerUser(login, pass, &newUserId, &err)) {
        QMessageBox::warning(this, "Ошибка регистрации", err.isEmpty() ? "Не удалось зарегистрировать пользователя!" : err);
        return;
    }

    QMessageBox::information(this, "OK", "Пользователь зарегистрирован.");

    //  После регистрации очищаем поля
    handleClearButton();
    ui->loginLine->setFocus();
}

// Авторизация (вход)
void MainWindow::handleEnterButton()
{
    if (!notEmptyFields()) {
        // Подсветим пустые поля
        if (ui->loginLine->text().trimmed().isEmpty()){
            ui->loginLine->setStyleSheet("border: 2px solid red;");
        }
        if (ui->passwordLine->text().isEmpty())       {
            ui->passwordLine->setStyleSheet("border: 2px solid red;");
        }
        return;
    }

    QString dbErr;
    if (!initDb(&dbErr)) {
        QMessageBox::warning(this, "Ошибка открытия БД!", dbErr);
        return;
    }

    const QString login = ui->loginLine->text().trimmed();
    const QString pass  = ui->passwordLine->text();

    // Пытаемся авторизоваться
    const AuthResult res = authUser(login, pass);

    if (!res.ok) {
        QMessageBox::warning(this, "Ошибка аутентификации", res.error.isEmpty() ? "Неверный логин или пароль!" : res.error);

        // При неудачном входе очищаем поля
        handleClearButton();
        ui->loginLine->setFocus();
        return;
    }

    // Открываем окно сортировки
    if (!m_sortWindow) {
        m_sortWindow = new SortWindow(res.userId, login);
        m_sortWindow->setAttribute(Qt::WA_DeleteOnClose, true);

        // Когда окно сортировки закроют — показываем обратно MainWindow
        connect(m_sortWindow, &QObject::destroyed, this, [this](){
            m_sortWindow = nullptr;
            this->show();
        });
    }

    m_sortWindow->show(); // Показать окно сортировки
    this->hide();         // Скрыть окно логина
}
