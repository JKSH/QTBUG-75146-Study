#ifndef WIDGET_H
#define WIDGET_H

class DraggableCircle;
class QGraphicsLineItem;
class QLabel;
class QLineEdit;

#include <QSplitter>
#include "mylinef.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QSplitter
{
	Q_OBJECT

public:
	Widget(QSplitter *parent = nullptr);
	~Widget();

private:
	void setZoomLevel(int zoom);
	void updateSegments();

	Ui::Widget *ui;
	DraggableCircle* l1p1;
	DraggableCircle* l1p2;
	DraggableCircle* l2p1;
	DraggableCircle* l2p2;
	QGraphicsLineItem* l1;
	QGraphicsLineItem* l2;
};


class ResultWidget : public QWidget
{
	Q_OBJECT

public:
	ResultWidget(QWidget* parent = nullptr);

	void setIntersectionPoint(const QPointF& point);
	void setSegmentRelations(MyLineF::SegmentRelations relations);

private:
	QLineEdit* m_x;
	QLineEdit* m_y;
	QLabel* m_label;
	int m_decimals;
};

#endif // WIDGET_H
