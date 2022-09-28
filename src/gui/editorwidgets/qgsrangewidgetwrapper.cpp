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

#include <QSettings>

#include "qgsrangewidgetwrapper.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgsvectorlayer.h"
#include "qgsdial.h"
#include "qgsslider.h"
#include "qgsapplication.h"



QgsRangeWidgetWrapper::QgsRangeWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}

QWidget *QgsRangeWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *editor = nullptr;

  if ( config( QStringLiteral( "Style" ) ).toString() == QLatin1String( "Dial" ) )
  {
    editor = new QgsDial( parent );
  }
  else if ( config( QStringLiteral( "Style" ) ).toString() == QLatin1String( "Slider" ) )
  {
    editor = new QgsSlider( Qt::Horizontal, parent );
  }
  else
  {
    switch ( layer()->fields().at( fieldIdx() ).type() )
    {
      case QVariant::Double:
      // for long long field types we have to use a double spin box with 0 decimal places,
      // as the min/max value supported by QSpinBox is not large enough
      case QVariant::LongLong:
      {

        editor = new QgsDoubleSpinBox( parent );
        static_cast<QgsDoubleSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
      }

      case QVariant::Int:
      default:
        editor = new QgsSpinBox( parent );
        static_cast<QgsSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
    }
  }

  return editor;
}

template<class T>
static void setupIntEditor( const QVariant &min, const QVariant &max, const QVariant &step, T *slider, QgsRangeWidgetWrapper *wrapper )
{
  // must use a template function because those methods are overloaded and not inherited by some classes
  slider->setMinimum( min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest() );
  slider->setMaximum( max.isValid() ? max.toInt() : std::numeric_limits<int>::max() );
  slider->setSingleStep( step.isValid() ? step.toInt() : 1 );
  QObject::connect( slider, SIGNAL( valueChanged( int ) ), wrapper, SLOT( emitValueChanged() ) );
}

void QgsRangeWidgetWrapper::initWidget( QWidget *editor )
{
  mDoubleSpinBox = qobject_cast<QDoubleSpinBox *>( editor );
  mIntSpinBox = qobject_cast<QSpinBox *>( editor );

  mDial = qobject_cast<QDial *>( editor );
  mSlider = qobject_cast<QSlider *>( editor );
  mQgsDial = qobject_cast<QgsDial *>( editor );
  mQgsSlider = qobject_cast<QgsSlider *>( editor );

  const bool allowNull = config( QStringLiteral( "AllowNull" ), true ).toBool();

  QVariant min( config( QStringLiteral( "Min" ) ) );
  QVariant max( config( QStringLiteral( "Max" ) ) );
  QVariant step( config( QStringLiteral( "Step" ) ) );
  const QVariant precision( config( QStringLiteral( "Precision" ) ) );

  if ( mDoubleSpinBox )
  {
    const double stepval = step.isValid() ? step.toDouble() : 1.0;
    double minval = min.isValid() ? min.toDouble() : std::numeric_limits<double>::lowest();
    const double maxval  = max.isValid() ? max.toDouble() : std::numeric_limits<double>::max();

    const QgsField field = layer()->fields().at( fieldIdx() );
    // we use the double spin box for long long fields in order to get sufficient range of min/max values
    const int precisionval = field.type() == QVariant::LongLong ? 0 : ( precision.isValid() ? precision.toInt() : field.precision() );

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
    if ( config( QStringLiteral( "Suffix" ) ).isValid() )
      mDoubleSpinBox->setSuffix( config( QStringLiteral( "Suffix" ) ).toString() );

    connect( mDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ),
    this, [ = ]( double ) { emitValueChanged(); } );
  }
  else if ( mIntSpinBox )
  {
    QgsSpinBox *qgsWidget = qobject_cast<QgsSpinBox *>( mIntSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    int minval =  min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest();
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
    if ( config( QStringLiteral( "Suffix" ) ).isValid() )
      mIntSpinBox->setSuffix( config( QStringLiteral( "Suffix" ) ).toString() );
  }
  else
  {
    ( void )field().convertCompatible( min );
    ( void )field().convertCompatible( max );
    ( void )field().convertCompatible( step );
    if ( mQgsDial )
      setupIntEditor( min, max, step, mQgsDial, this );
    else if ( mQgsSlider )
      setupIntEditor( min, max, step, mQgsSlider, this );
    else if ( mDial )
      setupIntEditor( min, max, step, mDial, this );
    else if ( mSlider )
      setupIntEditor( min, max, step, mSlider, this );
  }
}

bool QgsRangeWidgetWrapper::valid() const
{
  return mSlider || mDial || mQgsDial || mQgsSlider || mIntSpinBox || mDoubleSpinBox;
}

void QgsRangeWidgetWrapper::valueChangedVariant( const QVariant &v )
{
  if ( v.type() == QVariant::Int )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toInt() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toInt() );
  }
  else if ( v.type() == QVariant::LongLong )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toLongLong() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toLongLong() );
  }
  else if ( v.type() == QVariant::Double )
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

  if ( mDoubleSpinBox )
  {
    const QVariant::Type fieldType = field().type();
    switch ( fieldType )
    {
      case QVariant::Double:
        value = mDoubleSpinBox->value();
        break;

      case QVariant::LongLong:
        value = static_cast< long long >( mDoubleSpinBox->value() );
        break;

      default:
        break;
    }

    if ( value == mDoubleSpinBox->minimum() && config( QStringLiteral( "AllowNull" ), true ).toBool() )
    {
      value = QVariant( fieldType );
    }
  }
  else if ( mIntSpinBox )
  {
    value = mIntSpinBox->value();
    if ( value == mIntSpinBox->minimum() && config( QStringLiteral( "AllowNull" ), true ).toBool() )
    {
      value = QVariant( field().type() );
    }
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
  if ( mDoubleSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( QStringLiteral( "AllowNull" ), true ).toBool() )
    {
      mDoubleSpinBox->setValue( mDoubleSpinBox->minimum() );
    }
    else
    {
      mDoubleSpinBox->setValue( value.toDouble() );
    }
  }

  if ( mIntSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( QStringLiteral( "AllowNull" ), true ).toBool() )
    {
      mIntSpinBox->setValue( mIntSpinBox->minimum() );
    }
    else
    {
      mIntSpinBox->setValue( value.toInt() );
    }
  }

  if ( mQgsDial )
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
