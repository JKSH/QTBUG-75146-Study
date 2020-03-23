#include "widget.h"
#include "ui_widget.h"
#include "draggablecircle.h"
#include "mylinef.h"
#include "tests.h"

#include <QGraphicsLineItem>
#include <QLineEdit>

#include <QDebug>

#include <cmath>

Widget::Widget(QSplitter *parent) :
	QSplitter(parent),
	ui(new Ui::Widget),
	l1p1(new DraggableCircle(this)),
	l1p2(new DraggableCircle(this)),
	l2p1(new DraggableCircle(this)),
	l2p2(new DraggableCircle(this))
{
	ui->setupUi(this);

	auto scene = new QGraphicsScene(this);
	ui->graphicsView->setScene(scene);

	QPen pen;
	pen.setCosmetic(true);
	l1 = scene->addLine(QLineF(), pen);
	l2 = scene->addLine(QLineF(), pen);

	scene->addItem(l1p1);
	scene->addItem(l1p2);
	scene->addItem(l2p1);
	scene->addItem(l2p2);

	l1p1->setPos(-1, -1);
	l1p2->setPos( 1,  1);
	l2p1->setPos(-1,  1);
	l2p2->setPos( 1, -1);

	auto setupCircleControls = [this](DraggableCircle* c, QDoubleSpinBox* xBox, QDoubleSpinBox* yBox)->void
	{
		xBox->setValue(c->pos().x());
		yBox->setValue(c->pos().y());

		auto updateAllShapes = [=]()->void
		{
			c->setPos(xBox->value(), yBox->value());
			this->updateSegments();
		};

		QObject::connect(xBox, qOverload<double>(&QDoubleSpinBox::valueChanged), updateAllShapes);
		QObject::connect(yBox, qOverload<double>(&QDoubleSpinBox::valueChanged), updateAllShapes);
		QObject::connect(c, &DraggableCircle::moved, [=](const QPointF& pos)
		{
			xBox->setValue(pos.x());
			yBox->setValue(pos.y());
		});
	};
	setupCircleControls(l1p1, ui->dsb_l1x1, ui->dsb_l1y1);
	setupCircleControls(l1p2, ui->dsb_l1x2, ui->dsb_l1y2);
	setupCircleControls(l2p1, ui->dsb_l2x1, ui->dsb_l2y1);
	setupCircleControls(l2p2, ui->dsb_l2x2, ui->dsb_l2y2);

	ui->cb_presets->addItems(presets.keys());
	connect(ui->cb_presets, &QComboBox::textActivated, [=](const QString& text)
	{
		if (text == "<Manual>")
			return;

		auto coords = presets[text];
		ui->dsb_l1x1->setValue(coords.l1x1);
		ui->dsb_l1x2->setValue(coords.l1x2);
		ui->dsb_l1y1->setValue(coords.l1y1);
		ui->dsb_l1y2->setValue(coords.l1y2);
		ui->dsb_l2x1->setValue(coords.l2x1);
		ui->dsb_l2x2->setValue(coords.l2x2);
		ui->dsb_l2y1->setValue(coords.l2y1);
		ui->dsb_l2y2->setValue(coords.l2y2);
	});

	connect(ui->slider_zoom, &QSlider::valueChanged,
			this, &Widget::setZoomLevel);
	ui->slider_zoom->setValue(6);
}

Widget::~Widget()
{
	delete ui;
}

void Widget::setZoomLevel(int zoom)
{
	qreal scale = pow(2, zoom);
	qreal r = 5/scale; // TODO: Make zooming mechanism more numerically stable?

	l1p1->setRadius(r);
	l1p2->setRadius(r);
	l2p1->setRadius(r);
	l2p2->setRadius(r);

	updateSegments();
	ui->graphicsView->setTransform( QTransform::fromScale(scale, scale) );
}

void Widget::updateSegments()
{
	MyLineF myLine1(l1p1->pos(), l1p2->pos());
	MyLineF myLine2(l2p1->pos(), l2p2->pos());


	// Old enum
	auto showIntersection = [&](ResultWidget* output,
			std::function<QLineF::IntersectionType(MyLineF*, const MyLineF&, QPointF*)> intersectionFunc)
	{
		QPointF i(Q_QNAN, Q_QNAN);
		QLineF::IntersectionType type = intersectionFunc(&myLine1, myLine2, &i);
		output->setIntersectionPoint(i);
		output->setIntersectionType(type);
	};

	showIntersection(ui->result_flsiOrig, &MyLineF::intersects_flsiOrig);
	showIntersection(ui->result_flsiTweaked, &MyLineF::intersects_flsiTweaked);



	// New enum
	QPointF i(Q_QNAN, Q_QNAN);
	MyLineF::SegmentRelations relations = myLine1.intersects_flsiV2(myLine2, &i);
	ui->result_flsiV2->setIntersectionPoint(i);
	ui->result_flsiV2->setSegmentRelations(relations);

	i = { Q_QNAN, Q_QNAN };
	relations = myLine1.intersects_gaussElim(myLine2, &i);
	ui->result_gaussElim->setIntersectionPoint(i);
	ui->result_gaussElim->setSegmentRelations(relations);

	// QGraphicsEllipseItem::pos() refers to the top-left corner of the bounding rect
	// ASSUMPTION: All endpoints have the same radius
	const qreal r = l1p1->radius();
	const QPointF shift(r, r);
	l1->setLine(myLine1.translated(shift));
	l2->setLine(myLine2.translated(shift));

	auto xBounds = std::minmax({myLine1.p1().x(), myLine1.p2().x(), myLine2.p1().x(), myLine2.p2().x()});
	auto yBounds = std::minmax({myLine1.p1().y(), myLine1.p2().y(), myLine2.p1().y(), myLine2.p2().y()});
	QRectF bounds(QPointF(xBounds.first, yBounds.first), QPointF(xBounds.second+2*r, yBounds.second+2*r));
	ui->graphicsView->setSceneRect( bounds );
}

//=============
// ResultWidget
//=============
ResultWidget::ResultWidget(QWidget* parent) :
	QWidget(parent),
	m_x(new QLineEdit),
	m_y(new QLineEdit),
	m_label(new QLabel),
	m_decimals(20)
{
	m_x->setReadOnly(true);
	m_y->setReadOnly(true);

	auto layout = new QGridLayout(this);
	layout->addWidget(m_x, 0, 0);
	layout->addWidget(m_y, 0, 1);
	layout->addWidget(m_label, 1, 0, 1, 2);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
}

void ResultWidget::setIntersectionPoint(const QPointF& point)
{
	m_x->setText( QString::number(point.x(), 'E', m_decimals) );
	m_y->setText( QString::number(point.y(), 'E', m_decimals) );
}

void ResultWidget::setIntersectionType(QLineF::IntersectionType type)
{
	switch (type)
	{
	case QLineF::NoIntersection:
		m_label->setText("NoIntersection");
		break;
	case QLineF::BoundedIntersection:
		m_label->setText("BoundedIntersection");
		break;
	case QLineF::UnboundedIntersection:
		m_label->setText("UnboundedIntersection");
		break;
	}
}

void ResultWidget::setSegmentRelations(MyLineF::SegmentRelations relations)
{
	QStringList reportList;
	if (relations.testFlag(MyLineF::LinesIntersect))
		reportList << "LinesIntersect";
	if (relations.testFlag(MyLineF::SegmentsIntersect))
		reportList << "SegmentsIntersect";
	if (relations.testFlag(MyLineF::Parallel))
		reportList << "Parallel";
	m_label->setText(reportList.join(" | "));
}
