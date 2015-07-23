/***************************************************************************
    qgsrangewidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

QgsRangeWidgetWrapper::QgsRangeWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mIntSpinBox( 0 )
    , mDoubleSpinBox( 0 )
    , mSlider( 0 )
    , mDial( 0 )
    , mQgsSlider( 0 )
    , mQgsDial( 0 )
{
}

QWidget* QgsRangeWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* editor = 0;

  if ( config( "Style" ).toString() == "Dial" )
  {
    editor = new QgsDial( parent );
  }
  else if ( config( "Style" ).toString() == "Slider" )
  {
    editor = new QgsSlider( Qt::Horizontal, parent );
  }
  else
  {
    QgsDebugMsg( QString( "%1" ).arg(( int )layer()->pendingFields()[fieldIdx()].type() ) );
    switch ( layer()->pendingFields()[fieldIdx()].type() )
    {
      case QVariant::Double:
      {
        editor = new QgsDoubleSpinBox( parent );
        break;
      }
      case QVariant::Int:
      case QVariant::LongLong:
      default:
        editor = new QgsSpinBox( parent );
        break;
    }
  }

  return editor;
}

void QgsRangeWidgetWrapper::initWidget( QWidget* editor )
{
  mDoubleSpinBox = qobject_cast<QDoubleSpinBox*>( editor );
  mIntSpinBox = qobject_cast<QSpinBox*>( editor );
  mDial = qobject_cast<QDial*>( editor );
  mSlider = qobject_cast<QSlider*>( editor );
  mQgsDial = qobject_cast<QgsDial*>( editor );
  mQgsSlider = qobject_cast<QgsSlider*>( editor );

  bool allowNull = config( "AllowNull" ).toBool();

  QVariant min( config( "Min" ) );
  QVariant max( config( "Max" ) );
  QVariant step( config( "Step" ) );

  if ( mDoubleSpinBox )
  {
    // set the precision if field is integer
    int precision = layer()->pendingFields()[fieldIdx()].precision();
    if ( precision > 0 )
    {
      mDoubleSpinBox->setDecimals( layer()->pendingFields()[fieldIdx()].precision() );
    }

    double minval = min.toDouble();
    double stepval = step.toDouble();
    QgsDoubleSpinBox* qgsWidget = dynamic_cast<QgsDoubleSpinBox*>( mDoubleSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    if ( allowNull )
    {
      if ( precision > 0 )
      {
        minval -= 10 ^ -precision;
      }
      else
      {
        minval -= stepval;
      }
      mDoubleSpinBox->setValue( minval );
      mDoubleSpinBox->setSpecialValueText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    if ( min.isValid() )
      mDoubleSpinBox->setMinimum( min.toDouble() );
    if ( max.isValid() )
      mDoubleSpinBox->setMaximum( max.toDouble() );
    if ( step.isValid() )
      mDoubleSpinBox->setSingleStep( step.toDouble() );
    if ( config( "Suffix" ).isValid() )
      mDoubleSpinBox->setSuffix( config( "Suffix" ).toString() );

    connect( mDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( valueChanged( double ) ) );
  }

  if ( mIntSpinBox )
  {
    int minval = min.toInt();
    int stepval = step.toInt();
    QgsSpinBox* qgsWidget = dynamic_cast<QgsSpinBox*>( mIntSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    if ( allowNull )
    {
      minval -= stepval;
      mIntSpinBox->setValue( minval );
      mIntSpinBox->setSpecialValueText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    if ( min.isValid() )
      mIntSpinBox->setMinimum( min.toInt() );
    if ( max.isValid() )
      mIntSpinBox->setMaximum( max.toInt() );
    if ( step.isValid() )
      mIntSpinBox->setSingleStep( step.toInt() );
    if ( config( "Suffix" ).isValid() )
      mIntSpinBox->setSuffix( config( "Suffix" ).toString() );
    connect( mIntSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }


  if ( mQgsDial || mQgsSlider )
  {
    field().convertCompatible( min );
    field().convertCompatible( max );
    field().convertCompatible( step );

    if ( mQgsSlider )
    {
      if ( min.isValid() )
        mQgsSlider->setMinimum( min );
      if ( max.isValid() )
        mQgsSlider->setMaximum( max );
      if ( step.isValid() )
        mQgsSlider->setSingleStep( step );
    }

    if ( mQgsDial )
    {
      if ( min.isValid() )
        mQgsDial->setMinimum( min );
      if ( max.isValid() )
        mQgsDial->setMaximum( max );
      if ( step.isValid() )
        mQgsDial->setSingleStep( step );
    }

    connect( editor, SIGNAL( valueChanged( QVariant ) ), this, SLOT( valueChanged( QVariant ) ) );
  }
  else if ( mDial )
  {
    if ( min.isValid() )
      mDial->setMinimum( min.toInt() );
    if ( max.isValid() )
      mDial->setMaximum( max.toInt() );
    if ( step.isValid() )
      mDial->setSingleStep( step.toInt() );
    connect( mDial, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }
  else if ( mSlider )
  {
    if ( min.isValid() )
      mSlider->setMinimum( min.toInt() );
    if ( max.isValid() )
      mSlider->setMaximum( max.toInt() );
    if ( step.isValid() )
      mSlider->setSingleStep( step.toInt() );
    connect( mSlider, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }
}

bool QgsRangeWidgetWrapper::valid()
{
  return mSlider || mDial || mQgsDial || mQgsSlider || mIntSpinBox || mDoubleSpinBox;
}

void QgsRangeWidgetWrapper::valueChanged( QVariant v )
{
  if ( v.type() == QVariant::Int )
    valueChanged( v.toInt() );
  if ( v.type() == QVariant::Double )
    valueChanged( v.toDouble() );
}

QVariant QgsRangeWidgetWrapper::value()
{
  QVariant value;

  if ( mDoubleSpinBox )
  {
    value = mDoubleSpinBox->value();
    if ( value == mDoubleSpinBox->minimum() && config( "AllowNull" ).toBool() )
    {
      value = QVariant( field().type() );
    }
  }
  else if ( mIntSpinBox )
  {
    value = mIntSpinBox->value();
    if ( value == mIntSpinBox->minimum() && config( "AllowNull" ).toBool() )
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

void QgsRangeWidgetWrapper::setValue( const QVariant& value )
{
  if ( mDoubleSpinBox )
  {
    if ( value.isNull() && config( "AllowNull" ).toBool() )
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
    if ( value.isNull() && config( "AllowNull" ).toBool() )
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
