#ifndef TESTS_H
#define TESTS_H

#include "mylinef.h"

#include <QMap>
#include <QMetaObject>

struct EndpointCoords
{
	qreal l1x1, l1y1;
	qreal l1x2, l1y2;
	qreal l2x1, l2y1;
	qreal l2x2, l2y2;
};

struct SegmentPair { MyLineF l1, l2; };

const QMap<QString, EndpointCoords> presets
{
	{ "01. QTest: Parallel", {1.0, 1.0, 3.0, 4.0, 5.0, 6.0, 7.0, 9.0} },
	{ "02. QTest: Unbounded", {1.0, 1.0, 5.0, 5.0, 0.0, 4.0, 3.0, 4.0} },
	{ "03. QTest: Bounded", {1.0, 1.0, 5.0, 5.0, 0.0, 4.0, 5.0, 4.0} },
	{ "04. QTest: Almost vertical", {0.0, 10.0, 20.0000000000001, 10.0, 10.0, 0.0, 10.0, 20.0} },
	{ "05. QTest: Almost horizontal", {0.0, 10.0, 20.0, 10.0, 10.0000000000001, 0.0, 10.0, 20.0} },
	{ "06. QTest: Long vertical", {100.1599256468623, 100.7861905065196, 100.1599256468604, -9999.78619050651, 10.0, 50.0, 190.0, 50.0} },
	{
		"07. QTBUG-75146 Trigger",
		{
			494.70272621399579, -3419.3150119034844,
			484.87413636440681, -3439.7415553154151,
			1553.8915715961471, -1218.0259905149323,
			589.08872351030004, -3223.1546571877006
		}
	},
	{ "08. QTBUG-75146 Parallel unbounded", {0.0, 0.0, 4.0, 3.0, 8.0, 6.0, 10.0, 7.5} },
	{ "09. QTBUG-75146 Parallel bounded", {0.0, 0.0, 4.0, 3.0, 4.0, 3.0, 10.0, 7.5} },
	{ "10. QTBUG-75146 Parallel nested", {2.0, 1.0, 1.0, 1.0, -1.0, 1.0, 4.0, 1.0} },
	{ "11. Unit Vectors", {0, 0, 0, 1, 0, 0, 1, 0} },
	{ "12. Tiny vectors near origin", {1E-10, 1E-10, 0, 1E-10, 1E-10, 1E-10, 1E-10, 0} },
	{ "13. Sub-epsilon vectors near origin", {1E-18, 1E-18, 0, 1E-18, 1E-18, 1E-18, 1E-18, 0} }
};

class Benchmarker
{
	Q_GADGET

public:
	// TODO: Differentiate between collinear and parallel
	enum Category
	{
		PresetParallel,
		PresetParallelSwapped,
		PresetNonParallel,
		PresetNonParallelSwapped,
		MonteCarlo,
		MonteCarloSwapped
	};
	Q_ENUM(Category)

	void setIterationsPerFunction(int n) { m_iterationsPerFunction = n; }
	void setMonteCarloCaseCount(int n) { m_nMonteCarloCases = n; }
	void setRandomSeed(uint seed) { m_randomSeed = seed; }

	void runBenchmarks() const;
	// TODO: Compare outputs of different functions, check for discrepencies

private:
	QVector<SegmentPair> getTestSet(Category category) const;

	int m_iterationsPerFunction = 10000000;
	int m_nMonteCarloCases = 100000;
	uint m_randomSeed = 1;
};

#endif // TESTS_H
