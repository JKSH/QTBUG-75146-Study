#ifndef DRAGGABLECIRCLE_H
#define DRAGGABLECIRCLE_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>

class DraggableCircle : public QObject, public QGraphicsEllipseItem
{
	Q_OBJECT

signals:
	void moved(const QPointF& pos) const;

public:
	DraggableCircle(QObject* parent = nullptr);

	qreal radius() const { return m_radius; }
	void setRadius(qreal r)
	{
		m_radius = r;
		const QRectF oldRect = rect();
		setRect(oldRect.x(), oldRect.y(), 2*r, 2*r);
	}

	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
	{
		QGraphicsEllipseItem::mouseMoveEvent(event);
		emit moved(this->pos()); // NOT the event pos!
	}

private:
	qreal m_radius = 1.0/16;
};

#endif // DRAGGABLECIRCLE_H
