#include "dialogparams.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QAbstractSpinBox>

DialogParams::DialogParams(QWidget *parent)
    : QDialog(parent){

    setWindowTitle("Параметры генерации массива"); // Заголовок окна диалога
    setModal(true);                                // Диалог модальный: пока не закрыт, главное окно не активно

    // Создаём spinbox'ы
    m_min = new QSpinBox(this);
    m_max = new QSpinBox(this);
    m_quantity = new QSpinBox(this);

    // Убираем стрелки у всех QSpinBox (требование)
    m_min->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_max->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_quantity->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // Диапазоны (min/max допускают отрицательные)
    m_min->setRange(-1000000, 1000000); // Нижняя граница может быть отрицательной
    m_max->setRange(-1000000, 1000000); // Верхняя граница может быть отрицательной
    m_quantity->setRange(1, 1000);      // Количество элементов: минимум 1

    // Значения по умолчанию (чтобы сразу можно было нажать OK)
    m_min->setValue(-10);
    m_max->setValue(10);
    m_quantity->setValue(10);

    // Запрещаем одинаковые границы: всегда поддерживаем min < max.
    // Делается "живой" настройкой диапазонов.
    auto syncRanges = [this]() {
        const int minV = m_min->value(); // Текущее значение минимума
        const int maxV = m_max->value(); // Текущее значение максимума

        m_max->setMinimum(minV + 1); // Максимум должен быть строго больше минимума
        m_min->setMaximum(maxV - 1); // Минимум должен быть строго меньше максимума
    };

    connect(m_min, qOverload<int>(&QSpinBox::valueChanged), this, [syncRanges](int){ syncRanges(); });
    connect(m_max, qOverload<int>(&QSpinBox::valueChanged), this, [syncRanges](int){ syncRanges(); });
    syncRanges(); // Применяем сразу, чтобы диапазоны были корректными с начала

    // Разметка формы
    auto *form = new QFormLayout;
    form->addRow("Минимум:", m_min);
    form->addRow("Максимум:", m_max);
    form->addRow("Количество:", m_quantity);

    // Кнопки OK/Cancel
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // Проверка при нажатии OK (на всякий случай, если пользователь умудрился сделать некорректно)
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_min->value() >= m_max->value()) {
            QMessageBox::warning(this, "Ошибка", "Минимум должен быть строго меньше максимума!");
            return;
        }
        accept(); // Закрываем диалог с результатом Accepted
    });

    connect(buttons, &QDialogButtonBox::rejected, this, &DialogParams::reject);

    // Общая разметка диалога
    auto *root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);
    setLayout(root);
}

int DialogParams::getMin() const { return m_min->value(); }
int DialogParams::getMax() const { return m_max->value(); }
int DialogParams::getQuantity() const { return m_quantity->value(); }
