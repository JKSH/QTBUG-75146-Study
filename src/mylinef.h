#ifndef MYLINEF_H
#define MYLINEF_H

#include <QFlags>
#include <QLineF>

#include <vector>

class MyLineF : public QLineF
{
public:
	enum SegmentRelation
	{
		NoRelation = 0,
		LinesIntersect = 0x1,
		SegmentsIntersect = 0x2,
		Parallel = 0x4
	};
	Q_DECLARE_FLAGS(SegmentRelations, SegmentRelation)

	/*
		Instead of using the (SIC) SegmentRelation enum,
		we could instead update QLineF::IntersectionType (mosly SC):

		enum QLineF::IntersectionType
		{
			NoIntersection,
			UniqueIntersectionWithinSegments,                         // NOTE: This could be interpreted to include collinear segments that share 1 enpoint
			UniqueIntersectionWithoutASegment,
			InfiniteIntersectionWithinSegments,
			InfiniteIntersectionWithoutASegment,
			BoundedIntersection = UniqueIntersectionWithinSegments,   // DEPRECATED
			UnboundedIntersection = UniqueIntersectionWithoutASegment // DEPRECATED
		};

		Basically, we want to describe 5 distinct cases:
		- Parallel but non-collinear (non-overlapping)
		- Non-parallel, overlapping
		- Non-parallel, non-overlapping
		- Collinear, overlapping
		- Collinear, non-overlapping
	*/

	MyLineF() : QLineF(0, 0, 0, 0) {}
	MyLineF(qreal x1, qreal y1, qreal x2, qreal y2) : QLineF(x1, y1, x2, y2) {}
	MyLineF(const QPointF& p1, const QPointF& p2) : QLineF(p1, p2) {}

	// Using the old enum meanings with various algorithms
	IntersectionType intersects_flsiOrig(const QLineF& l, QPointF* intersectionPoint = nullptr) const;
	IntersectionType intersects_flsiTweaked(const QLineF& l, QPointF* intersectionPoint = nullptr) const;
	SegmentRelations intersects_gaussElim(const QLineF& l, QPointF* intersectionPoint = nullptr) const;

	// Using a new enum, new algorithms
	SegmentRelations intersects_flsiV2(const QLineF& l, QPointF* intersectionPoint = nullptr) const;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(MyLineF::SegmentRelations)

#endif // MYLINEF_H
