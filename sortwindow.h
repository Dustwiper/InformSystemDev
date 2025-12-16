#ifndef SORTWINDOW_H
#define SORTWINDOW_H

#include <QWidget>
#include <QString>
#include <vector>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class SortWindow; }
QT_END_NAMESPACE

class QTableWidgetItem;

class SortWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SortWindow(QWidget *parent = nullptr);
    SortWindow(long long userId, const QString &login, QWidget *parent = nullptr);

    void setUserContext(long long userId, const QString &login);

private slots:
    void onClearClicked();
    void onGenerateClicked();
    void onSortClicked();
    void onHistoryClicked();
    void onClearHistoryClicked();
    void onItemChangedSequential(QTableWidgetItem *item);
    void onHelpClicked();

private:
    void commonInit();

    void resetTableToInitial();
    void ensureCapacity(int itemCount);
    void ensureItem(int r, int c);

    int rcToIndex(int r, int c) const;
    void indexToRC(int idx, int &r, int &c) const;
    int capacity() const;

    int calcFilledCount() const;
    bool tryReadFirstN(int n, std::vector<int> &out) const;
    void writeFirstN(const std::vector<int> &v);
    void setFocusToIndex(int idx);

private:
    Ui::SortWindow *ui = nullptr;

    long long m_userId = -1;
    QString m_login;

    int m_maxColumns = 8;
    int m_nextIndex = 0;

    bool m_internalChange = false; // если true — itemChanged игнорируем

    QPushButton* m_historyButton = nullptr; // создаём программно рядом с "Сгенерировать"
    QPushButton* m_helpButton = nullptr;
};

#endif // SORTWINDOW_H
