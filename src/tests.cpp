#include "tests.h"

#include <cmath>
#include <limits>

#include <QDebug>
#include <QElapsedTimer>
#include <QMetaEnum>
#include <QTextStream>

typedef std::function<int(const MyLineF*, const MyLineF&, QPointF*)> IntersectionFunc;

struct TestFunctionInfo
{
	QString name;
	IntersectionFunc func;
	bool convert;
};

const QVector<TestFunctionInfo> testFunctions
{
	{"intersects_crossHypot ", &MyLineF::intersects_crossHypot, false},
	{"intersects_flsiOrig   ", &MyLineF::intersects_flsiOrig, false},
	{"intersects_flsiTweaked", &MyLineF::intersects_flsiTweaked, false},
	{"intersects_flsiV2     ", &MyLineF::intersects_flsiV2, true},
	{"intersects_XflsiOrigX  ", &MyLineF::intersects_flsiOrigX, false},
	{"intersects_gaussElim  ", &MyLineF::intersects_gaussElim, false}
};

static QVector<SegmentPair>
getTestSet_presets(bool parallel, bool swapSegments)
{
	const QStringList allKeys = presets.keys();

	QVector<SegmentPair> testSet;
	for (auto key : allKeys)
	{
		if (parallel ^ (key.contains("Parallel") || key.contains("Trigger")))
			continue;

//		qDebug() << key;
		auto coords = presets[key];

		MyLineF l1(coords.l1x1, coords.l1y1, coords.l1x2, coords.l1y2);
		MyLineF l2(coords.l2x1, coords.l2y1, coords.l2x2, coords.l2y2);

		if (swapSegments)
			std::swap(l1, l2);

		testSet << SegmentPair{l1, l2};
	}
	return testSet;
}

static QVector<SegmentPair>
getTestSet_monteCarlo(int nTestCases, uint seed, bool swapSegments)
{
	std::srand(seed);
	QVector<SegmentPair> testSet;

	auto randomFloat = []()->qreal
	{
		return qreal(std::rand()) / std::rand();
	};

	for (int i = 0; i < nTestCases; ++i)
	{
		MyLineF l1(randomFloat(), randomFloat(), randomFloat(), randomFloat());
		MyLineF l2(randomFloat(), randomFloat(), randomFloat(), randomFloat());

		if (swapSegments)
			std::swap(l1, l2);

		testSet << SegmentPair{l1, l2};
	}
	return testSet;
}

QVector<SegmentPair>
Benchmarker::getTestSet(Benchmarker::Category category) const
{
	switch (category)
	{
	case Benchmarker::PresetParallel: return getTestSet_presets(true, false);
	case Benchmarker::PresetParallelSwapped: return getTestSet_presets(true, true);
	case Benchmarker::PresetNonParallel: return getTestSet_presets(false, false);
	case Benchmarker::PresetNonParallelSwapped: return getTestSet_presets(false, true);
	case Benchmarker::MonteCarlo: return getTestSet_monteCarlo(m_nMonteCarloCases, m_randomSeed, false);
	case Benchmarker::MonteCarloSwapped: return getTestSet_monteCarlo(m_nMonteCarloCases, m_randomSeed, true);
	}
	Q_UNREACHABLE();
}

void Benchmarker::runSpeedBenchmarks() const
{
	QTextStream(stdout)
			<< "================"  "\n"
			<< "Speed Benchmarks"  "\n"
			<< "================"  "\n";

	QElapsedTimer timer;
	auto benchmarkEnum = QMetaEnum::fromType<Benchmarker::Category>();

	for (int i = 0; i < benchmarkEnum.keyCount(); ++i)
	{
		// ASSUMPTION: Enum values start from 0 and increase by 1
		const auto category = static_cast<Benchmarker::Category>(i);
		const auto testSet = getTestSet(category);

		QTextStream(stdout) << benchmarkEnum.valueToKey(category) << '\n';

		volatile int result = 0;	// Ensure dead code elimination doesn't occur
		Q_UNUSED(result);

		QPointF p(Q_QNAN, Q_QNAN);
		for (auto funcInfo : testFunctions)
		{
			timer.start();
			for (int j = 0; j < m_iterationsPerFunction; ++j)
			{
				int k = j % testSet.count();
				result = funcInfo.func( &(testSet[k].l1), testSet[k].l2, &p);
			}
			qreal duration = timer.nsecsElapsed();
			QTextStream(stdout) << QString("\t%1:\t%2 ns per call\n").arg(funcInfo.name).arg(duration/m_iterationsPerFunction);
		}
		QTextStream(stdout) << '\n';
	}
}


struct AccuracyCheck
{
	QString printSegmentCoords() const
	{
		return QString("{(%1, %2), (%3, %4)} | {(%5, %6), (%7, %8)}")
				.arg(segments.l1.p1().x())
				.arg(segments.l1.p1().y())
				.arg(segments.l1.p2().x())
				.arg(segments.l1.p2().y())
				.arg(segments.l2.p1().x())
				.arg(segments.l2.p1().y())
				.arg(segments.l2.p2().x())
				.arg(segments.l2.p2().y());
	}

	SegmentPair segments;
	qreal diff;
};

void Benchmarker::runAccuracyBenchmarks() const
{
	QTextStream out(stdout);
	out
		<< "==================="  "\n"
		<< "Accuracy Benchmarks"  "\n"
		<< "==================="  "\n";

	auto benchmarkEnum = QMetaEnum::fromType<Benchmarker::Category>();
	for (int i = 0; i < benchmarkEnum.keyCount(); ++i)
	{
		// ASSUMPTION: Enum values start from 0 and increase by 1
		const auto category = static_cast<Benchmarker::Category>(i);
		const auto testSet = getTestSet(category);

		out
			<< benchmarkEnum.valueToKey(category)
			<< QString(": %1 test cases\n").arg(testSet.count());

		QMap<QString, AccuracyCheck> checkMap;

		for (int j = 0; j < testSet.count(); ++j)
		{
			// ASSUMPTION: Gaussian elimination is the last function in the vector.
			//             We're using it as the gold standard.
			Q_ASSERT(testFunctions.last().name.contains("gauss"));
			QPointF expectedPoint;
			MyLineF::SegmentRelations relations = MyLineF::SegmentRelations(testFunctions.last().func( &(testSet[j].l1), testSet[j].l2, &expectedPoint));
			QLineF::IntersectionType expectedIntersection = toIntersectionType(relations);

			for (int k = 0, count = testFunctions.count() - 1; k < count; k++)
			{
				const TestFunctionInfo & testFunction = testFunctions[k];

				QPointF p(Q_QNAN, Q_QNAN);
				int intersection = testFunction.func( &(testSet[j].l1), testSet[j].l2, &p);

				if (testFunction.convert)
					intersection = toIntersectionType(MyLineF::SegmentRelations(intersection));

				qreal diff = std::numeric_limits<qreal>::infinity();

				if (expectedIntersection == QLineF::IntersectionType(intersection)) {
					QPointF offset = expectedPoint - p;
					diff = expectedIntersection == QLineF::NoIntersection ? 0 : std::sqrt(offset.x() * offset.x() + offset.y() * offset.y());
				}
				else  {
					int z= 0;
				}
				if (diff > checkMap[testFunctions[k].name].diff)
					checkMap[testFunctions[k].name] = AccuracyCheck{testSet[j], diff};
			}
		}
		for (auto key : checkMap.keys())
		{
			if (key.contains("gauss"))
				continue;

			out
				<< QString("\t%1:\tMax diff is %2\n").arg(key).arg(checkMap[key].diff)
				<< "\t\t" << checkMap[key].printSegmentCoords() << "\n\n";
		}
	}
}

