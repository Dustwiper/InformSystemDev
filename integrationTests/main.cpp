#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTextStream>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QSqlError>

#include "db_utils.h"
#include "gnomesort.h"

static QTextStream qout(stdout);
static QTextStream qerr(stderr);

static std::vector<int> genRandomArray(int minLen, int maxLen, int minVal, int maxVal)
{
    if (minLen < 1) minLen = 1;
    if (maxLen < minLen) maxLen = minLen;

    const int len = QRandomGenerator::global()->bounded(minLen, maxLen + 1);

    std::vector<int> v;
    v.reserve(len);

    for (int i = 0; i < len; ++i) {
        const int val = QRandomGenerator::global()->bounded(minVal, maxVal + 1);
        v.push_back(val);
    }
    return v;
}

static bool prepareDbWithN(QSqlDatabase db,
                           long long userId,
                           int n,
                           int minLen, int maxLen,
                           int minVal, int maxVal,
                           qint64 &timeMs,
                           QString &err)
{
    err.clear();
    timeMs = 0;

    if (!ensureSchema(db, &err)) return false;
    if (!clearHistory(db, userId, &err)) return false;

    if (!db.transaction()) {
        err = "Failed to start transaction: " + db.lastError().text();
        return false;
    }

    QElapsedTimer t;
    t.start();

    QSqlQuery q(db);
    q.prepare("INSERT INTO sort_history(user_id, original_json, sorted_json) "
              "VALUES (:u, :o, :s)");

    for (int i = 0; i < n; ++i) {
        auto original = genRandomArray(minLen, maxLen, minVal, maxVal);
        auto sorted = gnomeSort(original);

        q.bindValue(":u", userId);
        q.bindValue(":o", vecToJson(original));
        q.bindValue(":s", vecToJson(sorted));

        if (!q.exec()) {
            err = q.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        err = "Commit failed: " + db.lastError().text();
        db.rollback();
        return false;
    }

    timeMs = t.elapsed();

    const long long cnt = countHistory(db, userId, &err);
    if (cnt != n) {
        if (err.isEmpty()) err = "COUNT mismatch: expected " + QString::number(n) + " got " + QString::number(cnt);
        return false;
    }

    return true;
}

static bool testInsert(QSqlDatabase db, long long userId, int n,
                       int minLen, int maxLen,
                       int minVal, int maxVal)
{
    qint64 ms = 0;
    QString err;
    const bool ok = prepareDbWithN(db, userId, n, minLen, maxLen, minVal, maxVal, ms, err);

    qout << "[A] Insert N=" << n
         << " ok=" << (ok ? 1 : 0)
         << " time_ms=" << ms
         << "\n";

    if (!ok) qerr << "    error: " << err << "\n";
    return ok;
}

static bool testLoadSort100(QSqlDatabase db, long long userId, qint64 &totalMs, double &avgMs, QString &err)
{
    totalMs = 0;
    avgMs = 0.0;
    err.clear();

    QElapsedTimer t;
    t.start();

    const auto rows = fetchRandomHistory(db, userId, 100, &err);
    if (!err.isEmpty() || rows.size() != 100) return false;

    for (const auto &r : rows) {
        std::vector<int> original;
        std::vector<int> expected;

        if (!jsonToVec(r.originalJson, original, &err)) return false;
        if (!jsonToVec(r.sortedJson, expected, &err)) return false;

        const auto got = gnomeSort(original);
        if (got != expected) {
            err = "Sort mismatch (gnomeSort result differs from DB sorted_json)";
            return false;
        }
    }

    totalMs = t.elapsed();
    avgMs = totalMs / 100.0;
    return true;
}

static bool runLoadSortTestSet(QSqlDatabase db, long long userId, int dbSize,
                               int minLen, int maxLen,
                               int minVal, int maxVal)
{
    qint64 fillMs = 0;
    QString err;
    const bool prepOk = prepareDbWithN(db, userId, dbSize, minLen, maxLen, minVal, maxVal, fillMs, err);
    if (!prepOk) {
        qout << "[B] Load+Sort (prepare) size=" << dbSize << " ok=0\n";
        qerr << "    error: " << err << "\n";
        return false;
    }

    bool allOk = true;
    for (int run = 1; run <= 3; ++run) {
        qint64 totalMs = 0;
        double avgMs = 0.0;
        QString e;

        const bool ok = testLoadSort100(db, userId, totalMs, avgMs, e);
        allOk = allOk && ok;

        qout << "[B] Load+Sort size=" << dbSize
             << " run=" << run
             << " ok=" << (ok ? 1 : 0)
             << " total_ms=" << totalMs
             << " avg_ms=" << QString::number(avgMs, 'f', 3)
             << "\n";

        if (!ok) qerr << "    error: " << e << "\n";
    }

    return allOk;
}

static bool testClear(QSqlDatabase db, long long userId, qint64 &ms, QString &err)
{
    ms = 0;
    err.clear();

    QElapsedTimer t;
    t.start();

    if (!clearHistory(db, userId, &err)) return false;

    ms = t.elapsed();

    const long long cnt = countHistory(db, userId, &err);
    if (cnt != 0) {
        if (err.isEmpty()) err = "COUNT mismatch after clear: expected 0 got " + QString::number(cnt);
        return false;
    }
    return true;
}

static bool runClearTestSet(QSqlDatabase db, long long userId, int dbSize,
                            int minLen, int maxLen,
                            int minVal, int maxVal)
{
    bool allOk = true;

    for (int run = 1; run <= 3; ++run) {
        qint64 fillMs = 0;
        QString err;

        const bool prepOk = prepareDbWithN(db, userId, dbSize, minLen, maxLen, minVal, maxVal, fillMs, err);
        if (!prepOk) {
            allOk = false;
            qout << "[C] Clear size=" << dbSize << " run=" << run << " ok=0 time_ms=0\n";
            qerr << "    error (prepare): " << err << "\n";
            continue;
        }

        qint64 clearMs = 0;
        QString e;
        const bool ok = testClear(db, userId, clearMs, e);
        allOk = allOk && ok;

        qout << "[C] Clear size=" << dbSize
             << " run=" << run
             << " ok=" << (ok ? 1 : 0)
             << " time_ms=" << clearMs
             << "\n";

        if (!ok) qerr << "    error: " << e << "\n";
    }

    return allOk;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DbConfig cfg;
    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a.startsWith("--db=")) cfg.dbFile = a.mid(QString("--db=").size());
    }

    QString err;
    auto db = openTestDb(cfg, &err);
    if (!err.isEmpty() || !db.isOpen()) {
        qerr << "DB OPEN ERROR: " << err << "\n";
        return 3;
    }

    long long userId = 0;
    if (!ensureTestUser(db, userId, &err)) {
        qerr << "TEST USER ERROR: " << err << "\n";
        return 5;
    }

    const int minLen = 1;
    const int maxLen = 300;
    const int minVal = -1000000;
    const int maxVal =  1000000;

    bool okAll = true;

    okAll = testInsert(db, userId, 100,   minLen, maxLen, minVal, maxVal) && okAll;
    okAll = testInsert(db, userId, 1000,  minLen, maxLen, minVal, maxVal) && okAll;
    okAll = testInsert(db, userId, 10000, minLen, maxLen, minVal, maxVal) && okAll;

    okAll = runLoadSortTestSet(db, userId, 100,   minLen, maxLen, minVal, maxVal) && okAll;
    okAll = runLoadSortTestSet(db, userId, 1000,  minLen, maxLen, minVal, maxVal) && okAll;
    okAll = runLoadSortTestSet(db, userId, 10000, minLen, maxLen, minVal, maxVal) && okAll;

    okAll = runClearTestSet(db, userId, 100,   minLen, maxLen, minVal, maxVal) && okAll;
    okAll = runClearTestSet(db, userId, 1000,  minLen, maxLen, minVal, maxVal) && okAll;
    okAll = runClearTestSet(db, userId, 10000, minLen, maxLen, minVal, maxVal) && okAll;

    qout << "\nDONE ok=" << (okAll ? 1 : 0) << "\n";
    qout << "SQLite file: " << db.databaseName() << "\n";
    return okAll ? 0 : 1;
}
