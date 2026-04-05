#include <vector>
#include <array>
#include <QDebug>
#include "geometry_util.h"

bool isVerticesCounterClockWise(const QList<QVector3D> &vertices) {
    if (vertices.size() < 3) {
        return false;
    }
    QVector3D v1 = vertices[1] - vertices[0];
    QVector3D v2 = vertices[2] - vertices[0];
    QVector3D n = QVector3D::crossProduct(v1, v2);
    return n.z() > 0;
}

bool coplanar(const QVector3D &v, const QVector3D &plane, const QVector3D &normal) {
    return qFuzzyIsNull(v.distanceToPlane(plane, normal));
}

double normaliseAngleInDegrees(double angle) {
    return qRadiansToDegrees(normaliseAngle(qDegreesToRadians(angle)));
}

float normaliseAngleInDegrees(float angle) {
    return qRadiansToDegrees(normaliseAngle(qDegreesToRadians(angle)));
}

struct Point {
    int index = 0;
    const QVector3D *vertex = nullptr;

    Point *pre = nullptr;
    Point *next = nullptr;

    inline void remove() const {
        next->pre = pre;
        pre->next = next;
    }
};

struct PointHolder {
    explicit PointHolder(qsizetype n) : total(n) {
        points.resize(n);
    }

    void addPoint(int index, const QVector3D &vertex) {
        auto p = &points[index];
        p->index = index;
        p->vertex = &vertex;
        if (index == 0) {
            header = p;
        } else {
            p->pre = tail;
            tail->next = p;
        }
        tail = p;
        if (total - 1 == index) {
            header->pre = tail;
            tail->next = header;
        }
    }

    Point *header{};
    Point *tail{};
    QList<Point> points;
    qsizetype total;
};

inline double crossZ(double ax, double ay, double bx, double by, double cx, double cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

inline bool isPointInTriangle(double px, double py, double ax, double ay, double bx, double by, double cx, double cy, bool colinear) {
    auto z1 = crossZ(ax, ay, bx, by, px, py);
    auto z2 = crossZ(bx, by, cx, cy, px, py);
    auto z3 = crossZ(cx, cy, ax, ay, px, py);
    return colinear ? z1 > 0 && z2 > 0 && z3 > 0 : z1 >= 0 && z2 >= 0 && z3 >= 0;
}

bool isEar(const Point *it, bool collinear) {
    const Point *p1 = it->pre;
    const Point *p2 = it;
    const Point *p3 = it->next;
    const QVector3D &v1 = *p1->vertex;
    const QVector3D &v2 = *p2->vertex;
    const QVector3D &v3 = *p3->vertex;

    auto v1x = v1.x();
    auto v1y = v1.y();
    auto v2x = v2.x();
    auto v2y = v2.y();
    auto v3x = v3.x();
    auto v3y = v3.y();

    //v2 is tip
    auto z = crossZ(v2x, v2y, v1x, v1y, v3x, v3y);
    if (z >= 0) {
        return false;
    }
    auto p = p3->next;
    while (p != p1) {
        if (isPointInTriangle(p->vertex->x(), p->vertex->y(), v1x, v1y, v2x, v2y, v3x, v3y, collinear)) {
            return false;
        }
        p = p->next;
    }

    return true;
}

double length(const QVector3D &v) {
    return qHypot(static_cast<double>(v.x()), static_cast<double>(v.y()), static_cast<double>(v.z()));
}

QVector3D normalize(const QVector3D &v) {
    const double len = length(v);
    return qFuzzyIsNull(len - 1.0) ? v : qFuzzyIsNull(len) ? QVector3D()
                                                           : QVector3D(static_cast<float>(v[0] / len),
                                                                       static_cast<float>(v[1] / len),
                                                                       static_cast<float>(v[2] / len));
}

void triangulateInner(Point* p, QList<int> &triangles, int tag) {
    auto ear = p;
    auto self = ear;
    while (ear->pre != ear->next) {
        auto pre = ear->pre;
        auto next = ear->next;

        if (isEar(ear, tag == 1)) {
            triangles.append(pre->index);
            triangles.append(ear->index);
            triangles.append(next->index);
            ear->remove();
            ear = next->next;
            self = ear;
        } else {
            ear = next;
            if (ear == self) {
                if (tag == 0) {
                    triangulateInner(ear, triangles, 1);
                }
                break;
            }
        }
    }
//    bool ok = triangles.size() == triangles.capacity();
//    if (!ok) {
//        self = ear;
//        do {
//            qWarning() << "remain index:" << ear->index << "vert:" << ear->vertex->x() << "," << ear->vertex->y() << ","
//                       << ear->vertex->z();
//            ear = ear->next;
//        }
//        while (ear != self);
//    }
}

bool triangulateV2(const QList<QVector3D> &vertices, QList<int> &triangles) {
    auto n = vertices.size();
    if (n < 3) {
        return false;
    }
    triangles.reserve((n - 2) * 3);
    PointHolder holder(n);
    for (int i = 0; i < n; i++) {
        holder.addPoint(i, vertices[i]);
    }
    triangulateInner(holder.header, triangles, 0);
    return triangles.size() == triangles.capacity();
}

