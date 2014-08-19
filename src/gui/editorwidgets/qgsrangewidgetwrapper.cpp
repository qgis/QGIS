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

#include "qgsrangewidgetwrapper.h"

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
    switch ( layer()->pendingFields()[fieldIdx()].type() )
    {
      case QVariant::Double:
      {
        QDoubleSpinBox* spin  = new QDoubleSpinBox( parent );
        int precision = layer()->pendingFields()[fieldIdx()].precision();
        if ( precision > 0 )
        {
          spin->setDecimals( layer()->pendingFields()[fieldIdx()].precision() );
        }
        editor = spin;
        break;
      }
      case QVariant::Int:
      case QVariant::LongLong:
      default:
        editor = new QSpinBox( parent );
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

  if ( mDoubleSpinBox )
  {
    mDoubleSpinBox->setMinimum( config( "Min" ).toDouble() );
    mDoubleSpinBox->setMaximum( config( "Max" ).toDouble() );
    mDoubleSpinBox->setSingleStep( config( "Step" ).toDouble() );
    connect( mDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( valueChanged( double ) ) );
  }

  if ( mIntSpinBox )
  {
    mIntSpinBox->setMinimum( config( "Min" ).toInt() );
    mIntSpinBox->setMaximum( config( "Max" ).toInt() );
    mIntSpinBox->setSingleStep( config( "Step" ).toInt() );
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
  }
  else if ( mIntSpinBox )
  {
    value = mIntSpinBox->value();
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
    mDoubleSpinBox->setValue( value.toDouble() );
  }

  if ( mIntSpinBox )
  {
    mIntSpinBox->setValue( value.toInt() );
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

