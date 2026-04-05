#ifndef PMAXWELL_QT_LINE_GEOMETRY_H
#define PMAXWELL_QT_LINE_GEOMETRY_H

#include <Qt3DCore/QGeometry>
#include <QVector3D>
//opt: color attribute maybe, QPerVertexColorMaterial rhi not support
//#include <QColor>

class LineGeometry : public Qt3DCore::QGeometry {
Q_OBJECT

public:
    explicit LineGeometry(Qt3DCore::QNode *parent = nullptr);
    enum LineType {
        Line,
        LineLoop, //dx not support
        LineSegments,
    };

    Q_ENUM(LineType)

public slots:

    void setLinePoints(const QList<QVector3D> &points);

    void setLinePoints(QList<QVector3D> &&points);

    void addPoint(const QVector3D &point, int at = -1);

    void removePointAt(int at);

    void setLineType(LineType lineType);

public:
    [[nodiscard]] const QList<QVector3D> &points() const;

    [[nodiscard]] LineType lineType() const;

    void setNormalAttributeEnable(bool enable);

signals:

    void lineTypeChanged(LineType lineType);

private:
    void update();

    QByteArray generateVertices(uint count);

    bool loopPoints(bool loop);

private:
    QList<QVector3D> m_points;
    LineType m_lineType = Line;
    Qt3DCore::QAttribute *m_positionAttribute;
    //some material force ...
    Qt3DCore::QAttribute *m_normalAttribute{};
    Qt3DCore::QBuffer *m_vertexBuffer;
};


#endif //PMAXWELL_QT_LINE_GEOMETRY_H
