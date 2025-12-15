#include "userdb.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>

// Имя соединения в Qt, чтобы везде обращаться одинаково
static const char* kConnName = "qt_mysql_default";

// Параметры подключения к MySQL.
// ВАЖНО: здесь должны быть твои реальные параметры подключения.
static const char* kHost = "127.0.0.1";       // Хост (обычно localhost/127.0.0.1)
static const int   kPort = 3306;              // Порт MySQL
static const char* kDbName = "gnomesort_db";  // Название базы данных
static const char* kUser = "root";            // Пользователь MySQL
static const char* kPass = "root22860";       // Пароль MySQL

// Получить объект соединения (если уже создан — просто вернёт его)
QSqlDatabase dbInstance()
{
    if (QSqlDatabase::contains(kConnName)) {
        return QSqlDatabase::database(kConnName);
    }

    // Создаём новое подключение к MySQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", kConnName);
    db.setHostName(kHost);           // Указываем хост
    db.setPort(kPort);               // Указываем порт
    db.setDatabaseName(kDbName);     // Указываем имя базы
    db.setUserName(kUser);           // Логин
    db.setPassword(kPass);           // Пароль

    return db;
}

// Создание таблиц, если их нет
static bool ensureSchema(QSqlDatabase db, QString *err)
{
    // Таблица пользователей
    {
        QSqlQuery q(db);
        if (!q.exec(R"(
            CREATE TABLE IF NOT EXISTS user (
                id BIGINT PRIMARY KEY AUTO_INCREMENT,
                username VARCHAR(64) NOT NULL UNIQUE,
                password VARCHAR(128) NOT NULL
            )
        )")) {
            if (err) *err = q.lastError().text();
            return false;
        }
    }

    // Таблица истории сортировок
    {
        QSqlQuery q(db);
        if (!q.exec(R"(
            CREATE TABLE IF NOT EXISTS sort_history (
                id BIGINT PRIMARY KEY AUTO_INCREMENT,
                user_id BIGINT NOT NULL,
                original_json LONGTEXT NOT NULL,
                sorted_json LONGTEXT NOT NULL,
                created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
                INDEX idx_user_created(user_id, created_at),
                CONSTRAINT fk_history_user
                    FOREIGN KEY (user_id) REFERENCES user(id)
                    ON DELETE CASCADE
            )
        )")) {
            if (err) *err = q.lastError().text();
            return false;
        }
    }

    return true;
}

bool initDb(QString *err)
{
    QSqlDatabase db = dbInstance(); // Получаем соединение (или создаём)

    if (!db.isOpen()) {
        // Открываем соединение (если ошибка — возвращаем текст)
        if (!db.open()) {
            if (err) *err = db.lastError().text();
            return false;
        }
    }

    // Создаём схему (таблицы)
    return ensureSchema(db, err);
}

bool registerUser(const QString &login,
                  const QString &password,
                  long long *newUserId,
                  QString *err)
{
    if (newUserId) *newUserId = -1;

    QSqlDatabase db = dbInstance();
    if (!db.isOpen() && !db.open()) {
        if (err) *err = db.lastError().text();
        return false;
    }

    // Проверяем, что пользователь с таким login не существует
    {
        QSqlQuery q(db);
        q.prepare("SELECT id FROM user WHERE username = :u LIMIT 1");
        q.bindValue(":u", login);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return false;
        }

        if (q.next()) {
            if (err) *err = "Пользователь с таким логином уже существует.";
            return false;
        }
    }

    // Вставляем нового пользователя
    QSqlQuery q(db);
    q.prepare("INSERT INTO user(username, password) VALUES(:u, :p)");
    q.bindValue(":u", login);
    q.bindValue(":p", password);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }

    if (newUserId) {
        *newUserId = q.lastInsertId().toLongLong(); // id нового пользователя
    }

    return true;
}

AuthResult authUser(const QString &login, const QString &password)
{
    AuthResult res;

    QSqlDatabase db = dbInstance();
    if (!db.isOpen() && !db.open()) {
        res.ok = false;
        res.error = db.lastError().text();
        return res;
    }

    // Ищем пользователя по login + password
    QSqlQuery q(db);
    q.prepare("SELECT id FROM user WHERE username = :u AND password = :p LIMIT 1");
    q.bindValue(":u", login);
    q.bindValue(":p", password);

    if (!q.exec()) {
        res.ok = false;
        res.error = q.lastError().text();
        return res;
    }

    if (q.next()) {
        res.ok = true;
        res.userId = q.value(0).toLongLong();
    } else {
        res.ok = false;
    }

    return res;
}
