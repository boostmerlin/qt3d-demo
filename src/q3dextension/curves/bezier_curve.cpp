//
// Created by merlin
//

#include "bezier_curve.tpp"

//二次贝塞尔曲线
template class BezierCurve<QVector2D, 2>;
template class BezierCurve<QVector3D, 2>;
//三次贝塞尔曲线
template class BezierCurve<QVector2D, 3>;
template class BezierCurve<QVector3D, 3>;
//四次贝塞尔曲线
template class BezierCurve<QVector2D, 4>;
template class BezierCurve<QVector3D, 4>;

