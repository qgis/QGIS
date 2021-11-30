/***************************************************************************
                             qgsmaplayerserverproperties.cpp
                              ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerserverproperties.h"
#include "qgsmaplayer.h"
#include "vector/qgsvectorlayer.h"

#include <QDomNode>

// QgsServerMetadataUrlProperties

bool QgsServerMetadataUrlProperties::MetadataUrl::operator==( const QgsServerMetadataUrlProperties::MetadataUrl &other ) const
{
  return url == other.url &&
         type == other.type &&
         format == other.format;
}

void QgsServerMetadataUrlProperties::copyTo( QgsServerMetadataUrlProperties *properties ) const
{
  properties->setMetadataUrls( metadataUrls() );
}

void QgsServerMetadataUrlProperties::reset()
{
  mMetadataUrls.clear();
}

void QgsServerMetadataUrlProperties::readXml( const QDomNode &layer_node )
{
  QDomElement element = layer_node.namedItem( QStringLiteral( "metadataUrls" ) ).toElement();
  mMetadataUrls.clear();
  const QDomNodeList el = element.elementsByTagName( QStringLiteral( "metadataUrl" ) );
  for ( int i = 0; i < el.size(); i++ )
  {
    element = el.at( i ).toElement();
    QgsMapLayerServerProperties::MetadataUrl oneUrl;
    oneUrl.type = element.attribute( QStringLiteral( "type" ) );
    oneUrl.format = element.attribute( QStringLiteral( "format" ) );
    oneUrl.url = element.text();
    addMetadataUrl( oneUrl );
  }
}

void QgsServerMetadataUrlProperties::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  QDomElement urls = document.createElement( QStringLiteral( "metadataUrls" ) );
  for ( const QgsMapLayerServerProperties::MetadataUrl &url : mMetadataUrls )
  {
    QDomElement urlElement = document.createElement( QStringLiteral( "metadataUrl" ) );
    urlElement.setAttribute( QStringLiteral( "type" ), url.type );
    urlElement.setAttribute( QStringLiteral( "format" ), url.format );
    urlElement.appendChild( document.createTextNode( url.url ) );
    urls.appendChild( urlElement );
  }
  layer_node.appendChild( urls );
}

// QgsServerWmsDimensionProperties

void QgsServerWmsDimensionProperties::copyTo( QgsServerWmsDimensionProperties *properties ) const
{
  properties->setWmsDimensions( wmsDimensions() );
}

void QgsServerWmsDimensionProperties::reset()
{
  mWmsDimensions.clear();
}

void QgsServerWmsDimensionProperties::setWmsDimensions( const QList<QgsServerWmsDimensionProperties::WmsDimensionInfo> &dimensions )
{
  mWmsDimensions = dimensions;
}

QMap<int, QString> QgsServerWmsDimensionProperties::wmsDimensionDefaultDisplayLabels()
{
  QMap<int, QString> labels;
  labels[QgsServerWmsDimensionProperties::WmsDimensionInfo::AllValues] = QObject::tr( "All values" );
  labels[QgsServerWmsDimensionProperties::WmsDimensionInfo::MinValue] = QObject::tr( "Min value" );
  labels[QgsServerWmsDimensionProperties::WmsDimensionInfo::MaxValue] = QObject::tr( "Max value" );
  labels[QgsServerWmsDimensionProperties::WmsDimensionInfo::ReferenceValue] = QObject::tr( "Reference value" );
  return labels;
}

bool QgsServerWmsDimensionProperties::addWmsDimension( const QgsServerWmsDimensionProperties::WmsDimensionInfo &wmsDimInfo )
{
  for ( const QgsServerWmsDimensionProperties::WmsDimensionInfo &dim : mWmsDimensions )
  {
    if ( dim.name == wmsDimInfo.name )
    {
      return false;
    }
  }
  mWmsDimensions.append( wmsDimInfo );
  return true;
}

bool QgsServerWmsDimensionProperties::removeWmsDimension( const QString &wmsDimName )
{
  for ( int i = 0; i < mWmsDimensions.size(); ++i )
  {
    if ( mWmsDimensions[ i ].name == wmsDimName )
    {
      mWmsDimensions.removeAt( i );
      return true;
    }
  }
  return false;
}

const QList< QgsServerWmsDimensionProperties::WmsDimensionInfo > QgsServerWmsDimensionProperties::wmsDimensions() const
{
  return mWmsDimensions;
}

void QgsServerWmsDimensionProperties::readXml( const QDomNode &layer_node )
{
  reset();

  // Apply only for vector layers
  if ( layer()->type() != QgsMapLayerType::VectorLayer )
    return;

  const QgsFields fields = static_cast<const QgsVectorLayer *>( layer() )->fields();
  // QGIS Server WMS Dimensions
  const QDomNode wmsDimsNode = layer_node.namedItem( QStringLiteral( "wmsDimensions" ) );
  if ( wmsDimsNode.isNull() )
  {
    return;
  }
  const QDomElement wmsDimsElem = wmsDimsNode.toElement();
  const QDomNodeList wmsDimsList = wmsDimsElem.elementsByTagName( QStringLiteral( "dimension" ) );
  for ( int i = 0; i < wmsDimsList.size(); ++i )
  {
    const QDomElement dimElem = wmsDimsList.at( i ).toElement();
    const QString dimName = dimElem.attribute( QStringLiteral( "name" ) );
    const QString dimFieldName = dimElem.attribute( QStringLiteral( "fieldName" ) );
    // check field name
    const int dimFieldNameIndex = fields.indexOf( dimFieldName );
    if ( dimFieldNameIndex == -1 )
    {
      continue;
    }
    QVariant dimRefValue;
    const int dimDefaultDisplayType = dimElem.attribute( QStringLiteral( "defaultDisplayType" ) ).toInt();
    if ( dimDefaultDisplayType == QgsServerWmsDimensionProperties::WmsDimensionInfo::AllValues )
    {
      const QString dimRefValueStr = dimElem.attribute( QStringLiteral( "referenceValue" ) );
      if ( !dimRefValueStr.isEmpty() )
      {
        const QgsField dimField = fields.at( dimFieldNameIndex );
        dimRefValue = QVariant( dimRefValueStr );
        if ( !dimField.convertCompatible( dimRefValue ) )
        {
          continue;
        }
      }
    }
    QgsServerWmsDimensionProperties::WmsDimensionInfo dim( dimName, dimFieldName,
        dimElem.attribute( QStringLiteral( "endFieldName" ) ),
        dimElem.attribute( QStringLiteral( "units" ) ),
        dimElem.attribute( QStringLiteral( "unitSymbol" ) ),
        dimDefaultDisplayType, dimRefValue );
    //XXX This add O(n^2) complexity !!!!
    // addWmsDimension( dim );
    // Better to trust the XML:
    mWmsDimensions.append( dim );
  }
}

void QgsServerWmsDimensionProperties::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  // save QGIS Server WMS Dimension definitions
  if ( ! mWmsDimensions.isEmpty() )
  {
    QDomElement wmsDimsElem = document.createElement( QStringLiteral( "wmsDimensions" ) );
    for ( const QgsServerWmsDimensionProperties::WmsDimensionInfo &dim : mWmsDimensions )
    {
      QDomElement dimElem = document.createElement( QStringLiteral( "dimension" ) );
      dimElem.setAttribute( QStringLiteral( "name" ), dim.name );
      dimElem.setAttribute( QStringLiteral( "fieldName" ), dim.fieldName );
      dimElem.setAttribute( QStringLiteral( "endFieldName" ), dim.endFieldName );
      dimElem.setAttribute( QStringLiteral( "units" ), dim.units );
      dimElem.setAttribute( QStringLiteral( "unitSymbol" ), dim.unitSymbol );
      dimElem.setAttribute( QStringLiteral( "defaultDisplayType" ), dim.defaultDisplayType );
      dimElem.setAttribute( QStringLiteral( "referenceValue" ), dim.referenceValue.toString() );
      wmsDimsElem.appendChild( dimElem );
    }
    layer_node.appendChild( wmsDimsElem );
  }
}


// QgsMapLayerServerProperties

QgsMapLayerServerProperties::QgsMapLayerServerProperties( QgsMapLayer *layer )
  : mLayer( layer )
{
}

void QgsMapLayerServerProperties::copyTo( QgsMapLayerServerProperties *properties ) const
{
  QgsServerMetadataUrlProperties::copyTo( properties );
  QgsServerWmsDimensionProperties::copyTo( properties );
}

void QgsMapLayerServerProperties::reset()
{
  QgsServerMetadataUrlProperties::reset();
  QgsServerWmsDimensionProperties::reset();
}

void QgsMapLayerServerProperties::readXml( const QDomNode &layer_node )
{
  QgsServerMetadataUrlProperties::readXml( layer_node );
  QgsServerWmsDimensionProperties::readXml( layer_node );
}

void QgsMapLayerServerProperties::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  QgsServerMetadataUrlProperties::writeXml( layer_node, document );
  QgsServerWmsDimensionProperties::writeXml( layer_node, document );
}

