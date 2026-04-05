//
// Created by merlin
//

#ifndef PMAXWELL_QT_CURVE_H
#define PMAXWELL_QT_CURVE_H

#include <QVector3D>
#include <QObject>
#include <QList>

//curve could be in 3d space
class Curve : public QObject {
Q_OBJECT

public:
    struct CurveFrames {
        explicit CurveFrames(qsizetype n);

        QList<QVector3D> points;
        QList<QVector3D> tangents;
        QList<QVector3D> normals;
        QList<QVector3D> binormals;
    };

    static constexpr float DELTA_EPSILON = 0.001f;

    void setDivisions(uint divisions);

    [[nodiscard]] uint getDivisions() const;

    QList<QVector3D> getPoints() const;

    virtual QVector3D getPoint(float t) const = 0;

    virtual QVector3D getTangent(float t) const;

    //todo: cache and support bounding
    virtual float getCurveLength() const;

    [[nodiscard]] CurveFrames getFrames() const;

    void setAutoDivisions(bool isAutoDivisions);

    bool isAutoDivisions() const;

    void setMaxIterations(uint maxIterations);

    uint getMaxIterations() const;

    void setFrameStep(float frameStep);

    float getFrameStep() const;

signals:

    void curveChanged();

protected:
    virtual QPair<QVector3D, QVector3D> determineInitialNormal(float t) const;

private:
    uint m_divisions = 1;
    bool m_autoDivisions = true;
    uint m_maxIterations = 6;
    float m_frameStep = 0.02f;
    float m_lengthTolerance = 0.001f;
};

#endif