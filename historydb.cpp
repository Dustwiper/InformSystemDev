#include "historydb.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

// Преобразуем std::vector<int> в JSON-массив:
// [1,2,3,...]
QString vectorToJson(const std::vector<int>& v){
    QJsonArray arr;                 // JSON-массив
    for (int x : v) arr.append(x);  // Добавляем все элементы

    // Compact — без лишних пробелов и переносов
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

// Преобразуем JSON-массив обратно в std::vector<int>
bool jsonToVector(const QString& json, std::vector<int>* out){
    if (!out) return false;
    out->clear();

    const auto doc = QJsonDocument::fromJson(json.toUtf8()); // Парсим JSON
    if (!doc.isArray()){
        return false;
    }

    // Ожидаем массив
    const auto arr = doc.array();
    out->reserve(arr.size());

    for (const auto& val : arr) {
        out->push_back(val.toInt()); // toInt() безопасно приводит JSON-число к int
    }

    return true;
}

// Проверяем, есть ли уже такая запись (чтобы не хранить одинаковые результаты)
static bool isDuplicate(QSqlDatabase db,
                        long long userId,
                        const QString& orig,
                        const QString& sorted,
                        QString* err){
    QSqlQuery q(db);

    // Ищем запись с тем же user_id + original_json + sorted_json
    q.prepare(R"(
        SELECT 1
        FROM sort_history
        WHERE user_id = :uid AND original_json = :o AND sorted_json = :s
        LIMIT 1
    )");

    q.bindValue(":uid", userId);
    q.bindValue(":o", orig);
    q.bindValue(":s", sorted);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false; // Не смогли проверить (считаем, что не дубликат)
    }

    return q.next(); // Если есть строка — значит дубликат
}

bool saveSortHistory(QSqlDatabase db,
                     long long userId,
                     const std::vector<int>& original,
                     const std::vector<int>& sorted,
                     QString* err){

    if (!db.isValid() || !db.isOpen()) {
        if (err) *err = "Ошибка открытия БД!";
        return false;
    }

    const QString origJson = vectorToJson(original); // Сериализация исходного массива
    const QString sortedJson = vectorToJson(sorted); // Сериализация отсортированного массива

    // Если запись уже есть, считаем "успехом" и просто не вставляем дубликат
    if (isDuplicate(db, userId, origJson, sortedJson, err)) {
        return true;
    }

    QSqlQuery q(db);

    // created_at через NOW() в MySQL
    q.prepare(R"(
        INSERT INTO sort_history(user_id, original_json, sorted_json, created_at)
        VALUES(:uid, :o, :s, NOW())
    )");

    q.bindValue(":uid", userId);
    q.bindValue(":o", origJson);
    q.bindValue(":s", sortedJson);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }

    return true;
}

QVector<SortHistoryRow> loadSortHistory(QSqlDatabase db,
                                       long long userId,
                                       int limit,
                                       QString* err)
{
    QVector<SortHistoryRow> out;

    if (!db.isValid() || !db.isOpen()) {
        if (err) *err = "Не удалось открыть базу данных!";
        return out;
    }

    QSqlQuery q(db);

    // Берём последние записи по времени (DESC)
    q.prepare(R"(
        SELECT created_at, original_json, sorted_json
        FROM sort_history
        WHERE user_id = :uid
        ORDER BY created_at DESC
        LIMIT :lim
    )");

    q.bindValue(":uid", userId);
    q.bindValue(":lim", limit);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return out;
    }

    while (q.next()) {
        SortHistoryRow r;
        r.createdAt = q.value(0).toDateTime(); // Дата/время сохранения
        r.originalJson = q.value(1).toString();
        r.sortedJson = q.value(2).toString();
        out.push_back(r);
    }

    return out;
}

bool clearSortHistory(QSqlDatabase db, long long userId, QString* err){
    if (!db.isValid() || !db.isOpen()) {
        if (err) *err = "Не удалось открыть базу данных!";
        return false;
    }

    QSqlQuery q(db);

    // Удаляем только историю текущего пользователя
    q.prepare(R"(DELETE FROM sort_history WHERE user_id = :uid)");
    q.bindValue(":uid", userId);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }

    return true;
}
