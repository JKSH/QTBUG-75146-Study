#ifndef WIDGET_H
#define WIDGET_H

class DraggableCircle;
class QGraphicsLineItem;

#include <QSplitter>

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
#endif // WIDGET_H
