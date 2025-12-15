#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>

// Делегат для QTableWidget: заменяет стандартный редактор ячейки на QSpinBox.
// Это удобно, чтобы ввод всегда был целым числом, а также разрешить отрицательные значения.
class IntItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit IntItemDelegate(QObject *parent = nullptr);

    // Создаём редактор (QSpinBox) при редактировании ячейки
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    // Передаём значение из модели в редактор
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    // Забираем значение из редактора и записываем обратно в модель
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
};

#endif // ITEMDELEGATE_H
