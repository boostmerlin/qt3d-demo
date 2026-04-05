
#include <QQuaternion>
#include "curve.h"

QList<QVector3D> Curve::getPoints() const {
    QList<QVector3D> points;
    auto divisions = getDivisions();
    for (uint i = 0; i <= divisions; i++) {
        float t = float(i) / float(divisions);
        points.append(getPoint(t));
    }
    return points;
}

QVector3D Curve::getTangent(float t) const {
    auto t1 = t - DELTA_EPSILON;
    auto t2 = t + DELTA_EPSILON;
    if (t1 < 0) t1 = 0;
    if (t2 > 1) t2 = 1;

    const auto pt1 = getPoint(t1);
    const auto pt2 = getPoint(t2);

    return (pt2 - pt1).normalized();
}

QPair<QVector3D, QVector3D> Curve::determineInitialNormal(float t) const {
    const auto tangent = getTangent(0);
    float tx = qAbs(tangent.x());
    float ty = qAbs(tangent.y());
    float tz = qAbs(tangent.z());
    float min = std::numeric_limits<float>::max();

    QVector3D vec;
    if (tx <= min) {
        min = tx;
        vec.setX(1.0f);
    }
    if (ty <= min) {
        min = ty;
        vec.setX(0.0f);
        vec.setY(1.0f);
    }
    if (tz <= min) {
        vec.setX(0.0f);
        vec.setY(0.0f);
        vec.setZ(1.0f);
    }

    auto b = QVector3D::crossProduct(tangent, vec).normalized();
    auto n = QVector3D::crossProduct(b, tangent);
    return {n, b};
}

Curve::CurveFrames::CurveFrames(qsizetype n) {
    n += 1;
    points.reserve(n);
    tangents.reserve(n);
    normals.reserve(n);
    binormals.reserve(n);
}

Curve::CurveFrames Curve::getFrames() const {
    uint divisions = getDivisions();
    CurveFrames frames(divisions);
    auto [normal, binormal] = determineInitialNormal(0);
    frames.points.append(getPoint(0));
    frames.tangents.append(getTangent(0));
    frames.normals.append(normal);
    frames.binormals.append(binormal);
    for (uint i = 1; i <= divisions; i++) {
        float t = float(i) / float(divisions);
        frames.points.append(getPoint(t));
        frames.tangents.append(getTangent(t));
        uint pre = i - 1;
        frames.normals.append(frames.normals[pre]);

        auto v = QVector3D::crossProduct(frames.tangents[pre], frames.tangents[i]);
        if (v.lengthSquared() > std::numeric_limits<float>::epsilon()) {
            float angle = qAcos(qBound(-1.0f, QVector3D::dotProduct(frames.tangents[pre], frames.tangents[i]), 1.0f));
            //construct quaternion myself, no need to normalize and convert angle again
            v.normalize();
            float a = angle / 2.0f;
            float s = std::sin(a);
            float c = std::cos(a);
            auto q = QQuaternion(c, v.x() * s, v.y() * s, v.z() * s).normalized();
            frames.normals[i] = q.rotatedVector(frames.normals[pre]);
        }
        frames.binormals.append(QVector3D::crossProduct(frames.tangents[i], frames.normals[i]));
    } //to support closed curve?

    return frames;
}

void Curve::setDivisions(uint divisions) {
    if (divisions > 10000) {
        qWarning() << "Curve::setDivisions: maybe too many divisions";
    }
    if (divisions == 0) {
        divisions = 1;
    }
    if (m_divisions != divisions) {
        m_divisions = divisions;
        m_autoDivisions = false;
        emit curveChanged();
    }
}

float Curve::getCurveLength() const {
    auto p1 = getPoint(0);
    auto p2 = getPoint(1);
    float length = qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
    uint divisions = qMax(uint(length / m_frameStep), 10u);
    uint iter = m_maxIterations;
    while (iter-- > 0) {
        float sum = 0;
        QVector3D last = getPoint(0);
        for (uint i = 1; i <= divisions; i++) {
            float t = float(i) / float(divisions);
            auto current = getPoint(t);
            sum += current.distanceToPoint(last);
            last = current;
        }
        if (qAbs(sum - length) <= m_lengthTolerance) {
            qInfo() << "[Curve::getCurveLength] iterations: " << m_maxIterations - iter << "|length: " << length
                    << "|divisions: " << divisions / 2;
            break;
        }
        length = sum;
        divisions *= 2;
    }

    return length;
}

uint Curve::getDivisions() const {
    uint divisions = isAutoDivisions() ? uint(getCurveLength() / m_frameStep) : m_divisions;
    return divisions;
}

void Curve::setAutoDivisions(bool isAutoDivisions) {
    m_autoDivisions = isAutoDivisions;
}

bool Curve::isAutoDivisions() const {
    return m_autoDivisions;
}

void Curve::setMaxIterations(uint maxIterations) {
    m_maxIterations = qBound(1u, maxIterations, 32u);
}

uint Curve::getMaxIterations() const {
    return m_maxIterations;
}

float Curve::getFrameStep() const {
    return m_frameStep;
}

void Curve::setFrameStep(float frameStep) {
    Q_ASSERT_X(frameStep > 0, "Curve::setFrameStep", "frame step should be positive");
    m_frameStep = frameStep <= 0 ? 0.3f : frameStep;
}

