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
#include "moc_qgsmaplayerserverproperties.cpp"
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

bool QgsServerMetadataUrlProperties::operator==( const QgsServerMetadataUrlProperties &other ) const
{
  return mMetadataUrls == other.mMetadataUrls;
}

bool QgsServerMetadataUrlProperties::operator!=( const QgsServerMetadataUrlProperties &other ) const
{
  return !( *this == other );
}

bool QgsServerWmsDimensionProperties::WmsDimensionInfo::operator==( const WmsDimensionInfo &other ) const
{
  return name == other.name
         && fieldName == other.fieldName
         && endFieldName == other.endFieldName
         && units == other.units
         && unitSymbol == other.unitSymbol
         && defaultDisplayType == other.defaultDisplayType
         && referenceValue == other.referenceValue;
}

bool QgsServerWmsDimensionProperties::WmsDimensionInfo::operator!=( const WmsDimensionInfo &other ) const
{
  return !( *this == other );
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
  if ( !mMetadataUrls.empty() )
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
}

// QgsServerWmsDimensionProperties

bool QgsServerWmsDimensionProperties::operator==( const QgsServerWmsDimensionProperties &other ) const
{
  return mWmsDimensions == other.mWmsDimensions;
}

bool QgsServerWmsDimensionProperties::operator!=( const QgsServerWmsDimensionProperties &other ) const
{
  return !( *this == other );
}

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
  for ( const QgsServerWmsDimensionProperties::WmsDimensionInfo &dim : std::as_const( mWmsDimensions ) )
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
  if ( !layer() || layer()->type() != Qgis::LayerType::Vector )
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

  properties->setShortName( mShortName );
  properties->setTitle( mTitle );
  properties->setAbstract( mAbstract );
  properties->setKeywordList( mKeywordList );
  properties->setDataUrl( mDataUrl );
  properties->setDataUrlFormat( mDataUrlFormat );
  properties->setAttribution( mAttribution );
  properties->setAttributionUrl( mAttributionUrl );
  properties->setLegendUrl( mLegendUrl );
  properties->setLegendUrlFormat( mLegendUrlFormat );
}

bool QgsMapLayerServerProperties::operator==( const QgsMapLayerServerProperties &other ) const
{
  return QgsServerMetadataUrlProperties::operator==( other )
         && QgsServerWmsDimensionProperties::operator==( other )
         && mShortName == other.mShortName
         && mTitle == other.mTitle
         && mAbstract == other.mAbstract
         && mKeywordList == other.mKeywordList
         && mDataUrl == other.mDataUrl
         && mDataUrlFormat == other.mDataUrlFormat
         && mAttribution == other.mAttribution
         && mAttributionUrl == other.mAttributionUrl
         && mLegendUrl == other.mLegendUrl
         && mLegendUrlFormat == other.mLegendUrlFormat;
}

bool QgsMapLayerServerProperties::operator!=( const QgsMapLayerServerProperties &other ) const
{
  return !( *this == other );
}

void QgsMapLayerServerProperties::reset() // cppcheck-suppress duplInheritedMember
{
  QgsServerMetadataUrlProperties::reset();
  QgsServerWmsDimensionProperties::reset();
}

void QgsMapLayerServerProperties::readXml( const QDomNode &layerNode ) // cppcheck-suppress duplInheritedMember
{
  QgsServerMetadataUrlProperties::readXml( layerNode );
  if ( metadataUrls().isEmpty() )
  {
    // metadataUrl is still empty, maybe it's a QGIS Project < 3.22
    // keep for legacy
    const QDomElement metaUrlElem = layerNode.firstChildElement( QStringLiteral( "metadataUrl" ) );
    if ( !metaUrlElem.isNull() )
    {
      const QString url = metaUrlElem.text();
      const QString type = metaUrlElem.attribute( QStringLiteral( "type" ), QString() );
      const QString format = metaUrlElem.attribute( QStringLiteral( "format" ), QString() );
      const QgsMapLayerServerProperties::MetadataUrl newItem( url, type, format );
      setMetadataUrls( QList<QgsMapLayerServerProperties::MetadataUrl>() << newItem );
    }
  }

  QgsServerWmsDimensionProperties::readXml( layerNode );

  //short name
  const QDomElement shortNameElem = layerNode.firstChildElement( QStringLiteral( "shortname" ) );
  if ( !shortNameElem.isNull() )
  {
    mShortName = shortNameElem.text();
  }

  //title
  const QDomElement titleElem = layerNode.firstChildElement( QStringLiteral( "title" ) );
  if ( !titleElem.isNull() )
  {
    mTitle = titleElem.text();
    mWfsTitle = titleElem.attribute( QStringLiteral( "wfs" ) );
  }

  //abstract
  const QDomElement abstractElem = layerNode.firstChildElement( QStringLiteral( "abstract" ) );
  if ( !abstractElem.isNull() )
  {
    mAbstract = abstractElem.text();
  }

  //keywordList
  const QDomElement keywordListElem = layerNode.firstChildElement( QStringLiteral( "keywordList" ) );
  if ( !keywordListElem.isNull() )
  {
    QStringList kwdList;
    for ( QDomNode n = keywordListElem.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
      const QString keyword = n.toElement().text();
      if ( !keyword.isEmpty() )
        kwdList << keyword;
    }
    mKeywordList = kwdList.join( QLatin1String( ", " ) );
  }

  //dataUrl
  const QDomElement dataUrlElem = layerNode.firstChildElement( QStringLiteral( "dataUrl" ) );
  if ( !dataUrlElem.isNull() )
  {
    mDataUrl = dataUrlElem.text();
    mDataUrlFormat = dataUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  //attribution
  const QDomElement attribElem = layerNode.firstChildElement( QStringLiteral( "attribution" ) );
  if ( !attribElem.isNull() )
  {
    mAttribution = attribElem.text();
    mAttributionUrl = attribElem.attribute( QStringLiteral( "href" ), QString() );
  }

  //legendUrl
  const QDomElement legendUrlElem = layerNode.firstChildElement( QStringLiteral( "legendUrl" ) );
  if ( !legendUrlElem.isNull() )
  {
    mLegendUrl = legendUrlElem.text();
    mLegendUrlFormat = legendUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }
}

void QgsMapLayerServerProperties::writeXml( QDomNode &layerNode, QDomDocument &document ) const // cppcheck-suppress duplInheritedMember
{
  QgsServerMetadataUrlProperties::writeXml( layerNode, document );
  QgsServerWmsDimensionProperties::writeXml( layerNode, document );

  // layer short name
  if ( !mShortName.isEmpty() )
  {
    QDomElement layerShortName = document.createElement( QStringLiteral( "shortname" ) );
    const QDomText layerShortNameText = document.createTextNode( mShortName );
    layerShortName.appendChild( layerShortNameText );
    layerNode.appendChild( layerShortName );
  }

  // layer title
  if ( !mTitle.isEmpty() )
  {
    QDomElement layerTitle = document.createElement( QStringLiteral( "title" ) );
    const QDomText layerTitleText = document.createTextNode( mTitle );
    layerTitle.appendChild( layerTitleText );

    if ( mTitle != mWfsTitle )
    {
      layerTitle.setAttribute( "wfs",  mWfsTitle );
    }

    layerNode.appendChild( layerTitle );
  }

  // layer abstract
  if ( !mAbstract.isEmpty() )
  {
    QDomElement layerAbstract = document.createElement( QStringLiteral( "abstract" ) );
    const QDomText layerAbstractText = document.createTextNode( mAbstract );
    layerAbstract.appendChild( layerAbstractText );
    layerNode.appendChild( layerAbstract );
  }

  // layer keyword list
  const QStringList keywordStringList = mKeywordList.split( ',', Qt::SkipEmptyParts );
  if ( !keywordStringList.isEmpty() )
  {
    QDomElement layerKeywordList = document.createElement( QStringLiteral( "keywordList" ) );
    for ( int i = 0; i < keywordStringList.size(); ++i )
    {
      QDomElement layerKeywordValue = document.createElement( QStringLiteral( "value" ) );
      const QDomText layerKeywordText = document.createTextNode( keywordStringList.at( i ).trimmed() );
      layerKeywordValue.appendChild( layerKeywordText );
      layerKeywordList.appendChild( layerKeywordValue );
    }
    layerNode.appendChild( layerKeywordList );
  }

  // layer dataUrl
  if ( !mDataUrl.isEmpty() )
  {
    QDomElement layerDataUrl = document.createElement( QStringLiteral( "dataUrl" ) );
    const QDomText layerDataUrlText = document.createTextNode( mDataUrl );
    layerDataUrl.appendChild( layerDataUrlText );
    layerDataUrl.setAttribute( QStringLiteral( "format" ), mDataUrlFormat );
    layerNode.appendChild( layerDataUrl );
  }

  // layer legendUrl
  if ( !mLegendUrl.isEmpty() )
  {
    QDomElement layerLegendUrl = document.createElement( QStringLiteral( "legendUrl" ) );
    const QDomText layerLegendUrlText = document.createTextNode( mLegendUrl );
    layerLegendUrl.appendChild( layerLegendUrlText );
    layerLegendUrl.setAttribute( QStringLiteral( "format" ), mLegendUrlFormat );
    layerNode.appendChild( layerLegendUrl );
  }

  // layer attribution
  if ( !mAttribution.isEmpty() )
  {
    QDomElement layerAttribution = document.createElement( QStringLiteral( "attribution" ) );
    const QDomText layerAttributionText = document.createTextNode( mAttribution );
    layerAttribution.appendChild( layerAttributionText );
    layerAttribution.setAttribute( QStringLiteral( "href" ), mAttributionUrl );
    layerNode.appendChild( layerAttribution );
  }
}
