#pragma once

#include <functional>

#include <QVector2D>
#include <QVector3D>
#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

struct PointListEditorConfig
{
    QString addButtonText;
    int dimension = 3;
    bool allowAdd = true;
    bool allowReorder = true;
    bool allowRemove = true;
    std::function<QString(int)> rowLabel;
    std::function<QString(int, int)> axisLabel;
    std::function<bool(int, int)> axisVisible;
};

class ReorderablePointListEditor final : public QWidget
{
    Q_OBJECT

public:
    explicit ReorderablePointListEditor(PointListEditorConfig config, QWidget *parent = nullptr);

    void setPoints3D(const QList<QVector3D> &points);
    void setPoints2D(const QList<QVector2D> &points);
    void rebuild();

signals:
    void addRequested();
    void pointEdited(int index, int axis, double value);
    void moveRequested(int from, int to);
    void removeRequested(int index);

private:
    QString rowLabelForIndex(int index) const;
    QString axisLabelFor(int row, int axis) const;
    bool isAxisVisible(int row, int axis) const;

    PointListEditorConfig m_config;
    QPushButton *m_addButton{};
    QWidget *m_rowsWidget{};
    QVBoxLayout *m_rowsLayout{};
    QList<QVector3D> m_points;
};
