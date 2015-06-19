/***************************************************************************
    qgseditorwidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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
}

int QgsEditorWidgetWrapper::fieldIdx()
{
  return mFieldIdx;
}

QgsField QgsEditorWidgetWrapper::field()
{
  if ( mFieldIdx < layer()->pendingFields().count() )
    return layer()->pendingFields()[mFieldIdx];
  else
    return QgsField();
}

QVariant QgsEditorWidgetWrapper::defaultValue()
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
