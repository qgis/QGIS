/***************************************************************************
    qgsvectorlayerlabeling.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayerlabeling.h"

#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"


QgsAbstractVectorLayerLabeling *QgsAbstractVectorLayerLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  QString type = element.attribute( QStringLiteral( "type" ) );
  if ( type == QLatin1String( "rule-based" ) )
  {
    return QgsRuleBasedLabeling::create( element, context );
  }
  else if ( type == QLatin1String( "simple" ) )
  {
    return QgsVectorLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

QgsVectorLayerLabelProvider *QgsVectorLayerSimpleLabeling::provider( QgsVectorLayer *layer ) const
{
  return new QgsVectorLayerLabelProvider( layer, QString(), false, mSettings.get() );
}

QgsVectorLayerSimpleLabeling::QgsVectorLayerSimpleLabeling( const QgsPalLayerSettings &settings )
  : mSettings( new QgsPalLayerSettings( settings ) )
{

}

QString QgsVectorLayerSimpleLabeling::type() const
{
  return QStringLiteral( "simple" );
}

QgsAbstractVectorLayerLabeling *QgsVectorLayerSimpleLabeling::clone() const
{
  return new QgsVectorLayerSimpleLabeling( *mSettings );
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "simple" ) );
  elem.appendChild( mSettings->writeXml( doc, context ) );
  return elem;
}

QgsPalLayerSettings QgsVectorLayerSimpleLabeling::settings( const QString &providerId ) const
{
  Q_UNUSED( providerId );
  return *mSettings;
}

bool QgsVectorLayerSimpleLabeling::requiresAdvancedEffects() const
{
  return mSettings->format().containsAdvancedEffects();
}

QgsVectorLayerSimpleLabeling *QgsVectorLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  QgsPalLayerSettings *settings = nullptr;
  QDomElement settingsElem = element.firstChildElement( QStringLiteral( "settings" ) );
  if ( !settingsElem.isNull() )
  {
    settings = new QgsPalLayerSettings;
    settings->readXml( settingsElem, context );
  }

  return new QgsVectorLayerSimpleLabeling( *settings );
}
