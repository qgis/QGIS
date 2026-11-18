/***************************************************************************
    qgsrangewidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrangewidgetwrapper.h"

#include "qgsapplication.h"
#include "qgsdial.h"
#include "qgsdoublespinbox.h"
#include "qgsslider.h"
#include "qgsspinbox.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QString>
#include <qlineedit.h>

#include "moc_qgsrangewidgetwrapper.cpp"

using namespace Qt::StringLiterals;

QgsRangeWidgetWrapper::QgsRangeWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{}

int QgsRangeWidgetWrapper::defaultFieldPrecision( const QgsField &field )
{
  constexpr int DEFAULT_PRECISION_DOUBLE = 4;
  const int fieldPrecision = field.precision();
  switch ( field.type() )
  {
    case QMetaType::Type::Float:
    case QMetaType::Type::Double:
      return fieldPrecision > 0 ? fieldPrecision : DEFAULT_PRECISION_DOUBLE;

    // we use the double spin box for long long fields in order to get sufficient range of min/max values
    case QMetaType::Type::LongLong:
      return 0;

    default:
      break;
  }

  return fieldPrecision;
}

QWidget *QgsRangeWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *editor = nullptr;

  if ( config( u"Style"_s ).toString() == "Dial"_L1 )
  {
    editor = new QgsRangeWidgetDial( parent );
  }
  else if ( config( u"Style"_s ).toString() == "Slider"_L1 )
  {
    editor = new QgsRangeWidgetSlider( Qt::Horizontal, parent );
  }
  else
  {
    switch ( layer()->fields().at( fieldIdx() ).type() )
    {
      case QMetaType::Type::Double:
      // for long long field types we have to use a double spin box with 0 decimal places,
      // as the min/max value supported by QSpinBox is not large enough
      case QMetaType::Type::LongLong:
      {
        editor = new QgsRangeWidgetDoubleSpinBox( parent );
        static_cast<QgsRangeWidgetDoubleSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
      }

      case QMetaType::Type::Int:
      default:
        editor = new QgsRangeWidgetSpinBox( parent );
        static_cast<QgsRangeWidgetSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
    }
  }

  return editor;
}

template<class T> static void setupIntEditor( const QVariant &min, const QVariant &max, const QVariant &step, T *slider, QgsRangeWidgetWrapper *wrapper )
{
  // must use a template function because those methods are overloaded and not inherited by some classes
  slider->setMinimum( min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest() );
  slider->setMaximum( max.isValid() ? max.toInt() : std::numeric_limits<int>::max() );
  slider->setSingleStep( step.isValid() ? step.toInt() : 1 );
  QObject::connect( slider, qOverload<int>( &T::valueChanged ), wrapper, &QgsRangeWidgetWrapper::emitValueChanged );
}

template<class T> static void setupVariantEditor( const QVariant &min, const QVariant &max, const QVariant &step, T *slider, QgsRangeWidgetWrapper *wrapper )
{
  // must use a template function because those methods are overloaded and not inherited by some classes
  slider->setMinimum( min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest() );
  slider->setMaximum( max.isValid() ? max.toInt() : std::numeric_limits<int>::max() );
  slider->setSingleStep( step.isValid() ? step.toInt() : 1 );
  QObject::connect( slider, qOverload<const QVariant &>( &T::valueChanged ), wrapper, &QgsRangeWidgetWrapper::emitValueChanged );
}

void QgsRangeWidgetWrapper::initWidget( QWidget *editor )
{
  mDoubleSpinBox = qobject_cast<QDoubleSpinBox *>( editor );
  mIntSpinBox = qobject_cast<QSpinBox *>( editor );
  mQgsRangeWidgetIntSpinBox = qobject_cast<QgsRangeWidgetSpinBox *>( editor );
  mQgsRangeWidgetDoubleSpinBox = qobject_cast<QgsRangeWidgetDoubleSpinBox *>( editor );

  mDial = qobject_cast<QDial *>( editor );
  mSlider = qobject_cast<QSlider *>( editor );
  mQgsDial = qobject_cast<QgsDial *>( editor );
  mQgsSlider = qobject_cast<QgsSlider *>( editor );
  mQgsRangeWidgetDial = qobject_cast<QgsRangeWidgetDial *>( editor );
  mQgsRangeWidgetSlider = qobject_cast<QgsRangeWidgetSlider *>( editor );

  const bool allowNull = config( u"AllowNull"_s, true ).toBool();

  QVariant min( config( u"Min"_s ) );
  QVariant max( config( u"Max"_s ) );
  QVariant step( config( u"Step"_s ) );
  const QVariant precision( config( u"Precision"_s ) );

  if ( mQgsRangeWidgetDoubleSpinBox )
  {
    const double stepval = step.isValid() ? step.toDouble() : 1.0;
    double minval = min.isValid() ? min.toDouble() : std::numeric_limits<double>::lowest();
    const double maxval = max.isValid() ? max.toDouble() : std::numeric_limits<double>::max();

    const QgsField field = layer()->fields().at( fieldIdx() );
    const int precisionval = precision.isValid() ? precision.toInt() : defaultFieldPrecision( field );

    mQgsRangeWidgetDoubleSpinBox->setDecimals( precisionval );
    mQgsRangeWidgetDoubleSpinBox->setShowClearButton( allowNull );
    // Make room for null value: lower the minimum to allow for NULL special values
    if ( allowNull )
    {
      double decr;
      if ( precisionval > 0 )
      {
        decr = std::pow( 10, -precisionval );
      }
      else
      {
        decr = stepval;
      }
      minval -= decr;
      // Note: call setMinimum here or setValue won't work
      mQgsRangeWidgetDoubleSpinBox->setMinimum( minval );
      mQgsRangeWidgetDoubleSpinBox->setValue( minval );
      mQgsRangeWidgetDoubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    mQgsRangeWidgetDoubleSpinBox->setMinimum( minval );
    mQgsRangeWidgetDoubleSpinBox->setMaximum( maxval );
    mQgsRangeWidgetDoubleSpinBox->setSingleStep( stepval );
    if ( config( u"Suffix"_s ).isValid() )
      mQgsRangeWidgetDoubleSpinBox->setSuffix( config( u"Suffix"_s ).toString() );

    connect( mQgsRangeWidgetDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) { emitValueChanged(); } );
  }
  else if ( mDoubleSpinBox )
  {
    const double stepval = step.isValid() ? step.toDouble() : 1.0;
    double minval = min.isValid() ? min.toDouble() : std::numeric_limits<double>::lowest();
    const double maxval = max.isValid() ? max.toDouble() : std::numeric_limits<double>::max();

    const QgsField field = layer()->fields().at( fieldIdx() );
    const int precisionval = precision.isValid() ? precision.toInt() : defaultFieldPrecision( field );
    mDoubleSpinBox->setDecimals( precisionval );

    QgsDoubleSpinBox *qgsWidget = qobject_cast<QgsDoubleSpinBox *>( mDoubleSpinBox );

    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    // Make room for null value: lower the minimum to allow for NULL special values
    if ( allowNull )
    {
      double decr;
      if ( precisionval > 0 )
      {
        decr = std::pow( 10, -precisionval );
      }
      else
      {
        decr = stepval;
      }
      minval -= decr;
      // Note: call setMinimum here or setValue won't work
      mDoubleSpinBox->setMinimum( minval );
      mDoubleSpinBox->setValue( minval );
      QgsDoubleSpinBox *doubleSpinBox( qobject_cast<QgsDoubleSpinBox *>( mDoubleSpinBox ) );
      if ( doubleSpinBox )
        doubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
      else
        mDoubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    mDoubleSpinBox->setMinimum( minval );
    mDoubleSpinBox->setMaximum( maxval );
    mDoubleSpinBox->setSingleStep( stepval );
    if ( config( u"Suffix"_s ).isValid() )
      mDoubleSpinBox->setSuffix( config( u"Suffix"_s ).toString() );

    connect( mDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) { emitValueChanged(); } );
  }
  else if ( mQgsRangeWidgetIntSpinBox )
  {
    mQgsRangeWidgetIntSpinBox->setShowClearButton( allowNull );
    int minval = min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest();
    const int maxval = max.isValid() ? max.toInt() : std::numeric_limits<int>::max();
    const uint stepval = step.isValid() ? step.toUInt() : 1;
    if ( allowNull )
    {
      // make sure there is room for a new value (i.e. signed integer does not overflow)
      const int minvalOverflow = uint( minval ) - stepval;
      if ( minvalOverflow < minval )
      {
        minval = minvalOverflow;
      }
      mQgsRangeWidgetIntSpinBox->setValuesTolerant( minval );
      mQgsRangeWidgetIntSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    setupIntEditor( minval, maxval, stepval, mQgsRangeWidgetIntSpinBox, this );
    if ( config( u"Suffix"_s ).isValid() )
      mQgsRangeWidgetIntSpinBox->setSuffix( config( u"Suffix"_s ).toString() );
  }
  else if ( mIntSpinBox )
  {
    QgsSpinBox *qgsWidget = qobject_cast<QgsSpinBox *>( mIntSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    int minval = min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest();
    const int maxval = max.isValid() ? max.toInt() : std::numeric_limits<int>::max();
    const uint stepval = step.isValid() ? step.toUInt() : 1;
    if ( allowNull )
    {
      // make sure there is room for a new value (i.e. signed integer does not overflow)
      const int minvalOverflow = uint( minval ) - stepval;
      if ( minvalOverflow < minval )
      {
        minval = minvalOverflow;
      }
      mIntSpinBox->setValue( minval );
      QgsSpinBox *intSpinBox( qobject_cast<QgsSpinBox *>( mIntSpinBox ) );
      if ( intSpinBox )
        intSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
      else
        mIntSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    setupIntEditor( minval, maxval, stepval, mIntSpinBox, this );
    if ( config( u"Suffix"_s ).isValid() )
      mIntSpinBox->setSuffix( config( u"Suffix"_s ).toString() );
  }
  else
  {
    ( void ) field().convertCompatible( min );
    ( void ) field().convertCompatible( max );
    ( void ) field().convertCompatible( step );
    if ( mQgsRangeWidgetDial )
      setupVariantEditor( min, max, step, mQgsRangeWidgetDial, this );
    else if ( mQgsRangeWidgetSlider )
      setupVariantEditor( min, max, step, mQgsRangeWidgetSlider, this );
    else if ( mQgsDial )
      setupVariantEditor( min, max, step, mQgsDial, this );
    else if ( mQgsSlider )
      setupVariantEditor( min, max, step, mQgsSlider, this );
    else if ( mDial )
      setupIntEditor( min, max, step, mDial, this );
    else if ( mSlider )
      setupIntEditor( min, max, step, mSlider, this );
  }
}

bool QgsRangeWidgetWrapper::valid() const
{
  return mSlider || mDial || mQgsDial || mQgsSlider || mQgsRangeWidgetDial || mQgsRangeWidgetSlider || mIntSpinBox || mQgsRangeWidgetIntSpinBox || mDoubleSpinBox || mQgsRangeWidgetDoubleSpinBox;
}

void QgsRangeWidgetWrapper::valueChangedVariant( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::Int )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toInt() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toInt() );
  }
  else if ( v.userType() == QMetaType::Type::LongLong )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toLongLong() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toLongLong() );
  }
  else if ( v.userType() == QMetaType::Type::Double )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toDouble() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toDouble() );
  }
}

QVariant QgsRangeWidgetWrapper::value() const
{
  QVariant value;

  if ( mQgsRangeWidgetDoubleSpinBox )
  {
    const QMetaType::Type fieldType = field().type();
    switch ( fieldType )
    {
      case QMetaType::Type::Double:
        value = mQgsRangeWidgetDoubleSpinBox->value();
        break;

      case QMetaType::Type::LongLong:
        value = static_cast<long long>( mQgsRangeWidgetDoubleSpinBox->value() );
        break;

      default:
        break;
    }

    if ( value == mQgsRangeWidgetDoubleSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( fieldType );
    }
  }
  else if ( mDoubleSpinBox )
  {
    const QMetaType::Type fieldType = field().type();
    switch ( fieldType )
    {
      case QMetaType::Type::Double:
        value = mDoubleSpinBox->value();
        break;

      case QMetaType::Type::LongLong:
        value = static_cast<long long>( mDoubleSpinBox->value() );
        break;

      default:
        break;
    }

    if ( value == mDoubleSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( fieldType );
    }
  }
  else if ( mQgsRangeWidgetIntSpinBox )
  {
    value = mQgsRangeWidgetIntSpinBox->value();
    if ( value == mQgsRangeWidgetIntSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( field().type() );
    }
  }
  else if ( mIntSpinBox )
  {
    value = mIntSpinBox->value();
    if ( value == mIntSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( field().type() );
    }
  }
  else if ( mQgsRangeWidgetDial )
  {
    value = mQgsRangeWidgetDial->variantValue();
  }
  else if ( mQgsRangeWidgetSlider )
  {
    value = mQgsRangeWidgetSlider->variantValue();
  }
  else if ( mQgsDial )
  {
    value = mQgsDial->variantValue();
  }
  else if ( mQgsSlider )
  {
    value = mQgsSlider->variantValue();
  }
  else if ( mDial )
  {
    value = mDial->value();
  }
  else if ( mSlider )
  {
    value = mSlider->value();
  }

  return value;
}

void QgsRangeWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mQgsRangeWidgetDoubleSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mQgsRangeWidgetDoubleSpinBox->setValuesTolerant( mQgsRangeWidgetDoubleSpinBox->minimum() );
    }
    else
    {
      mQgsRangeWidgetDoubleSpinBox->setValuesTolerant( value.toDouble() );
    }
  }
  else if ( mDoubleSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mDoubleSpinBox->setValue( mDoubleSpinBox->minimum() );
    }
    else
    {
      mDoubleSpinBox->setValue( value.toDouble() );
    }
  }
  else if ( mQgsRangeWidgetIntSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mQgsRangeWidgetIntSpinBox->setValuesTolerant( mQgsRangeWidgetIntSpinBox->minimum() );
    }
    else
    {
      mQgsRangeWidgetIntSpinBox->setValuesTolerant( value.toInt() );
    }
  }
  else if ( mIntSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mIntSpinBox->setValue( mIntSpinBox->minimum() );
    }
    else
    {
      mIntSpinBox->setValue( value.toInt() );
    }
  }
  else if ( mQgsRangeWidgetDial )
  {
    mQgsRangeWidgetDial->setValuesTolerant( value );
  }
  else if ( mQgsRangeWidgetSlider )
  {
    mQgsRangeWidgetSlider->setValuesTolerant( value );
  }
  else if ( mQgsDial )
  {
    mQgsDial->setValue( value );
  }
  else if ( mQgsSlider )
  {
    mQgsSlider->setValue( value );
  }
  else if ( mDial )
  {
    mDial->setValue( value.toInt() );
  }
  else if ( mSlider )
  {
    mSlider->setValue( value.toInt() );
  }
}

void QgsRangeWidgetWrapper::setEnabled( bool enabled )
{
  if ( mQgsRangeWidgetDoubleSpinBox )
  {
    mQgsRangeWidgetDoubleSpinBox->setReadOnly( !enabled );
    mQgsRangeWidgetDoubleSpinBox->setFrame( enabled );
  }
  else if ( mDoubleSpinBox )
  {
    mDoubleSpinBox->setReadOnly( !enabled );
    mDoubleSpinBox->setFrame( enabled );
  }
  else if ( mQgsRangeWidgetIntSpinBox )
  {
    mQgsRangeWidgetIntSpinBox->setReadOnly( !enabled );
    mQgsRangeWidgetIntSpinBox->setFrame( enabled );
  }
  else if ( mIntSpinBox )
  {
    mIntSpinBox->setReadOnly( !enabled );
    mIntSpinBox->setFrame( enabled );
  }
  else
  {
    QgsEditorWidgetWrapper::setEnabled( enabled );
  }
}

// Tolerant SpinBox

QgsRangeWidgetSpinBox::QgsRangeWidgetSpinBox( QWidget *parent )
  : QgsSpinBox( parent )
{
  connect( this, &QgsRangeWidgetSpinBox::textChanged, this, [this]() {
    // trigger the paint event when the text is changed to display the out-of-range-warning
    update();
  } );
  connect( this, &QgsSpinBox::valueChanged, this, [this]() {
    // a clear should update the tolerant value
    if ( QgsSpinBox::value() == clearValue() )
    {
      mTolerantValue = QgsSpinBox::value();
    }
  } );
}

QString QgsRangeWidgetSpinBox::textFromValue( int val ) const
{
  Q_UNUSED( val );
  // the text must be taken from the tolerant value
  return QString::number( mTolerantValue );
}

void QgsRangeWidgetSpinBox::stepBy( int steps )
{
  // in case the value is below the minimum, we must reset it so that we start with the first valid value.
  QgsSpinBox::setValue( mTolerantValue );
  QgsSpinBox::stepBy( steps );
  // for the text output, the tolerant value must be updated
  setValues( QgsSpinBox::value() );
}

int QgsRangeWidgetSpinBox::value()
{
  // as long as the text hasn't been changed by an interaction or just "touching" (focus in), we return the original value
  int value = mTolerantValue;
  if ( mTouched )
  {
    if ( value < minimum() )
    {
      // if touched and below the minimum it has to fixup the allowed (dummy) value to the real clear-value
      QgsSpinBox::setValue( value );
    }
    value = QgsSpinBox::value();
  }
  return value;
}

void QgsRangeWidgetSpinBox::setValuesTolerant( int val )
{
  // the tolerant value should remain unclamped
  mTolerantValue = val;
  // but the value will be clamped to the allowed range
  if ( mTolerantValue < minimum() )
  {
    // to avoid to have a null value here, we set inthe background to an allowed (dummy) value
    QgsSpinBox::setValue( minimum() + 1 );
  }
  else
  {
    QgsSpinBox::setValue( val );
  }
}

void QgsRangeWidgetSpinBox::setValues( int val )
{
  // the tolerant value should remain unclamped
  mTolerantValue = val;
  // but the value will be clamped to the allowed range
  QgsSpinBox::setValue( val );
}

void QgsRangeWidgetSpinBox::paintEvent( QPaintEvent *event )
{
  QgsSpinBox::paintEvent( event );
  const int val = valueFromText( lineEdit()->text() );
  if ( ( minimum() > val || val > maximum() ) && val != clearValue() )
  {
    //we must set the background of the lineedit to transparent to see the warnings
    lineEdit()->setStyleSheet( u"QWidget { background-color: transparent; }"_s );

    QPainter painter( this );
    const QRect rect = geometry();
    QIcon icon = QgsApplication::getThemeIcon( u"/mIndicatorWarning.svg"_s, QgsApplication::palette().color( QPalette::PlaceholderText ) );
    int iconSize = fontMetrics().height();
    int margin = ( rect.height() - iconSize ) / 2;
    icon.paint( &painter, QRect( margin, margin * 1.5, iconSize, iconSize ), Qt::AlignLeft );
    painter.setPen( QPen( palette().color( QPalette::PlaceholderText ) ) );
    painter.drawText( QRectF( margin * 2 + iconSize, margin * 1.5, rect.width(), rect.height() ), Qt::AlignLeft, "out of range", nullptr );
    painter.end();
  }
}

void QgsRangeWidgetSpinBox::focusInEvent( QFocusEvent *event )
{
  QgsSpinBox::focusInEvent( event );
  // remember that we "touched" the field
  mTouched = true;
}

void QgsRangeWidgetSpinBox::focusOutEvent( QFocusEvent *event )
{
  // first the values have to be set properly from the text
  setValues( valueFromText( lineEdit()->text() ) );
  QgsSpinBox::focusOutEvent( event );
  // and for the text output, the tolerant value must be updated with the (clamped) value
  setValues( QgsSpinBox::value() );
  // we have to emit valueChanged to save even an unchanged but clamped value
  emit valueChanged( QgsSpinBox::value() );
}

void QgsRangeWidgetSpinBox::wheelEvent( QWheelEvent *event )
{
  // in case the value is below the minimum, we must reset it so that we start with the first valid value.
  QgsSpinBox::setValue( mTolerantValue );
  QgsSpinBox::wheelEvent( event );
  // for the text output, the tolerant value must be updated
  setValues( QgsSpinBox::value() );
}

// Tolerant DoubleSpinBox

QgsRangeWidgetDoubleSpinBox::QgsRangeWidgetDoubleSpinBox( QWidget *parent )
  : QgsDoubleSpinBox( parent )
{
  connect( this, &QgsRangeWidgetDoubleSpinBox::textChanged, this, [this] {
    // trigger the paint event when the text is changed to display the out-of-range-warning
    update();
  } );
  connect( this, &QgsDoubleSpinBox::valueChanged, this, [this]() {
    // a clear should update the tolerant value
    if ( QgsDoubleSpinBox::value() == clearValue() )
    {
      mTolerantValue = QgsDoubleSpinBox::value();
    }
  } );
}

QString QgsRangeWidgetDoubleSpinBox::textFromValue( double val ) const
{
  Q_UNUSED( val );
  // the text must be taken from the tolerant value
  return QLocale().toString( mTolerantValue, 'f', decimals() );
}

void QgsRangeWidgetDoubleSpinBox::stepBy( int steps )
{
  // in case the value is below the minimum, we must reset it so that we start with the first valid value.
  QgsDoubleSpinBox::setValue( mTolerantValue );
  QgsDoubleSpinBox::stepBy( steps );
  // for the text output, the tolerant value must be updated
  setValues( QgsDoubleSpinBox::value() );
}

double QgsRangeWidgetDoubleSpinBox::value()
{
  // as long as the text hasn't been changed by an interaction or just "touching" (focus in), we return the original value
  double value = mTolerantValue;
  if ( mTouched )
  {
    if ( value < minimum() )
    {
      // if touched and below the minimum it has to fixup the allowed (dummy) value to the real clear-value
      QgsDoubleSpinBox::setValue( value );
    }
    value = QgsDoubleSpinBox::value();
  }
  return value;
}

void QgsRangeWidgetDoubleSpinBox::setValuesTolerant( double val )
{
  // the tolerant value should remain unclamped
  mTolerantValue = val;
  // but the value will be clamped to the allowed range
  if ( mTolerantValue < minimum() )
  {
    // to avoid to have a null value here, we set inthe background to an allowed (dummy) value
    QgsDoubleSpinBox::setValue( minimum() + 1 );
  }
  else
  {
    QgsDoubleSpinBox::setValue( val );
  }
}

void QgsRangeWidgetDoubleSpinBox::setValues( double val )
{
  // the tolerant value should remain unclamped
  mTolerantValue = val;
  // but the value will be clamped to the allowed range
  QgsDoubleSpinBox::setValue( val );
}

void QgsRangeWidgetDoubleSpinBox::paintEvent( QPaintEvent *event )
{
  QgsDoubleSpinBox::paintEvent( event );

  const double val = valueFromText( lineEdit()->text() );
  if ( ( minimum() > val || val > maximum() ) && val != clearValue() )
  {
    //we must set the background of the lineedit to transparent to see the warnings
    lineEdit()->setStyleSheet( u"QWidget { background-color: transparent; }"_s );

    QPainter painter( this );
    const QRect rect = geometry();
    QIcon icon = QgsApplication::getThemeIcon( u"/mIndicatorWarning.svg"_s, QgsApplication::palette().color( QPalette::PlaceholderText ) );
    int iconSize = fontMetrics().height();
    int margin = ( rect.height() - iconSize ) / 2;
    icon.paint( &painter, QRect( margin, margin * 1.5, iconSize, iconSize ), Qt::AlignLeft );
    painter.setPen( QPen( palette().color( QPalette::PlaceholderText ) ) );
    painter.drawText( QRectF( margin * 2 + iconSize, margin * 1.5, rect.width(), rect.height() ), Qt::AlignLeft, "out of range", nullptr );
    painter.end();
  }
}

void QgsRangeWidgetDoubleSpinBox::focusInEvent( QFocusEvent *event )
{
  QgsDoubleSpinBox::focusInEvent( event );
  // remember that we "touched" the field
  mTouched = true;
}

void QgsRangeWidgetDoubleSpinBox::focusOutEvent( QFocusEvent *event )
{
  // first the values have to be set properly from the text
  setValues( valueFromText( lineEdit()->text() ) );
  QgsDoubleSpinBox::focusOutEvent( event );
  // and for the text output, the tolerant value must be updated with the (clamped) value
  setValues( QgsDoubleSpinBox::value() );
  // we have to emit valueChanged to save even an unchanged but clamped value
  emit valueChanged( QgsDoubleSpinBox::value() );
}

void QgsRangeWidgetDoubleSpinBox::wheelEvent( QWheelEvent *event )
{
  // in case the value is below the minimum, we must reset it so that we start with the first valid value.
  QgsDoubleSpinBox::setValue( mTolerantValue );
  QgsDoubleSpinBox::wheelEvent( event );
  // for the text output, the tolerant value must be updated
  setValues( QgsDoubleSpinBox::value() );
}

// Tolerant Slider

QgsRangeWidgetSlider::QgsRangeWidgetSlider( QWidget *parent )
  : QgsSlider( parent )
{
  // when the value is edited, the tolerant value must be updated as well
  connect( this, &QSlider::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QgsRangeWidgetSlider::QgsRangeWidgetSlider( Qt::Orientation orientation, QWidget *parent )
  : QgsSlider( orientation, parent )
{
  // when the value edited, the tolerant value should be updated as well
  connect( this, &QSlider::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QVariant QgsRangeWidgetSlider::variantValue() const
{
  // as long as the text hasn't been changed by an interaction or just "touching" (focus in), we return the original value
  QVariant value = mTolerantValue;
  if ( mTouched )
  {
    value = QgsSlider::variantValue();
  }
  return value;
}

void QgsRangeWidgetSlider::setValuesTolerant( const QVariant &val )
{
  setValues( val );
}

void QgsRangeWidgetSlider::setValues( const QVariant &val )
{
  QgsSlider::setValue( val );
  // the tolerant value should be set after the value, to avoid clamping
  mTolerantValue = val;
}

void QgsRangeWidgetSlider::paintEvent( QPaintEvent *event )
{
  QgsSlider::paintEvent( event );
  const QVariant val = variantValue();
  if ( minimum().toInt() > val.toInt() || val.toInt() > maximum().toInt() )
  {
    const QString text = val.toString();
    int textWidth = fontMetrics().horizontalAdvance( text );
    QPainter painter( this );
    const QRect rect = geometry();
    QIcon icon = QgsApplication::getThemeIcon( u"/mIndicatorWarning.svg"_s, QgsApplication::palette().color( QPalette::PlaceholderText ) );
    int iconSize = fontMetrics().height();
    icon.paint( &painter, QRect( ( rect.width() / 2 ) - ( textWidth / 2 ) - iconSize, rect.height() * 0.5, iconSize, iconSize ), Qt::AlignLeft );
  }
}

void QgsRangeWidgetSlider::focusInEvent( QFocusEvent *event )
{
  QgsSlider::focusInEvent( event );
  // remember that we "touched" the field
  mTouched = true;
}

void QgsRangeWidgetSlider::focusOutEvent( QFocusEvent *event )
{
  QgsSlider::focusOutEvent( event );
  // we have to emit valueChanged to save even an unchanged but clamped value
  emit valueChanged( QSlider::value() );
}

// Tolerant Dial

QgsRangeWidgetDial::QgsRangeWidgetDial( QWidget *parent )
  : QgsDial( parent )
{
  // when the value is edited, the tolerant value must be updated as well
  connect( this, &QDial::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QVariant QgsRangeWidgetDial::variantValue() const
{
  // as long as the text hasn't been changed by an interaction or just "touching" (focus in), we return the original value
  QVariant value = mTolerantValue;
  if ( mTouched )
  {
    value = QDial::value();
  }
  return value;
}

void QgsRangeWidgetDial::setValuesTolerant( const QVariant &val )
{
  setValues( val );
}

void QgsRangeWidgetDial::setValues( const QVariant &val )
{
  QgsDial::setValue( val );
  // the tolerant value should be set after the value, to avoid clamping
  mTolerantValue = val;
}

void QgsRangeWidgetDial::paintEvent( QPaintEvent *event )
{
  QgsDial::paintEvent( event );

  const QVariant val = variantValue();
  if ( minimum().toInt() > val.toInt() || val.toInt() > maximum().toInt() )
  {
    const QString text = val.toString();
    int textWidth = fontMetrics().horizontalAdvance( text );
    QPainter painter( this );
    const QRect rect = geometry();
    QIcon icon = QgsApplication::getThemeIcon( u"/mIndicatorWarning.svg"_s, QgsApplication::palette().color( QPalette::PlaceholderText ) );
    int iconSize = fontMetrics().height();
    icon.paint( &painter, QRect( ( rect.width() / 2 ) - ( textWidth / 2 ) - iconSize, rect.height() * 0.65, iconSize, iconSize ), Qt::AlignLeft );
  }
}

void QgsRangeWidgetDial::focusInEvent( QFocusEvent *event )
{
  QgsDial::focusInEvent( event );
  // remember that we "touched" the field
  mTouched = true;
}

void QgsRangeWidgetDial::focusOutEvent( QFocusEvent *event )
{
  QgsDial::focusOutEvent( event );
  // we have to emit valueChanged to save even an unchanged but clamped value
  emit valueChanged( QDial::value() );
}
