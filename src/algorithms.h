#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "mylinef.h"

#include <cmath>

namespace Algo
{

Q_REQUIRED_RESULT constexpr static inline Q_DECL_UNUSED qreal findTolerance(const QPointF& vector1, const QPointF& vector2)
{
	return std::numeric_limits<qreal>::epsilon() * std::min({1.0,
			vector1.x()*vector1.x() + vector1.y()*vector1.y(),
			vector2.x()*vector2.x() + vector2.y()*vector2.y()});
}

Q_REQUIRED_RESULT constexpr static inline Q_DECL_UNUSED qreal findTolerance(const QLineF& segment1, const QLineF& segment2)
{
	return findTolerance(QPointF(segment1.dx(), segment1.dy()), QPointF(segment2.dx(), segment2.dy()));
}

Q_REQUIRED_RESULT constexpr static inline Q_DECL_UNUSED bool robustFuzzyCompare(qreal p1, qreal p2, qreal zeroTolerance = std::numeric_limits<qreal>::epsilon())
{
	// NOTE: qFuzzyCompare() fails if one input is exactly 0 but the other is near 0
	if (qMin(qAbs(p1), qAbs(p2)) > 0)
		return qFuzzyCompare(p1, p2);

	// TODO: Use zeroTolerance for the fuzzy comparison too?

	return qMax(qAbs(p1), qAbs(p2)) < zeroTolerance; // Stricter than qFuzzyIsNull() if zeroTolerance == epsilon()
}

MyLineF::SegmentRelations analyzeCollinearSegments(const QLineF& s1, const QLineF& s2, QPointF* oneIntersectionPoint = nullptr, qreal zeroTolerance = std::numeric_limits<qreal>::epsilon());

}

#endif // ALGORITHMS_H
