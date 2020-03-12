#include "algorithms.h"
#include "mylinef.h"

#include <cmath>

/*
	Implementation by Konstantin Shegunov
	https://codereview.qt-project.org/c/qt/qtbase/+/292807
*/
QLineF::IntersectionType
MyLineF::intersects_gaussElim(const QLineF &l, QPointF *intersectionPoint) const
{
	qreal matrix[2][3] = {
		{ p2().x() - p1().x(), l.p1().x() - l.p2().x(), l.p1().x() - p1().x() },
		{ p2().y() - p1().y(), l.p1().y() - l.p2().y(), l.p1().y() - p1().y() }
	};
	bool columnsExchanged = false;  // In the general case this'd be the (column) permutation matrix
	auto swapRows = [&matrix] () -> void {
		qSwap(matrix[0][0], matrix[1][0]);
		qSwap(matrix[0][1], matrix[1][1]);
		qSwap(matrix[0][2], matrix[1][2]);
	};

	auto swapColumns = [&matrix, &columnsExchanged] () -> void {
		columnsExchanged = true;
		qSwap(matrix[0][0], matrix[0][1]);
		qSwap(matrix[1][0], matrix[1][1]);
	};

	// Select the pivot
	if (qAbs(matrix[0][1]) > qAbs(matrix[0][0]) || qAbs(matrix[1][1]) > qAbs(matrix[0][0]))
		swapColumns();
	if (qAbs(matrix[1][0]) > qAbs(matrix[0][0]))
		swapRows();

	// Bring to row-echelon form (i.e. Gauss eliminate)
	for (int i = 2; i > 0; i--)
		matrix[1][i] = std::fma(matrix[1][0], -matrix[0][i] / matrix[0][0], matrix[1][i]);

	// Check if we are rank deficient and deal with it accordingly
	if (qAbs(matrix[1][1]) < qAbs(matrix[0][0]) * std::numeric_limits<qreal>::epsilon()) {
		// Calculate the line parameter and the tolerance base value
		qreal na, nb, n = matrix[0][2] / matrix[0][0];
		qreal tolerance = 2 * std::numeric_limits<qreal>::epsilon();

		// We need to handle the edge case where the lines are parallel, but still their continuations meet (i.e. it's the same line)
		if (columnsExchanged) {
			qreal diff = qAbs(std::fma(n, l.p2().x() - l.p1().x(), l.p1().x() - p1().x()) * p1().y())
					+ qAbs(std::fma(n, l.p2().y() - l.p1().y(), l.p1().y() - p1().y()) * p1().x());
			tolerance *= qAbs(p1().x() * p1().y());

			if (diff > tolerance)
				return NoIntersection;

			na = 0;
			nb = n;

			if (intersectionPoint)
				*intersectionPoint = p1();
		} else {
			qreal diff = qAbs(std::fma(n, p2().x() - p1().x(), p1().x() - l.p1().x()) * l.p1().y())
					+ qAbs(std::fma(n, p2().y() - p1().y(), p1().y() - l.p1().y()) * l.p1().x());
			tolerance *= qAbs(l.p1().x() * l.p1().y());

			if (diff > tolerance)
				return NoIntersection;

			na = n;
			nb = 0;
			if (intersectionPoint)
				*intersectionPoint = l.p1();
		}

		Q_UNUSED(na)
		Q_UNUSED(nb)

		return NoIntersection;
	}
	// We are not near-singular, back-substitute normally
	qreal nb = matrix[1][2] / matrix[1][1];
	qreal na = std::fma(-nb, matrix[0][1], matrix[0][2]) / matrix[0][0];
	if (intersectionPoint)  {   // Calculate the actual intersection point only if we need to
		if (columnsExchanged)
			qSwap(na, nb);
		*intersectionPoint = { std::fma(na, p2().x() - p1().x(), p1().x()), std::fma(na, p2().y() - p1().y(), p1().y()) };
	}
	return (na >= 0 && na <= 1 && nb >= 0 && nb <= 1) ? BoundedIntersection : UnboundedIntersection;
}

/*
	Implementation copied from QLineF::intersects() in Qt 5.14.1, except:
	- Replaced private member access with public getters, e.g. QLineF::pt1 -> QLineF::p1()
	- Replaced private function: qt_is_finite() -> std::isfinite()

	Based on Franklin Antonio's "Faster Line Segment Intersection" algorithm from the book
	"Graphics Gems III".

	Replicated here for ease of comparison.
*/
QLineF::IntersectionType
MyLineF::intersects_flsiOrig(const QLineF &l, QPointF *intersectionPoint) const
{
	// ipmlementation is based on Graphics Gems III's "Faster Line Segment Intersection"
	const QPointF a = p2() - p1();
	const QPointF b = l.p1() - l.p2();
	const QPointF c = p1() - l.p1();

	const qreal denominator = a.y() * b.x() - a.x() * b.y();
	if (denominator == 0 || !std::isfinite(denominator))
		return NoIntersection;

	const qreal reciprocal = 1 / denominator;
	const qreal na = (b.y() * c.x() - b.x() * c.y()) * reciprocal;
	if (intersectionPoint)
		*intersectionPoint = p1() + a * na;

	if (na < 0 || na > 1)
		return UnboundedIntersection;

	const qreal nb = (a.x() * c.y() - a.y() * c.x()) * reciprocal;
	if (nb < 0 || nb > 1)
		return UnboundedIntersection;

	return BoundedIntersection;
}

/*
	Slightly modified version of MyLineF::intersects_flsiTweaked() / QLineF::intersects()
	- Avoids a check for exact equality with 0
	- Avoids the division operation in all intermediate results
*/
QLineF::IntersectionType
MyLineF::intersects_flsiTweaked(const QLineF &l, QPointF *intersectionPoint) const
{
	// implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
	const QPointF a = p2() - p1();
	const QPointF b = l.p1() - l.p2();
	const QPointF c = p1() - l.p1();

	const qreal d1 = a.y() * b.x();
	const qreal d2 = a.x() * b.y();

	if (  Algo::robustFuzzyCompare( d1, d2, Algo::findTolerance(a, b) )  ) // Parallel
		return NoIntersection;

	const qreal denominator = d1 - d2;
	if (!std::isfinite(denominator)) // Invalid input: NaN or Inf in at least 1 point
		return NoIntersection;

	const qreal nna = b.y() * c.x() - b.x() * c.y();

	if (intersectionPoint)
		*intersectionPoint = p1() + a * (nna / denominator);

	// Equivalent to but more stable than
	// (nna/denominator) < 0 || (nna/denominator) > 1
	if (   (  denominator>0  &&  ( nna<0 || nna>denominator )  )
		|| (  denominator<0  &&  ( nna>0 || nna<denominator )  )   )
		return UnboundedIntersection;

	const qreal nnb = a.x() * c.y() - a.y() * c.x();
	if (   (  denominator>0  &&  ( nnb<0 || nnb>denominator )  )
		|| (  denominator<0  &&  ( nnb>0 || nnb<denominator )  )   )
		return UnboundedIntersection;

	return BoundedIntersection;
}

/*
	Modified version of MyLineF::intersects_flsiTweaked()
	- Illustrates a more descriptive result than QLineF::IntersectionType
		- Differentiates between parallel and non-parallel segments
		- Differentiates between collinear and non-collinear parallel segments
		- Returns a null result if input is invalid
	- Outputs an intersection point for collinear segments
		- Either the midpoint of the overlap, or the midpoint of the gap
*/
MyLineF::SegmentRelations
MyLineF::intersects_flsiV2(const QLineF& l, QPointF* intersectionPoint) const
{
	// Implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
	const QPointF a = p2() - p1();
	const QPointF b = l.p1() - l.p2();
	const QPointF c = p1() - l.p1();

	const qreal tolerance = Algo::findTolerance(a, b);

	const qreal d1 = a.y() * b.x();
	const qreal d2 = a.x() * b.y();
	const qreal denominator = d1 - d2;

	if (!std::isfinite(denominator)) // Invalid input: At least 1 point contains NaN or Inf
		return SegmentRelations();   // TODO: Set intersection to QPointF(Q_QNAN, Q_QNAN)?

	// TODO: Treat inputs as invalid if any of the segments are zero-length?

	const qreal na1 = b.y() * c.x();
	const qreal na2 = b.x() * c.y();

	if ( Algo::robustFuzzyCompare(d1, d2, tolerance) ) // Parallel
	{
		if ( Algo::robustFuzzyCompare(na1, na2, tolerance) ) // Collinear
			return Algo::analyzeCollinearSegments(*this, l, intersectionPoint);
		return Parallel; // TODO: Set intersection to QPointF(Q_QNAN, Q_QNAN)?
	}

	const qreal nna = na1 - na2;

	// NOTE: The following calculation is unstable if `denominator` is small.
	//       `denominator` approaches 0 as abs(this->angle()) approaches abs(l.angle())
	// TODO: Investigate the effects of enforcing an order of operations
	//       (e.g. (a*nna)/denominator ) and/or "pivoting" by swapping points earlier
	if (intersectionPoint)
		*intersectionPoint = p1() + a * (nna / denominator); // TODO: Use std::fma()?

	// Equivalent to but more stable than
	// (nna/denominator) < 0 || (nna/denominator) > 1
	if (   (  denominator>0  &&  ( nna<0 || nna>denominator )  )
		|| (  denominator<0  &&  ( nna>0 || nna<denominator )  )   )
		return LinesIntersect;

	const qreal nnb = a.x() * c.y() - a.y() * c.x();
	if (   (  denominator>0  &&  ( nnb<0 || nnb>denominator )  )
		|| (  denominator<0  &&  ( nnb>0 || nnb<denominator )  )   )
		return LinesIntersect;

	return LinesIntersect | SegmentsIntersect;
}
