#include "gui/widget.h"
#include "tests.h"

#include <QApplication>

#define DO_BENCHMARKS

int main(int argc, char *argv[])
{
#ifndef DO_BENCHMARKS
	QApplication app(argc, argv);

	Widget w;
	w.show();

	return app.exec();
#else
	Q_UNUSED(argc)
	Q_UNUSED(argv)

	Benchmarker benchmarker;
	benchmarker.setIterationsPerFunction(10000000);
	benchmarker.setMonteCarloCaseCount(100000);
	benchmarker.setRandomSeed(1);
	benchmarker.runBenchmarks();

	return 0;
#endif
}
