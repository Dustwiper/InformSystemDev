#ifndef HISTORYDB_H
#define HISTORYDB_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QtSql/QSqlDatabase>
#include <vector>

// Одна запись истории сортировки для пользователя
struct SortHistoryRow
{
    QDateTime createdAt;   // Когда была сохранена запись
    QString originalJson;  // Исходный массив в формате JSON
    QString sortedJson;    // Отсортированный массив в формате JSON
};

// Утилиты преобразования в JSON и обратно
QString vectorToJson(const std::vector<int>& v);
bool jsonToVector(const QString& json, std::vector<int>* out);

// Сохраняет историю сортировки (не сохраняет дубликаты)
bool saveSortHistory(QSqlDatabase db,
                     long long userId,
                     const std::vector<int>& original,
                     const std::vector<int>& sorted,
                     QString* err = nullptr);

// Загружает последние записи истории (по убыванию времени)
QVector<SortHistoryRow> loadSortHistory(QSqlDatabase db,
                                       long long userId,
                                       int limit = 200,
                                       QString* err = nullptr);

// Очищает историю только для выбранного пользователя (удаление в БД)
bool clearSortHistory(QSqlDatabase db,
                      long long userId,
                      QString* err = nullptr);

#endif // HISTORYDB_H
