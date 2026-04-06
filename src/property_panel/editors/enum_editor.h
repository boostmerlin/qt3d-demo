#pragma once

#include <QVariant>
#include <QWidget>

class QLabel;
class QComboBox;

struct EnumEditorItem
{
    QString label;
    QVariant value;
};

class EnumEditor final : public QWidget
{
    Q_OBJECT

public:
    explicit EnumEditor(const QString &label, QWidget *parent = nullptr);

    void setItems(const QList<EnumEditorItem> &items);
    void setCurrentValue(const QVariant &value);
    [[nodiscard]] QVariant currentValue() const;
    [[nodiscard]] QComboBox *comboBox() const;

signals:
    void valueEdited(const QVariant &value);

private:
    QLabel *m_label{};
    QComboBox *m_comboBox{};
};
