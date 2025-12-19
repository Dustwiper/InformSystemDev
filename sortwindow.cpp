#include "sortwindow.h"
#include "ui_sortwindow.h"

#include "itemdelegate.h"
#include "dialogparams.h"
#include "gnomesort.h"
#include "historydb.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSignalBlocker>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QSqlError>
#include <QBoxLayout>
#include <QtSql/QSqlDatabase>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QFile>
#include <QCoreApplication>

static const char* kConnName = "qt_mysql_default";

static QSqlDatabase getOpenedDb(QString *err = nullptr){

    if (!QSqlDatabase::contains(kConnName)) {
        if (err) *err = "Соединение с БД не найдено (нет подключения '" + QString(kConnName) + "').";
        return QSqlDatabase();
    }

    auto db = QSqlDatabase::database(kConnName);
    if (!db.isValid()) {
        if (err) *err = "Соединение с БД некорректно (isValid=false).";
        return QSqlDatabase();
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            if (err) *err = db.lastError().text();
            return QSqlDatabase();
        }
    }

    if (err) err->clear();
    return db;
}

SortWindow::SortWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SortWindow)
{
    ui->setupUi(this);
    commonInit();
}


SortWindow::SortWindow(long long userId, const QString &login, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SortWindow)
    , m_userId(userId)
    , m_login(login){

    ui->setupUi(this);
    commonInit();
}

void SortWindow::setUserContext(long long userId, const QString &login){

    m_userId = userId;
    m_login = login;
}

void SortWindow::commonInit(){

    //таблица
    ui->arrayTable->setItemDelegate(new IntItemDelegate(ui->arrayTable));

    ui->arrayTable->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::AnyKeyPressed
        );

    ui->arrayTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->arrayTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->arrayTable->horizontalHeader()->setStretchLastSection(false);

    const int colW = 110;
    const int rowH = 44;
    ui->arrayTable->horizontalHeader()->setDefaultSectionSize(colW);
    ui->arrayTable->verticalHeader()->setDefaultSectionSize(rowH);

    ui->arrayTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // старт: 1 строка, 8 столбцов
    resetTableToInitial();

    // кнопки из UI
    connect(ui->clearTable, &QPushButton::released, this, &SortWindow::onClearClicked);
    connect(ui->generateArray, &QPushButton::released, this, &SortWindow::onGenerateClicked);

    connect(ui->sort, &QPushButton::released, this, &SortWindow::onSortClicked);

    //  кнопка "История" без UI, рядом с "Сгенерировать"
    if (!m_historyButton) {
        QWidget* panel = ui->generateArray->parentWidget(); // topPanel
        if (panel && panel->layout()) {
            if (auto* box = qobject_cast<QBoxLayout*>(panel->layout())) {
                m_historyButton = new QPushButton("История", panel);

                int idx = box->indexOf(ui->generateArray);
                box->insertWidget(idx + 1, m_historyButton);

                connect(m_historyButton, &QPushButton::released,
                        this, &SortWindow::onHistoryClicked);
            }
        }
    }

    if (!m_helpButton) {
        QWidget* panel = ui->generateArray->parentWidget(); // topPanel
        if (panel && panel->layout()) {
            if (auto* box = qobject_cast<QBoxLayout*>(panel->layout())) {
                m_helpButton = new QPushButton("Справка", panel);

                // вставляем после кнопки "История" (если она есть), иначе после "Сгенерировать"
                int idx = m_historyButton ? box->indexOf(m_historyButton) : box->indexOf(ui->generateArray);
                box->insertWidget(idx + 1, m_helpButton);

                connect(m_helpButton, &QPushButton::released,
                        this, &SortWindow::onHelpClicked);
            }
        }
    }

    // ---- последовательный ввод ----
    connect(ui->arrayTable, &QTableWidget::itemChanged,
            this, &SortWindow::onItemChangedSequential);

    setFocusToIndex(0);
}

void SortWindow::resetTableToInitial(){

    m_internalChange = true;
    QSignalBlocker blocker(ui->arrayTable);

    ui->arrayTable->clearContents();
    ui->arrayTable->setColumnCount(m_maxColumns);
    ui->arrayTable->setRowCount(1);

    for (int c = 0; c < m_maxColumns; ++c) {
        ensureItem(0, c);
        ui->arrayTable->item(0, c)->setText("");
    }

    m_nextIndex = 0;

    m_internalChange = false;
}

int SortWindow::rcToIndex(int r, int c) const{

    return r * m_maxColumns + c;
}

void SortWindow::indexToRC(int idx, int &r, int &c) const{

    r = idx / m_maxColumns;
    c = idx % m_maxColumns;
}

int SortWindow::capacity() const{

    return ui->arrayTable->rowCount() * m_maxColumns;
}

void SortWindow::ensureItem(int r, int c){

    if (!ui->arrayTable->item(r, c)) {
        ui->arrayTable->setItem(r, c, new QTableWidgetItem());
        ui->arrayTable->item(r, c)->setTextAlignment(Qt::AlignCenter);
    }
}

void SortWindow::ensureCapacity(int itemCount){

    if (itemCount < 1) itemCount = 1;

    const int needRows = (itemCount + m_maxColumns - 1) / m_maxColumns;
    if (needRows <= ui->arrayTable->rowCount()){
        return;
    }

    m_internalChange = true;
    QSignalBlocker blocker(ui->arrayTable);

    const int oldRows = ui->arrayTable->rowCount();
    ui->arrayTable->setRowCount(needRows);

    for (int r = oldRows; r < needRows; ++r) {
        for (int c = 0; c < m_maxColumns; ++c) {
            ensureItem(r, c);
            ui->arrayTable->item(r, c)->setText("");
        }
    }

    m_internalChange = false;
}

int SortWindow::calcFilledCount() const
{
    const int rows = ui->arrayTable->rowCount();
    const int cols = m_maxColumns;

    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c, ++idx) {
            auto *it = ui->arrayTable->item(r, c);
            if (!it) return idx;
            if (it->text().trimmed().isEmpty()) return idx;
        }
    }
    return rows * cols;
}


bool SortWindow::tryReadFirstN(int n, std::vector<int> &out) const
{
    out.clear();
    if (n <= 0) {
        return false;
    }

    const int rows = ui->arrayTable->rowCount();
    const int cols = m_maxColumns;

    int idx = 0;
    for (int r = 0; r < rows && idx < n; ++r) {
        for (int c = 0; c < cols && idx < n; ++c, ++idx) {
            auto *it = ui->arrayTable->item(r, c);
            if (!it){
                return false;
            }

            const QString s = it->text().trimmed();
            if (s.isEmpty()){
                return false;
            }
            bool ok = false;
            const int val = s.toInt(&ok);
            if (!ok) {
                return false;
            }

            out.push_back(val);
        }
    }
    return (int)out.size() == n;
}

void SortWindow::writeFirstN(const std::vector<int> &v)
{
    m_internalChange = true;
    QSignalBlocker blocker(ui->arrayTable);

    const int n = (int)v.size();
    ensureCapacity(n);

    int idx = 0;
    for (int i = 0; i < n; ++i) {
        int r = 0, c = 0;
        indexToRC(idx++, r, c);
        ensureItem(r, c);
        ui->arrayTable->item(r, c)->setText(QString::number(v[i]));
    }

    m_internalChange = false;
}

void SortWindow::setFocusToIndex(int idx)
{
    if (idx < 0){
        idx = 0;
    }

    ensureCapacity(idx + 1);

    int r = 0, c = 0;
    indexToRC(idx, r, c);
    ensureItem(r, c);

    ui->arrayTable->setCurrentCell(r, c);
}

void SortWindow::onItemChangedSequential(QTableWidgetItem *item)
{
    if (m_internalChange){
        return;
    }
    if (!item){

        return;
    }
    const int r = item->row();
    const int c = item->column();
    const int idx = rcToIndex(r, c);

    const QString txt = item->text().trimmed();

    if (txt.isEmpty()) {
        if (idx < m_nextIndex) {
            m_nextIndex = idx;
            setFocusToIndex(m_nextIndex);
        }
        return;
    }

    bool ok = false;
    txt.toInt(&ok);
    if (!ok) {
        m_internalChange = true;
        QSignalBlocker blocker(ui->arrayTable);
        item->setText("");
        m_internalChange = false;
        return;
    }

    if (idx == m_nextIndex) {
        m_nextIndex++;
        setFocusToIndex(m_nextIndex);
    }
}

void SortWindow::onClearClicked(){
    resetTableToInitial();
    setFocusToIndex(0);
}

void SortWindow::onGenerateClicked()
{
    DialogParams dlg(this);
    if (dlg.exec() != QDialog::Accepted){
        return;
    }

    const int mn = dlg.getMin();
    const int mx = dlg.getMax();
    const int qty = dlg.getQuantity();

    if (qty <= 0) {
        QMessageBox::warning(this, "Параметры", "Количество должно быть больше 0.");
        return;
    }
    if (mn == mx) {
        QMessageBox::warning(this, "Параметры", "Минимальная граница не должна быть равна максимальной.");
        return;
    }

    const int lo = std::min(mn, mx);
    const int hi = std::max(mn, mx);

    std::vector<int> v;
    v.reserve(qty);
    for (int i = 0; i < qty; ++i) {
        const int x = QRandomGenerator::global()->bounded(lo, hi + 1);
        v.push_back(x);
    }

    resetTableToInitial();   // отчищаем старые значения
    writeFirstN(v);

    m_nextIndex = qty;
    setFocusToIndex(std::max(0, qty - 1));
}

void SortWindow::onSortClicked()
{
    const int n = calcFilledCount();
    if (n <= 0) {
        QMessageBox::information(this, "Сортировка", "Массив пуст.");
        return;
    }

    std::vector<int> original;
    if (!tryReadFirstN(n, original)) {
        QMessageBox::warning(this, "Сортировка", "Заполните массив подряд, без пустых ячеек внутри.");
        return;
    }

    const std::vector<int> sorted = gnomeSort(original);

    writeFirstN(sorted);

    // Историю сохраняем, если есть userId
    if (m_userId > 0) {
        QString err;
        auto db = getOpenedDb(&err);
        if (!err.isEmpty()) {
            QMessageBox::warning(this, "БД", "Не удалось сохранить историю:\n" + err);
        }
        else {
            if (!saveSortHistory(db, m_userId, original, sorted, &err)) {
                if (!err.isEmpty()){
                    QMessageBox::warning(this, "БД", "Не удалось сохранить историю:\n" + err);
                }
            }
        }
    }

    setFocusToIndex(std::max(0, n - 1));
}


void SortWindow::onHistoryClicked()
{
    if (m_userId <= 0) {
        QMessageBox::information(this, "История", "Пользователь не определён (userId отсутствует).");
        return;
    }

    QString err;
    auto db = getOpenedDb(&err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "БД", "Не удалось открыть БД:\n" + err);
        return;
    }

    const auto rows = loadSortHistory(db, m_userId, 200, &err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "БД", "Не удалось загрузить историю:\n" + err);
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("История сортировок");
    dlg.resize(950, 520);

    auto *layout = new QVBoxLayout(&dlg);

    auto *table = new QTableWidget(&dlg);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({
        "Дата",
        "Исходный массив",
        "Отсортированный массив"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // ─── helpers ────────────────────────────────────────────────
    auto normalizeArrayText = [](QString s) {
        // убираем [ ]
        s.remove('[');
        s.remove(']');

        // нормализуем пробелы: "1,2,3" → "1, 2, 3"
        s.replace(",", ", ");

        // убираем двойные пробелы (на всякий случай)
        while (s.contains("  "))
            s.replace("  ", " ");

        return s.trimmed();
    };

    auto makeArrayItem = [&](const QString& fullJson) -> QTableWidgetItem* {
        const QString full = normalizeArrayText(fullJson);

        QString shown = full;
        const int limit = 140;
        if (shown.size() > limit)
            shown = shown.left(limit) + "...";

        auto *it = new QTableWidgetItem(shown);
        it->setToolTip(full); // полный текст с пробелами
        it->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return it;
    };
    // ────────────────────────────────────────────────────────────

    table->setRowCount((int)rows.size());
    for (int i = 0; i < (int)rows.size(); ++i) {
        table->setItem(i, 0,
                       new QTableWidgetItem(rows[i].createdAt.toString("yyyy-MM-dd HH:mm:ss")));
        table->setItem(i, 1, makeArrayItem(rows[i].originalJson));
        table->setItem(i, 2, makeArrayItem(rows[i].sortedJson));
    }

    layout->addWidget(table);

    // Двойной клик — окно с полным массивом
    connect(table, &QTableWidget::cellDoubleClicked, &dlg,
            [table, this](int row, int col) {
                if (col == 0) return;

                auto *item = table->item(row, col);
                if (!item) return;

                const QString full =
                    item->toolTip().isEmpty() ? item->text() : item->toolTip();

                QDialog view(this);
                view.setWindowTitle(col == 1
                                        ? "Исходный массив"
                                        : "Отсортированный массив");
                view.resize(900, 500);

                auto *lay = new QVBoxLayout(&view);
                auto *edit = new QPlainTextEdit(&view);
                edit->setReadOnly(true);
                edit->setPlainText(full);

                lay->addWidget(edit);
                view.exec();
            });

    auto *btnRow = new QHBoxLayout();
    auto *btnClear = new QPushButton("Очистить историю", &dlg);
    auto *btnClose = new QPushButton("Закрыть", &dlg);

    btnRow->addStretch();
    btnRow->addWidget(btnClear);
    btnRow->addWidget(btnClose);

    layout->addLayout(btnRow);

    connect(btnClose, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(btnClear, &QPushButton::clicked, this, [this, table]() {
        onClearHistoryClicked();
        table->setRowCount(0);
    });

    dlg.exec();
}


void SortWindow::onClearHistoryClicked()
{
    if (m_userId <= 0){
        return;
    }

    const auto answer = QMessageBox::question(
        this,
        "Подтверждение",
        "Вы действительно хотите очистить всю историю сортировок?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (answer != QMessageBox::Yes){
        return;
    }

    QString err;
    auto db = getOpenedDb(&err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "БД", "Не удалось открыть БД:\n" + err);
        return;
    }

    if (!clearSortHistory(db, m_userId, &err)) {
        if (!err.isEmpty()){
            QMessageBox::warning(this, "БД", "Не удалось очистить историю:\n" + err);
        }
        return;
    }
}

void SortWindow::onHelpClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Справка — как пользоваться приложением");
    dlg.resize(860, 560);

    auto *layout = new QVBoxLayout(&dlg);

    auto *browser = new QTextBrowser(&dlg);
    browser->setOpenExternalLinks(true);
    browser->setReadOnly(true);

    const QString helpPath = QCoreApplication::applicationDirPath() + "/help/help.html";

    QFile f(helpPath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        browser->setHtml(QString::fromUtf8(f.readAll()));
    } else {
        browser->setHtml(
            "<h2>Справка</h2>"
            "<p>Не найден файл справки:</p>"
            "<pre>" + helpPath.toHtmlEscaped() + "</pre>"
                                         "<p>Создай папку <code>help</code> рядом с приложением и положи туда <code>help.html</code>.</p>"
            );
    }

    layout->addWidget(browser);

    auto *btnClose = new QPushButton("Закрыть", &dlg);
    btnClose->setDefault(true);
    connect(btnClose, &QPushButton::clicked, &dlg, &QDialog::accept);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(btnClose);
    layout->addLayout(btnRow);

    dlg.exec();
}

