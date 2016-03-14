/***************************************************************************
    qgseditorwidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfield.h"

#include <QWidget>

QgsEditorWidgetWrapper::QgsEditorWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsWidgetWrapper( vl, editor, parent )
    , mFieldIdx( fieldIdx )
{
  connect( this, SIGNAL( valueChanged( QVariant ) ), this, SLOT( onValueChanged( QVariant ) ) );
}

int QgsEditorWidgetWrapper::fieldIdx() const
{
  return mFieldIdx;
}

QgsField QgsEditorWidgetWrapper::field() const
{
  if ( mFieldIdx < layer()->fields().count() )
    return layer()->fields().at( mFieldIdx );
  else
    return QgsField();
}

QVariant QgsEditorWidgetWrapper::defaultValue() const
{
  return layer()->dataProvider()->defaultValue( mFieldIdx );
}

QgsEditorWidgetWrapper* QgsEditorWidgetWrapper::fromWidget( QWidget* widget )
{
  return qobject_cast<QgsEditorWidgetWrapper*>( widget->property( "EWV2Wrapper" ).value<QgsWidgetWrapper*>() );
}

void QgsEditorWidgetWrapper::setEnabled( bool enabled )
{
  QWidget* wdg = widget();
  if ( wdg )
  {
    wdg->setEnabled( enabled );
  }
}

void QgsEditorWidgetWrapper::setFeature( const QgsFeature& feature )
{
  setValue( feature.attribute( mFieldIdx ) );
  onValueChanged( value() );
}

void QgsEditorWidgetWrapper::valueChanged( const QString& value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( int value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( double value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( bool value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( qlonglong value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged()
{
  emit valueChanged( value() );
}

void QgsEditorWidgetWrapper::updateConstraintsOk( bool constraintStatus )
{
  if ( constraintStatus )
  {
    widget()->setStyleSheet( "" );
  }
  else
  {
    widget()->setStyleSheet( "QWidget{ background-color: '#dd7777': }" );
  }
}

void QgsEditorWidgetWrapper::onValueChanged( const QVariant& value )
{
  if ( layer()->editFormConfig()->notNull( mFieldIdx ) )
  {
    if ( value.isNull() != mIsNull )
    {
      updateConstraintsOk( value.isNull() );
      emit constraintStatusChanged( "NotNull", !value.isNull() );
      mIsNull = value.isNull();
    }
  }
}
