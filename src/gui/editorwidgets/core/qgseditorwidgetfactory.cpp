/***************************************************************************
    qgseditorwidgetfactory.cpp
     --------------------------------------
    Date                 : 21.4.2013
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

#include "qgseditorwidgetfactory.h"
#include "qgsdefaultsearchwidgetwrapper.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"

#include <QSettings>

class QgsDefaultSearchWidgetWrapper;

QgsEditorWidgetFactory::QgsEditorWidgetFactory( const QString& name )
    : mName( name )
{
}

QgsEditorWidgetFactory::~QgsEditorWidgetFactory()
{
}

/**
 * By default a simple QgsFilterLineEdit is returned as search widget.
 * Override in own factory to get something different than the default.
 */
QgsSearchWidgetWrapper* QgsEditorWidgetFactory::createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent );
}

QString QgsEditorWidgetFactory::name()
{
  return mName;
}

QVariantMap QgsEditorWidgetFactory::readEditorConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  return readConfig( configElement, layer, fieldIdx );
}

void QgsEditorWidgetFactory::writeConfig( const QVariantMap& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( config );
  Q_UNUSED( configElement );
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
}

QVariantMap QgsEditorWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( configElement );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  return QVariantMap();
}

unsigned int QgsEditorWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  return 5;
}

void QgsEditorWidgetFactory::config2xml( const QVariantMap& config, QDomElement& configElement, const QString& fieldName )
{
  const QVariant value = config.value( fieldName );
  if ( value.isValid() )
  {
    if ( value.type() == QVariant::Bool )
    {
      configElement.setAttribute( fieldName, value.toBool() ? "1" : "0" );
    }
    else
    {
      configElement.setAttribute( fieldName, value.toString() );
    }
  }
}

void QgsEditorWidgetFactory::xml2config( const QDomElement& configElement, QVariantMap& config, const QString& fieldName )
{
  const QString value = configElement.attribute( fieldName );
  if ( !value.isNull() )
  {
    config.insert( fieldName, value );
  }
}
