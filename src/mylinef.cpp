#include "algorithms.h"
#include "mylinef.h"

#include <cmath>

/*
	Implementation by Konstantin Shegunov
	https://codereview.qt-project.org/c/qt/qtbase/+/292807
*/
MyLineF::SegmentRelations MyLineF::intersects_gaussElim(const QLineF &l, QPointF *intersectionPoint) const
{
	QPointF origin = p1(), lorigin = l.p1(), dir = p2() - origin, ldir = l.p2() - lorigin, v = lorigin - origin;
	qreal matrix[2][3] = {
		{ dir.x(), -ldir.x(), v.x() },
		{ dir.y(), -ldir.y(), v.y() }
	};

	// Select the pivot, i.e. bring the heaviest element by abs value to position (0, 0)
	if (qAbs(matrix[0][1]) > qAbs(matrix[0][0]) || qAbs(matrix[1][1]) > qAbs(matrix[0][0]))  {
		// Swap the columns
		qSwap(matrix[0][0], matrix[0][1]);
		qSwap(matrix[1][0], matrix[1][1]);

		qSwap(origin, lorigin);
		qSwap(dir, ldir);
	}
	if (qAbs(matrix[1][0]) > qAbs(matrix[0][0]))  {
		// Swap the rows
		qSwap(matrix[0][0], matrix[1][0]);
		qSwap(matrix[0][1], matrix[1][1]);
		qSwap(matrix[0][2], matrix[1][2]);
	}

	// Bring to row-echelon form (i.e. Gauss eliminate)
	qreal pivot = 1 / matrix[0][0];

	matrix[1][0] *= -pivot;
	for (int i = 2; i > 0; i--)
		matrix[1][i] = std::fma(matrix[1][0], matrix[0][i], matrix[1][i]);

	// Check if we are rank deficient and deal with it accordingly
	if (qAbs(matrix[1][1]) < qAbs(matrix[0][0]) * std::numeric_limits<qreal>::epsilon())  {
		// Solve for the origin point
		qreal n = pivot * matrix[0][2];

		// We need to handle the edge case where the lines are parallel, but still their continuations meet (i.e. it's the same line)
		// Check if the origin point is the same (thus the segments lie on the same line)
		QPointF r = { std::fma(n, dir.x(), origin.x()), std::fma(n, dir.y(), origin.y()) };
		if (qAbs(r.x() * lorigin.y() - r.y() * lorigin.x()) > 2 * std::numeric_limits<qreal>::epsilon() * qAbs(lorigin.x() * lorigin.y()))
			return Parallel;

		// Solve for the end point
		qreal n2 = pivot * (matrix[0][2] - matrix[0][1]);
		// Normal order the parameters
		if (n > n2)
			qSwap(n, n2);

		// Check the type of intersection and find the midpoint for it
		qreal mid = 0;
		SegmentRelations relation = Parallel | LinesIntersect;
		if (n < 0)  {
			if (n2 > 1) {
				mid = qreal(0.5);
				relation |= SegmentsIntersect;
			}
			else {
				if (n2 >= 0)
					relation |= SegmentsIntersect;
				mid = qreal(0.5) * n2;
			}
		}
		else if (n <= 1)  {
			relation |= SegmentsIntersect;
			mid = qreal(0.5) * (n + (n2 > 1 ? 1 : n2));
		}
		else
			mid = qreal(0.5) * (1 + n);

		// Calculate the intersection point if required
		if (intersectionPoint)
			*intersectionPoint = { std::fma(mid, dir.x(), origin.x()), std::fma(mid, dir.y(), origin.y()) };
		return relation;
	}

	// We are not near-singular, back-substitute normally
	qreal nb = matrix[1][2] / matrix[1][1];
	// Calculate the actual intersection point only if we need to
	if (intersectionPoint)
		*intersectionPoint = { std::fma(nb, ldir.x(), lorigin.x()), std::fma(nb, ldir.y(), lorigin.y()) };

	// Check and return the line intersection type
	if (nb < 0 || nb > 1)
		return LinesIntersect;

	qreal na = pivot * std::fma(-nb, matrix[0][1], matrix[0][2]);
	return LinesIntersect | ((na >= 0 && na <= 1) ? SegmentsIntersect : NoRelation);
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

/*
	Implementation described and started by Edward Welbourne
	See comment (2020-03-24) at https://codereview.qt-project.org/c/qt/qtbase/+/292807
	- Replaced private member access with public getters, e.g. QLineF::pt1 -> QLineF::p1()
	- Added declaration and definition for `tolerance` to allow compilation
	- Fixed calculation of intersectionPoint by negating nb
	- Code for parallel case not implemented (yet)
*/
QLineF::IntersectionType
MyLineF::intersects_crossHypot(const QLineF& l, QPointF* intersectionPoint) const
{
	const QPointF a = p2() - p1();
	const QPointF b = l.p1() - l.p2();
	const QPointF c = p1() - l.p1();
	const auto cross = [](QPointF u, QPointF v) -> qreal {
	   return u.x() * v.y() - u.y() * v.x();
	};
	const qreal denominator = cross(a, b);
	const qreal lena = std::hypot(a.x(), a.y());
	const qreal lenb = std::hypot(b.x(), b.y());
	const qreal ca = cross(c, a);
	const qreal bc = cross(b, c);

	qreal tolerance = std::numeric_limits<qreal>::epsilon();
	if (abs(denominator) <= tolerance * lena * lenb) {
		// Degenerate (parallel, or a line has zero length)
		const qreal lenc = std::hypot(c.x(), c.y());
		if (abs(ca) > tolerance * lenc * lena || abs(bc) > tolerance * lenc * lenb)
			return NoIntersection;
		bool overlap = false;
		if (intersectionPoint) {
			// Find mid-point of overlap or gap
		} else {
			// Merely find *whether* the lines overlap
		}
		return overlap ? BoundedIntersection : UnboundedIntersection;
	}
	// The prior code is essentially good enough:
	const qreal na = bc / denominator;
	const qreal nb = -ca / denominator;
	if (intersectionPoint)
		*intersectionPoint = abs(na) > abs(nb) ? l.p1() + nb * b : p1() + na * a;
	return (na < 0 || na > 1 || nb < 0 || nb > 1) ? UnboundedIntersection : BoundedIntersection;
}

/*
	Based on Franklin Antonio's "Faster Line Segment Intersection" algorithm from the book
	"Graphics Gems III".

	- Modifications to stabilize by K. Shegunov
*/
QLineF::IntersectionType
MyLineF::intersects_flsiOrigX(const QLineF &l, QPointF *intersectionPoint) const
{
	constexpr qreal tolerance = 1000;

	// ipmlementation is based on Graphics Gems III's "Faster Line Segment Intersection"
	const QPointF a = p2() - p1();
	const QPointF b = l.p1() - l.p2();
	const QPointF c = p1() - l.p1();

	auto pointAt = [this, &a] (const qreal at) noexcept -> QPointF {
		return p1() + at * a;
	};

	const qreal denominator = a.y() * b.x() - a.x() * b.y();
	if (!std::isfinite(denominator))
		return NoIntersection;

	const qreal length = a.x() * a.x() + a.y() * a.y();
	const qreal epsilon = tolerance * length * std::numeric_limits<qreal>::epsilon();

	if (std::abs(denominator) < epsilon) [[unlikely]] {
		const qreal r = a.y() * c.x() - a.x() * c.y();
		if (std::abs(r) >= epsilon)
			return NoIntersection;

		// Colinear segments
		const QPointF d = l.p2() - p1();
		const qreal n1 = -a.y() * c.y() - a.x() * c.x();
		const qreal n2 = a.x() * d.x() + a.y() * d.y();

		if (std::signbit(n1) != std::signbit(n2) || n1 < 0) {
			const qreal n = std::max(n1, n2);
			if (n > length) {
				if (intersectionPoint)
					*intersectionPoint = pointAt(0.5);
				return BoundedIntersection;
			}
			else  {
				if (intersectionPoint)
					*intersectionPoint = pointAt(0.5 * n / length);
				return n < 0 ? UnboundedIntersection : BoundedIntersection;
			}
		}
		else  {
			const qreal n = std::min(n1, n2);
			if (n > length)  {
				if (intersectionPoint)
					*intersectionPoint = pointAt(0.5 * (1 + n / length));
				return UnboundedIntersection;
			}
			else  {
				if (intersectionPoint)
					*intersectionPoint = (n2 > length) ? pointAt(0.5 * (1 + n / length)) : pointAt(0.5 * (n1 + n2) / length);
				return BoundedIntersection;
			}
		}
	}

	const qreal na = b.y() * c.x() - b.x() * c.y();
	if (intersectionPoint)	// Do the division iff we really need to
		*intersectionPoint = pointAt(na / denominator);

	if (denominator > 0)  {
		if (na < 0 || na > denominator)
			return UnboundedIntersection;
	}
	else if (na > 0 || na < denominator)
		return UnboundedIntersection;

	const qreal nb = a.x() * c.y() - a.y() * c.x();
	if (denominator > 0)  {
		if (nb < 0 || nb > denominator)
			return UnboundedIntersection;
	}
	else if (nb > 0 || nb < denominator)
		return UnboundedIntersection;

	return BoundedIntersection;
}
