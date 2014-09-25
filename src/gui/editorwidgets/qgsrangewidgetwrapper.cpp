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
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mIntSpinBox( 0 )
    , mDoubleSpinBox( 0 )
    , mSlider( 0 )
    , mDial( 0 )
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

  bool allowNull = config( "AllowNull" ).toBool();

  if ( mDoubleSpinBox )
  {
    // set the precision if field is integer
    int precision = layer()->pendingFields()[fieldIdx()].precision();
    if ( precision > 0 )
    {
      mDoubleSpinBox->setDecimals( layer()->pendingFields()[fieldIdx()].precision() );
    }

    double min = config( "Min" ).toDouble();
    double step = config( "Step" ).toDouble();
    QgsDoubleSpinBox* qgsWidget = dynamic_cast<QgsDoubleSpinBox*>( mDoubleSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    if ( allowNull )
    {
      if ( precision > 0 )
      {
        min -= 10 ^ -precision;
      }
      else
      {
        min -= step;
      }
      mDoubleSpinBox->setValue( min );
      mDoubleSpinBox->setSpecialValueText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    mDoubleSpinBox->setMinimum( min );
    mDoubleSpinBox->setMaximum( config( "Max" ).toDouble() );
    mDoubleSpinBox->setSingleStep( step );
    if ( config( "Suffix" ).isValid() )
    {
      mDoubleSpinBox->setSuffix( config( "Suffix" ).toString() );
    }
    connect( mDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( valueChanged( double ) ) );
  }

  if ( mIntSpinBox )
  {
    int min = config( "Min" ).toInt();
    int step = config( "Step" ).toInt();
    QgsSpinBox* qgsWidget = dynamic_cast<QgsSpinBox*>( mIntSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    if ( allowNull )
    {
      min -= step;
      mIntSpinBox->setValue( min );
      mIntSpinBox->setSpecialValueText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }
    mIntSpinBox->setMinimum( min );
    mIntSpinBox->setMaximum( config( "Max" ).toInt() );
    mIntSpinBox->setSingleStep( step );
    if ( config( "Suffix" ).isValid() )
    {
      mIntSpinBox->setSuffix( config( "Suffix" ).toString() );
    }
    connect( mIntSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }

  if ( mDial )
  {
    mDial->setMinimum( config( "Min" ).toInt() );
    mDial->setMaximum( config( "Max" ).toInt() );
    mDial->setSingleStep( config( "Step" ).toInt() );
    connect( mDial, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }

  if ( mSlider )
  {
    mSlider->setMinimum( config( "Min" ).toInt() );
    mSlider->setMaximum( config( "Max" ).toInt() );
    mSlider->setSingleStep( config( "Step" ).toInt() );
    connect( mSlider, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
  }
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
  if ( mDial )
  {
    mDial->setValue( value.toInt() );
  }
  if ( mSlider )
  {
    mSlider->setValue( value.toInt() );
  }
}

