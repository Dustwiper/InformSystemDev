#include "db_utils.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>

QSqlDatabase openTestDb(const DbConfig &cfg, QString *err)
{
    const QString connName = "gnomesort_test_sqlite_conn";

    if (QSqlDatabase::contains(connName)) {
        auto db = QSqlDatabase::database(connName);
        if (db.isOpen()) return db;
        if (!db.open()) {
            if (err) *err = db.lastError().text();
        }
        return db;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);

    QString filePath = cfg.dbFile;
    if (QDir::isRelativePath(filePath)) {
        filePath = QDir::current().absoluteFilePath(filePath);
    }
    db.setDatabaseName(filePath);

    if (!db.open()) {
        if (err) *err = db.lastError().text();
    } else if (err) {
        err->clear();
    }
    return db;
}

bool ensureSchema(QSqlDatabase db, QString *err)
{
    if (!db.isValid() || !db.isOpen()) {
        if (err) *err = "DB is not open";
        return false;
    }

    QSqlQuery q(db);

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS user (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          username TEXT NOT NULL,
          password TEXT NOT NULL,
          created_at TEXT NOT NULL DEFAULT (datetime('now'))
        );
    )")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS sort_history (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user_id INTEGER NOT NULL,
          original_json TEXT NOT NULL,
          sorted_json TEXT NOT NULL,
          created_at TEXT NOT NULL DEFAULT (datetime('now'))
        );
    )")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    if (!q.exec("CREATE INDEX IF NOT EXISTS idx_user_id ON sort_history(user_id);")) {
        if (err) *err = q.lastError().text();
        return false;
    }
    if (!q.exec("CREATE INDEX IF NOT EXISTS idx_created_at ON sort_history(created_at);")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    if (err) err->clear();
    return true;
}

bool ensureTestUser(QSqlDatabase db, long long &userId, QString *err)
{
    userId = 0;
    if (!ensureSchema(db, err)) return false;

    const QString u = "integration_test_user";

    {
        QSqlQuery q(db);
        q.prepare("SELECT id FROM user WHERE username = :u ORDER BY id DESC LIMIT 1");
        q.bindValue(":u", u);
        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return false;
        }
        if (q.next()) {
            userId = q.value(0).toLongLong();
            if (err) err->clear();
            return true;
        }
    }

    {
        QSqlQuery q(db);
        q.prepare("INSERT INTO user(username, password) VALUES (:u, :p)");
        q.bindValue(":u", u);
        q.bindValue(":p", "test");
        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return false;
        }
    }

    {
        QSqlQuery q(db);
        q.prepare("SELECT id FROM user WHERE username = :u ORDER BY id DESC LIMIT 1");
        q.bindValue(":u", u);
        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return false;
        }
        if (!q.next()) {
            if (err) *err = "Failed to read inserted user id";
            return false;
        }
        userId = q.value(0).toLongLong();
    }

    if (err) err->clear();
    return true;
}

QString vecToJson(const std::vector<int> &v)
{
    QJsonArray arr;
    for (int x : v) arr.append(x);
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

bool jsonToVec(const QString &json, std::vector<int> &out, QString *err)
{
    out.clear();
    QJsonParseError pe;
    const auto doc = QJsonDocument::fromJson(json.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isArray()) {
        if (err) *err = "JSON parse error: " + pe.errorString();
        return false;
    }

    const auto arr = doc.array();
    out.reserve(arr.size());
    for (const auto &v : arr) {
        if (!v.isDouble()) {
            if (err) *err = "JSON contains non-number";
            return false;
        }
        out.push_back(v.toInt());
    }
    if (err) err->clear();
    return true;
}

bool clearHistory(QSqlDatabase db, long long userId, QString *err)
{
    QSqlQuery q(db);
    q.prepare("DELETE FROM sort_history WHERE user_id = :u");
    q.bindValue(":u", userId);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    if (err) err->clear();
    return true;
}

long long countHistory(QSqlDatabase db, long long userId, QString *err)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM sort_history WHERE user_id = :u");
    q.bindValue(":u", userId);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return -1;
    }
    if (!q.next()) {
        if (err) *err = "COUNT query returned no rows";
        return -1;
    }
    if (err) err->clear();
    return q.value(0).toLongLong();
}

QVector<HistoryRow> fetchRandomHistory(QSqlDatabase db, long long userId, int limit, QString *err)
{
    QVector<HistoryRow> out;
    out.reserve(limit);

    QSqlQuery q(db);
    q.prepare("SELECT original_json, sorted_json "
              "FROM sort_history "
              "WHERE user_id = :u "
              "ORDER BY RANDOM() "
              "LIMIT :lim");
    q.bindValue(":u", userId);
    q.bindValue(":lim", limit);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return out;
    }

    while (q.next()) {
        HistoryRow r;
        r.originalJson = q.value(0).toString();
        r.sortedJson = q.value(1).toString();
        out.push_back(r);
    }

    if ((int)out.size() != limit) {
        if (err) *err = "Not enough rows in DB (need " + QString::number(limit) +
                        ", got " + QString::number(out.size()) + ")";
        return QVector<HistoryRow>();
    }

    if (err) err->clear();
    return out;
}
