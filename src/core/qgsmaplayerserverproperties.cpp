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

#include "moc_qgsmaplayerserverproperties.cpp"

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
  QDomElement element = layer_node.namedItem( u"metadataUrls"_s ).toElement();
  mMetadataUrls.clear();
  const QDomNodeList el = element.elementsByTagName( u"metadataUrl"_s );
  for ( int i = 0; i < el.size(); i++ )
  {
    element = el.at( i ).toElement();
    QgsMapLayerServerProperties::MetadataUrl oneUrl;
    oneUrl.type = element.attribute( u"type"_s );
    oneUrl.format = element.attribute( u"format"_s );
    oneUrl.url = element.text();
    addMetadataUrl( oneUrl );
  }
}

void QgsServerMetadataUrlProperties::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  if ( !mMetadataUrls.empty() )
  {
    QDomElement urls = document.createElement( u"metadataUrls"_s );
    for ( const QgsMapLayerServerProperties::MetadataUrl &url : mMetadataUrls )
    {
      QDomElement urlElement = document.createElement( u"metadataUrl"_s );
      urlElement.setAttribute( u"type"_s, url.type );
      urlElement.setAttribute( u"format"_s, url.format );
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
  const QDomNode wmsDimsNode = layer_node.namedItem( u"wmsDimensions"_s );
  if ( wmsDimsNode.isNull() )
  {
    return;
  }
  const QDomElement wmsDimsElem = wmsDimsNode.toElement();
  const QDomNodeList wmsDimsList = wmsDimsElem.elementsByTagName( u"dimension"_s );
  for ( int i = 0; i < wmsDimsList.size(); ++i )
  {
    const QDomElement dimElem = wmsDimsList.at( i ).toElement();
    const QString dimName = dimElem.attribute( u"name"_s );
    const QString dimFieldName = dimElem.attribute( u"fieldName"_s );
    // check field name
    const int dimFieldNameIndex = fields.indexOf( dimFieldName );
    if ( dimFieldNameIndex == -1 )
    {
      continue;
    }
    QVariant dimRefValue;
    const int dimDefaultDisplayType = dimElem.attribute( u"defaultDisplayType"_s ).toInt();
    if ( dimDefaultDisplayType == QgsServerWmsDimensionProperties::WmsDimensionInfo::AllValues )
    {
      const QString dimRefValueStr = dimElem.attribute( u"referenceValue"_s );
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
        dimElem.attribute( u"endFieldName"_s ),
        dimElem.attribute( u"units"_s ),
        dimElem.attribute( u"unitSymbol"_s ),
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
    QDomElement wmsDimsElem = document.createElement( u"wmsDimensions"_s );
    for ( const QgsServerWmsDimensionProperties::WmsDimensionInfo &dim : mWmsDimensions )
    {
      QDomElement dimElem = document.createElement( u"dimension"_s );
      dimElem.setAttribute( u"name"_s, dim.name );
      dimElem.setAttribute( u"fieldName"_s, dim.fieldName );
      dimElem.setAttribute( u"endFieldName"_s, dim.endFieldName );
      dimElem.setAttribute( u"units"_s, dim.units );
      dimElem.setAttribute( u"unitSymbol"_s, dim.unitSymbol );
      dimElem.setAttribute( u"defaultDisplayType"_s, dim.defaultDisplayType );
      dimElem.setAttribute( u"referenceValue"_s, dim.referenceValue.toString() );
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

QString QgsMapLayerServerProperties::wfsTypeName() const
{
  QString name = mLayer ? mLayer->name() : QString();
  if ( !mShortName.isEmpty() )
    name = mShortName;

  name.replace( ' ', '_' ).replace( ':', '-' ).replace( QChar( 0x2014 ) /* em-dash */, '-' );

  return name.toLocal8Bit();
}

void QgsMapLayerServerProperties::readXml( const QDomNode &layerNode ) // cppcheck-suppress duplInheritedMember
{
  QgsServerMetadataUrlProperties::readXml( layerNode );
  if ( metadataUrls().isEmpty() )
  {
    // metadataUrl is still empty, maybe it's a QGIS Project < 3.22
    // keep for legacy
    const QDomElement metaUrlElem = layerNode.firstChildElement( u"metadataUrl"_s );
    if ( !metaUrlElem.isNull() )
    {
      const QString url = metaUrlElem.text();
      const QString type = metaUrlElem.attribute( u"type"_s, QString() );
      const QString format = metaUrlElem.attribute( u"format"_s, QString() );
      const QgsMapLayerServerProperties::MetadataUrl newItem( url, type, format );
      setMetadataUrls( QList<QgsMapLayerServerProperties::MetadataUrl>() << newItem );
    }
  }

  QgsServerWmsDimensionProperties::readXml( layerNode );

  //short name
  const QDomElement shortNameElem = layerNode.firstChildElement( u"shortname"_s );
  if ( !shortNameElem.isNull() )
  {
    mShortName = shortNameElem.text();
  }

  //title
  const QDomElement titleElem = layerNode.firstChildElement( u"title"_s );
  if ( !titleElem.isNull() )
  {
    mTitle = titleElem.text();
    mWfsTitle = titleElem.attribute( u"wfs"_s );
  }

  //abstract
  const QDomElement abstractElem = layerNode.firstChildElement( u"abstract"_s );
  if ( !abstractElem.isNull() )
  {
    mAbstract = abstractElem.text();
  }

  //keywordList
  const QDomElement keywordListElem = layerNode.firstChildElement( u"keywordList"_s );
  if ( !keywordListElem.isNull() )
  {
    QStringList kwdList;
    for ( QDomNode n = keywordListElem.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
      const QString keyword = n.toElement().text();
      if ( !keyword.isEmpty() )
        kwdList << keyword;
    }
    mKeywordList = kwdList.join( ", "_L1 );
  }

  //dataUrl
  const QDomElement dataUrlElem = layerNode.firstChildElement( u"dataUrl"_s );
  if ( !dataUrlElem.isNull() )
  {
    mDataUrl = dataUrlElem.text();
    mDataUrlFormat = dataUrlElem.attribute( u"format"_s, QString() );
  }

  //attribution
  const QDomElement attribElem = layerNode.firstChildElement( u"attribution"_s );
  if ( !attribElem.isNull() )
  {
    mAttribution = attribElem.text();
    mAttributionUrl = attribElem.attribute( u"href"_s, QString() );
  }

  //legendUrl
  const QDomElement legendUrlElem = layerNode.firstChildElement( u"legendUrl"_s );
  if ( !legendUrlElem.isNull() )
  {
    mLegendUrl = legendUrlElem.text();
    mLegendUrlFormat = legendUrlElem.attribute( u"format"_s, QString() );
  }
}

void QgsMapLayerServerProperties::writeXml( QDomNode &layerNode, QDomDocument &document ) const // cppcheck-suppress duplInheritedMember
{
  QgsServerMetadataUrlProperties::writeXml( layerNode, document );
  QgsServerWmsDimensionProperties::writeXml( layerNode, document );

  // layer short name
  if ( !mShortName.isEmpty() )
  {
    QDomElement layerShortName = document.createElement( u"shortname"_s );
    const QDomText layerShortNameText = document.createTextNode( mShortName );
    layerShortName.appendChild( layerShortNameText );
    layerNode.appendChild( layerShortName );
  }

  // layer title
  if ( !mTitle.isEmpty() )
  {
    QDomElement layerTitle = document.createElement( u"title"_s );
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
    QDomElement layerAbstract = document.createElement( u"abstract"_s );
    const QDomText layerAbstractText = document.createTextNode( mAbstract );
    layerAbstract.appendChild( layerAbstractText );
    layerNode.appendChild( layerAbstract );
  }

  // layer keyword list
  const QStringList keywordStringList = mKeywordList.split( ',', Qt::SkipEmptyParts );
  if ( !keywordStringList.isEmpty() )
  {
    QDomElement layerKeywordList = document.createElement( u"keywordList"_s );
    for ( int i = 0; i < keywordStringList.size(); ++i )
    {
      QDomElement layerKeywordValue = document.createElement( u"value"_s );
      const QDomText layerKeywordText = document.createTextNode( keywordStringList.at( i ).trimmed() );
      layerKeywordValue.appendChild( layerKeywordText );
      layerKeywordList.appendChild( layerKeywordValue );
    }
    layerNode.appendChild( layerKeywordList );
  }

  // layer dataUrl
  if ( !mDataUrl.isEmpty() )
  {
    QDomElement layerDataUrl = document.createElement( u"dataUrl"_s );
    const QDomText layerDataUrlText = document.createTextNode( mDataUrl );
    layerDataUrl.appendChild( layerDataUrlText );
    layerDataUrl.setAttribute( u"format"_s, mDataUrlFormat );
    layerNode.appendChild( layerDataUrl );
  }

  // layer legendUrl
  if ( !mLegendUrl.isEmpty() )
  {
    QDomElement layerLegendUrl = document.createElement( u"legendUrl"_s );
    const QDomText layerLegendUrlText = document.createTextNode( mLegendUrl );
    layerLegendUrl.appendChild( layerLegendUrlText );
    layerLegendUrl.setAttribute( u"format"_s, mLegendUrlFormat );
    layerNode.appendChild( layerLegendUrl );
  }

  // layer attribution
  if ( !mAttribution.isEmpty() )
  {
    QDomElement layerAttribution = document.createElement( u"attribution"_s );
    const QDomText layerAttributionText = document.createTextNode( mAttribution );
    layerAttribution.appendChild( layerAttributionText );
    layerAttribution.setAttribute( u"href"_s, mAttributionUrl );
    layerNode.appendChild( layerAttribution );
  }
}
