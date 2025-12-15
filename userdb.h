#ifndef USERDB_H
#define USERDB_H

#include <QString>
#include <QtSql/QSqlDatabase>

// Результат авторизации
struct AuthResult {
    bool ok = false;          // true, если логин/пароль верные
    long long userId = -1;    // id пользователя из таблицы user
    QString error;            // текст ошибки (если нужна детализация)
};

// Инициализация базы данных:
// - открываем соединение QMYSQL
// - создаём таблицы, если их нет
bool initDb(QString *err = nullptr);

// Возвращаем экземпляр соединения по имени (qt_mysql_default)
QSqlDatabase dbInstance();

// Регистрация пользователя (login уникален)
bool registerUser(const QString &login,
                  const QString &password,
                  long long *newUserId,
                  QString *err = nullptr);

// Авторизация пользователя
AuthResult authUser(const QString &login, const QString &password);

#endif // USERDB_H
