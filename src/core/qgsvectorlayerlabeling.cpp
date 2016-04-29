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


QgsAbstractVectorLayerLabeling::~QgsAbstractVectorLayerLabeling()
{
}

QgsAbstractVectorLayerLabeling* QgsAbstractVectorLayerLabeling::create( const QDomElement& element )
{
  if ( element.attribute( "type" ) == "rule-based" )
  {
    return QgsRuleBasedLabeling::create( element );
  }
  else
  {
    // default
    return new QgsVectorLayerSimpleLabeling;
  }
}

QgsVectorLayerLabelProvider* QgsVectorLayerSimpleLabeling::provider( QgsVectorLayer* layer ) const
{
  if ( layer->customProperty( "labeling" ).toString() == QLatin1String( "pal" ) && layer->labelsEnabled() )
    return new QgsVectorLayerLabelProvider( layer, QString(), false );

  return nullptr;
}

QString QgsVectorLayerSimpleLabeling::type() const
{
  return "simple";
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument& doc ) const
{
  // all configuration is kept in layer custom properties (for compatibility)
  QDomElement elem = doc.createElement( "labeling" );
  elem.setAttribute( "type", "simple" );
  return elem;
}

QgsPalLayerSettings QgsVectorLayerSimpleLabeling::settings( QgsVectorLayer* layer, const QString& providerId ) const
{
  if ( providerId.isEmpty() )
    return QgsPalLayerSettings::fromLayer( layer );
  else
    return QgsPalLayerSettings();
}
