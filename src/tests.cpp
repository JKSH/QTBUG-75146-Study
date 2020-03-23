#include "tests.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QMetaEnum>
#include <QTextStream>

typedef std::function<int(const MyLineF*, const MyLineF&, QPointF*)> IntersectionFunc;

struct TestFunctionInfo
{
	QString name;
	IntersectionFunc func;
};

const QVector<TestFunctionInfo> testFunctions
{
	{"intersects_flsiOrig   ", &MyLineF::intersects_flsiOrig},
	{"intersects_flsiTweaked", &MyLineF::intersects_flsiTweaked},
	{"intersects_flsiV2     ", &MyLineF::intersects_flsiV2},
	{"intersects_gaussElim  ", &MyLineF::intersects_gaussElim}
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

void Benchmarker::runBenchmarks() const
{
	QElapsedTimer timer;
	auto benchmarkEnum = QMetaEnum::fromType<Benchmarker::Category>();

	for (int i = 0; i < benchmarkEnum.keyCount(); ++i)
	{
		// ASSUMPTION: Enum values start from 0 and increase by 1
		const auto category = static_cast<Benchmarker::Category>(i);
		const auto testSet = getTestSet(category);

		QTextStream(stdout) << benchmarkEnum.valueToKey(category) << '\n';

		for (auto funcInfo : testFunctions)
		{
			timer.start();
			for (int j = 0; j < m_iterationsPerFunction; ++j)
			{
				int k = j % testSet.count();
				QPointF p(Q_QNAN, Q_QNAN);

				funcInfo.func( &(testSet[k].l1), testSet[k].l2, &p);
				// TODO: Ensure dead code elimination doesn't occur?
			}
			qreal duration = timer.nsecsElapsed();
			QTextStream(stdout) << QString("\t%1:\t%2 ns per call\n").arg(funcInfo.name).arg(duration/m_iterationsPerFunction);
		}
	}
}
