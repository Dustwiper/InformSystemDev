#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <QString>
#include <QSqlDatabase>
#include <QVector>
#include <vector>

struct DbConfig {
    // SQLite: путь к файлу базы (можно относительный)
    QString dbFile = "gnomesort_test.sqlite";
};

struct HistoryRow {
    QString originalJson;
    QString sortedJson;
};

QSqlDatabase openTestDb(const DbConfig &cfg, QString *err = nullptr);

bool ensureSchema(QSqlDatabase db, QString *err = nullptr);
bool ensureTestUser(QSqlDatabase db, long long &userId, QString *err = nullptr);

QString vecToJson(const std::vector<int> &v);
bool jsonToVec(const QString &json, std::vector<int> &out, QString *err = nullptr);

bool clearHistory(QSqlDatabase db, long long userId, QString *err = nullptr);
long long countHistory(QSqlDatabase db, long long userId, QString *err = nullptr);

QVector<HistoryRow> fetchRandomHistory(QSqlDatabase db, long long userId, int limit, QString *err = nullptr);

#endif // DB_UTILS_H
