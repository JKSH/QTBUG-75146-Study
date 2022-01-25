#include "algorithms.h"
#include "mylinef.h"

struct TaggedPoint
{
	QPointF point;
	uint8_t parentId;

	static bool lessX(const TaggedPoint& a, const TaggedPoint& b)
	{ return a.point.x() < b.point.x(); }

	static bool lessY(const TaggedPoint& a, const TaggedPoint& b)
	{ return a.point.y() < b.point.y(); }
};

MyLineF::SegmentRelations
Algo::analyzeCollinearSegments(const QLineF& s1, const QLineF& s2, QPointF* oneIntersectionPoint, qreal zeroTolerance)
{
	// ASSUMPTION: The segments are guaranteed to be valid and collinear
	MyLineF::SegmentRelations relations = MyLineF::Parallel | MyLineF::LinesIntersect;

	std::vector<TaggedPoint> endPoints
	{
		TaggedPoint{s1.p1(), 1},
		TaggedPoint{s1.p2(), 1},
		TaggedPoint{s2.p1(), 2},
		TaggedPoint{s2.p2(), 2}
	};

	// Sort the endpoints by their coordinates on one axis
	const bool vertical = Algo::robustFuzzyCompare(s1.p1().x(), s1.p2().x(), zeroTolerance);
	std::sort(endPoints.begin(), endPoints.end(),
			vertical ? &TaggedPoint::lessY : &TaggedPoint::lessX);
	// TODO: Sort array-of-pointers instead?

	// This calculation yields:
	// - (If the segments overlap      ) The midpoint of the overlap, OR
	// - (If the segments don't overlap) The midpoint of the gap between the 2 segments
	if (oneIntersectionPoint)
		*oneIntersectionPoint = (endPoints[1].point + endPoints[2].point) / 2;

	if (endPoints[0].parentId != endPoints[1].parentId)
		return relations | MyLineF::SegmentsIntersect; // >= 1 points in common

	// TODO: Double-check if the following test is actually needed.
	//       If the segments have exactly 1 point in common, is it
	//       possible for the previous test to return false?
	const qreal i1 = vertical ? endPoints[1].point.y() : endPoints[1].point.x();
	const qreal i2 = vertical ? endPoints[2].point.y() : endPoints[2].point.x();
	if (Algo::robustFuzzyCompare(i1, i2, zeroTolerance))
		return relations | MyLineF::SegmentsIntersect; // Exactly 1 point in common

	return relations;
}
