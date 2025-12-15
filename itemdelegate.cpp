#include "itemdelegate.h"

#include <QSpinBox>
#include <QAbstractSpinBox>

IntItemDelegate::IntItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    // Конструктор делегата — дополнительных настроек здесь не требуется
}

QWidget *IntItemDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &,
                                       const QModelIndex &) const
{
    // Создаём QSpinBox как редактор ячейки.
    // Так пользователь гарантированно вводит целое число, а не произвольный текст.
    auto *editor = new QSpinBox(parent);

    editor->setRange(-10000, 10000);                       // Разрешаем отрицательные числа и ограничиваем диапазон
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons); // Убираем стрелки справа (требование из задачи)

    // Важно для стабильности: чтобы сигнал itemChanged не срабатывал на каждую набранную цифру.
    // Значение коммитится один раз, когда пользователь завершил ввод (Enter/потеря фокуса).
    editor->setKeyboardTracking(false);

    return editor;
}

void IntItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Передаём текущее значение из ячейки в QSpinBox при начале редактирования
    auto *spin = qobject_cast<QSpinBox*>(editor);
    if (!spin) return;

    bool ok = false;
    const int v = index.data(Qt::EditRole).toString().toInt(&ok); // Берём значение из модели
    spin->setValue(ok ? v : 0);                                    // Если пусто/не число — показываем 0
}

void IntItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    // Забираем значение из QSpinBox и записываем обратно в модель/таблицу
    auto *spin = qobject_cast<QSpinBox*>(editor);
    if (!spin) return;

    // Записываем в EditRole, чтобы у Qt было "правильное" число в модели.
    // Также Qt сама обновит отображение (DisplayRole).
    model->setData(index, spin->value(), Qt::EditRole);
}
