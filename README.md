# QTBUG-75146 Study

In Qt 5.14.1 the implementation of [`QLineF::intersects()`](https://doc.qt.io/qt-5/qlinef.html#intersects)
has 2 main problems:

1. It is numerically unstable, producing blatantly wrong results with some line segments that are
   almost parallel.
2. It returns "NoIntersection" for colliear line segments that overlap, which is also wrong.

The 1st issue was reported in [QTBUG-75146](https://bugreports.qt.io/browse/QTBUG-75146).

[Konstantin Shegunov](https://github.com/kshegunov) spearheaded efforts to 
[fix #1 and #2](https://codereview.qt-project.org/c/qt/qtbase/+/292807), revealing a 3rd issue in
the process:

3. `QLineF::IntersectionType` is not currently descriptive enough to differentiate between parallel
   and non-parallel intersections.


This little project was created to try out, visualize, and compare different ways for addressing the
issues above. It also provides me an excuse to tinker with the Qt
[Graphics View Framework](https://doc.qt.io/qt-5/graphicsview.html) which I hadn't really used before.


## Line Segment Intersection Functions

The functions implemented here are:

* `intersects_flsiOrig()`: The original algorithm that has been in Qt for many years. Based on
  Franklin Antonio's "Faster Line Segment Intersection" algorithm [1][2].

* `intersects_flsiTweaked()`: A slightly modified version of `intersects_flsiOrig()`; it simply
  tries to improve numerical stability by avoiding an equals-to-zero check, and by avoiding division
  operations in all intermediate results. It doesn't try to address issues #2 or #3.

* `intersects_flsiV2()`: A further modified version of `intersects_flsiTweaked()`. It illustrates a
  more descriptive return value and calculates a well-defined intersection point for collinear line
  segments.

* `intersects_gaussElim()`: An implementation using fast Gaussian elimination by Konstantin Shegunov
  [3].


[1] https://www.sciencedirect.com/science/article/pii/B9780080507552500452  
[2] https://github.com/erich666/GraphicsGems/blob/master/gemsiii/insectc.c  
[3] https://codereview.qt-project.org/c/qt/qtbase/+/292807
