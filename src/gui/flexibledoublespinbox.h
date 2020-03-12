#ifndef FLEXIBLEDOUBLESPINBOX_H
#define FLEXIBLEDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QDoubleValidator>

// Why doesn't QDoubleSpinBox use QDoubleValidator?!

class FlexibleDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	FlexibleDoubleSpinBox(QWidget* parent = nullptr) :
			QDoubleSpinBox(parent),
			m_validator(new QDoubleValidator(-10000, 10000, 324, this))
	{
		// NOTE: The smallest possible value representable by qreal is 5E-324
		// TODO: Keep QDoubleSpinBox::setRange() and QDoubleValidator::setRange() in sync

		setStepType(QDoubleSpinBox::AdaptiveDecimalStepType);
		setKeyboardTracking(false);
	}

	QString textFromValue(double value) const override
	{ return locale().toString(value, 'g', decimals()); }

	double valueFromText(const QString& text) const override
	{ return locale().toDouble(text); }

	QValidator::State validate(QString& text, int& pos) const override
	{ return m_validator->validate(text, pos); }

private:
	QDoubleValidator* m_validator;
};

#endif // FLEXIBLEDOUBLESPINBOX_H
