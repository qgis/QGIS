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

#include "qgsjsoneditwidget.h"
#include "qgsfields.h"
#include "qgsfieldvalidator.h"
#include "qgsfilterlineedit.h"
#include "qgsapplication.h"
#include "qgsjsonutils.h"
#include "qgsmessagebar.h"
#include "qgslogger.h"

#include <QSettings>
#include <nlohmann/json.hpp>

QgsJsonEditWrapper::QgsJsonEditWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
}

QVariant QgsJsonEditWrapper::value() const
{
  if ( !mJsonEditWidget )
    return QVariant();

  return QVariant( mJsonEditWidget->jsonText() );
}

QWidget *QgsJsonEditWrapper::createWidget( QWidget *parent )
{
  QgsJsonEditWidget *jsonEditWidget = new QgsJsonEditWidget( parent );
  jsonEditWidget->setView( static_cast<QgsJsonEditWidget::View>( config( QStringLiteral( "DefaultView" ) ).toInt() ) );
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
  return ( mJsonEditWidget && mJsonEditWidget->validJson() );
}

void QgsJsonEditWrapper::showIndeterminateState()
{
  if ( !mJsonEditWidget )
    return;

  mJsonEditWidget->blockSignals( true );
  //note - this is deliberately a zero length string, not a null string!
  mJsonEditWidget->setJsonText( QStringLiteral( "" ) );
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

  //restore placeholder text, which may have been removed by showIndeterminateState()
  mJsonEditWidget->setJsonText( field().displayString( value ) );
}

void QgsJsonEditWrapper::setEnabled( bool enabled )
{
  if ( mJsonEditWidget )
    mJsonEditWidget->setEnabled( enabled );
}

