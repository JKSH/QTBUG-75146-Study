#include "draggablecircle.h"

#include <QPen>

static const qreal defaultRadius = 1;

DraggableCircle::DraggableCircle(QObject* parent) :
		QObject(parent),
		QGraphicsEllipseItem(0, 0, 2*defaultRadius, 2*defaultRadius),
		m_radius(defaultRadius)
{
	QPen pen;
	pen.setCosmetic(true);

	setPen(pen);
	setFlag(QGraphicsItem::ItemIsMovable);
}
