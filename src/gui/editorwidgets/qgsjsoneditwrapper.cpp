/***************************************************************************
    qgsjsoneditwrapper.cpp
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditwrapper.h"
#include "moc_qgsjsoneditwrapper.cpp"

#include "qgsjsoneditwidget.h"

QgsJsonEditWrapper::QgsJsonEditWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
}

QVariant QgsJsonEditWrapper::value() const
{
  if ( !mJsonEditWidget )
    return QVariant();

  return mJsonEditWidget->property( RAW_VALUE_PROPERTY.toUtf8().data() );
}

QWidget *QgsJsonEditWrapper::createWidget( QWidget *parent )
{
  QgsJsonEditWidget *jsonEditWidget = new QgsJsonEditWidget( parent );
  jsonEditWidget->setView( static_cast<QgsJsonEditWidget::View>( config( QStringLiteral( "DefaultView" ) ).toInt() ) );
  jsonEditWidget->setFormatJsonMode( static_cast<QgsJsonEditWidget::FormatJson>( config( QStringLiteral( "FormatJson" ) ).toInt() ) );
  return jsonEditWidget;
}

void QgsJsonEditWrapper::initWidget( QWidget *editor )
{
  mJsonEditWidget = qobject_cast<QgsJsonEditWidget *>( editor );
  if ( !mJsonEditWidget )
  {
    mJsonEditWidget = new QgsJsonEditWidget( editor );
  }
}

bool QgsJsonEditWrapper::valid() const
{
  return ( mJsonEditWidget );
}

void QgsJsonEditWrapper::showIndeterminateState()
{
  if ( !mJsonEditWidget )
    return;

  mJsonEditWidget->blockSignals( true );
  mJsonEditWidget->setJsonText( QStringLiteral( "<mixed values>" ) );
  mJsonEditWidget->blockSignals( false );
}

void QgsJsonEditWrapper::setFeature( const QgsFeature &feature )
{
  setFormFeature( feature );
  setValue( feature.attribute( fieldIdx() ) );
}

void QgsJsonEditWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( !mJsonEditWidget )
    return;

  mJsonEditWidget->setProperty( RAW_VALUE_PROPERTY.toUtf8().data(), value );
  mJsonEditWidget->setJsonText( field().displayString( value ) );
}

void QgsJsonEditWrapper::setEnabled( bool enabled )
{
  // No need to disable JsonEditWidget as it is already read only
  Q_UNUSED( enabled )
}
