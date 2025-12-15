#ifndef DIALOGPARAMS_H
#define DIALOGPARAMS_H

#include <QDialog>

class QSpinBox;

// Диалог параметров генерации массива:
// - минимум (может быть отрицательным)
// - максимум
// - количество элементов
class DialogParams : public QDialog
{
    Q_OBJECT
public:
    explicit DialogParams(QWidget *parent = nullptr);

    int getMin() const;      // Получить нижнюю границу
    int getMax() const;      // Получить верхнюю границу
    int getQuantity() const; // Получить количество элементов

private:
    QSpinBox* m_min = nullptr;      // Поле "минимум"
    QSpinBox* m_max = nullptr;      // Поле "максимум"
    QSpinBox* m_quantity = nullptr; // Поле "количество"
};

#endif // DIALOGPARAMS_H
