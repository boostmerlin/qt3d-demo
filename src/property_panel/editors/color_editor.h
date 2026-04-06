#pragma once

#include <QColor>
#include <QWidget>

class QLabel;
class QPushButton;

class ColorEditor final : public QWidget
{
    Q_OBJECT

public:
    explicit ColorEditor(const QString &label, QWidget *parent = nullptr);

    void setColor(const QColor &color);
    [[nodiscard]] QColor color() const;
    void setDialogTitle(const QString &title);
    [[nodiscard]] QPushButton *button() const;

signals:
    void valueEdited(const QColor &color);

private:
    void refreshButton() const;

    QLabel *m_label{};
    QPushButton *m_button{};
    QColor m_color{QStringLiteral("#e2e8f0")};
    QString m_dialogTitle;
};
