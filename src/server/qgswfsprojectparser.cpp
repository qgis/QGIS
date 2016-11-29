/***************************************************************************
                              qgswfsprojectparser.cpp
                              -----------------------
  begin                : March 25, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsprojectparser.h"
#include "qgsconfigcache.h"
#include "qgsconfigparserutils.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsmapserviceexception.h"
#include "qgsaccesscontrol.h"
#include "qgslogger.h"

QgsWfsProjectParser::QgsWfsProjectParser(
  const QString& filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* ac
#endif
)
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  :
  mAccessControl( ac )
#endif
{
  mProjectParser = QgsConfigCache::instance()->serverConfiguration( filePath );
}

QgsWfsProjectParser::~QgsWfsProjectParser()
{
  delete mProjectParser;
}

void QgsWfsProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  mProjectParser->serviceCapabilities( parentElement, doc, QStringLiteral( "WFS" ) );
}

QString QgsWfsProjectParser::serviceUrl() const
{
  return mProjectParser->serviceUrl();
}

QString QgsWfsProjectParser::wfsServiceUrl() const
{
  return mProjectParser->wfsServiceUrl();
}

void QgsWfsProjectParser::featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wfsLayersId = mProjectParser->wfsLayers();
  QSet<QString> wfstUpdateLayersId = wfstUpdateLayers();
  QSet<QString> wfstInsertLayersId = wfstInsertLayers();
  QSet<QString> wfstDeleteLayersId = wfstDeleteLayers();

  QMap<QString, QgsMapLayer*> layerMap;

  Q_FOREACH ( const QDomElement& elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "vector" ) )
    {
      QString layerId = mProjectParser->layerId( elem );
      if ( !wfsLayersId.contains( layerId ) )
      {
        continue;
      }
      QgsMapLayer* layer = mProjectParser->createLayerFromElement( elem );
      if ( !layer )
      {
        continue;
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( layer ) )
      {
        continue;
      }
#endif
      QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
      layerMap.insert( layer->id(), layer );

      QDomElement layerElem = doc.createElement( QStringLiteral( "FeatureType" ) );
      QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
      //We use the layer name even though it might not be unique.
      //Because the id sometimes contains user/pw information and the name is more descriptive
      QString typeName = layer->name();
      if ( !layer->shortName().isEmpty() )
        typeName = layer->shortName();
      typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      QDomText nameText = doc.createTextNode( typeName );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
      QString titleName = layer->title();
      if ( titleName.isEmpty() )
      {
        titleName = layer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
      QString abstractName = layer->abstract();
      if ( abstractName.isEmpty() )
      {
        abstractName = QLatin1String( "" );
      }
      QDomText abstractText = doc.createTextNode( abstractName );
      abstractElem.appendChild( abstractText );
      layerElem.appendChild( abstractElem );

      //keyword list
      if ( !layer->keywordList().isEmpty() )
      {
        QDomElement keywordsElem = doc.createElement( QStringLiteral( "Keywords" ) );
        QDomText keywordsText = doc.createTextNode( layer->keywordList() );
        keywordsElem.appendChild( keywordsText );
        layerElem.appendChild( keywordsElem );
      }

      //appendExGeographicBoundingBox( layerElem, doc, layer->extent(), layer->crs() );

      QDomElement srsElem = doc.createElement( QStringLiteral( "SRS" ) );
      QDomText srsText = doc.createTextNode( layer->crs().authid() );
      srsElem.appendChild( srsText );
      layerElem.appendChild( srsElem );

      //wfs:Operations element
      QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
      //wfs:Query element
      QDomElement queryElement = doc.createElement( QStringLiteral( "Query" )/*wfs:Query*/ );
      operationsElement.appendChild( queryElement );

      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
      QgsVectorDataProvider* provider = vlayer->dataProvider();
      if (( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
      {
        //wfs:Insert element
        QDomElement insertElement = doc.createElement( QStringLiteral( "Insert" )/*wfs:Insert*/ );
        operationsElement.appendChild( insertElement );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
          ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) &&
          wfstUpdateLayersId.contains( layer->id() ) )
      {
        //wfs:Update element
        QDomElement updateElement = doc.createElement( QStringLiteral( "Update" )/*wfs:Update*/ );
        operationsElement.appendChild( updateElement );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
      {
        //wfs:Delete element
        QDomElement deleteElement = doc.createElement( QStringLiteral( "Delete" )/*wfs:Delete*/ );
        operationsElement.appendChild( deleteElement );
      }

      layerElem.appendChild( operationsElement );

      QgsRectangle layerExtent = layer->extent();
      QDomElement bBoxElement = doc.createElement( QStringLiteral( "LatLongBoundingBox" ) );
      bBoxElement.setAttribute( QStringLiteral( "minx" ), QString::number( layerExtent.xMinimum() ) );
      bBoxElement.setAttribute( QStringLiteral( "miny" ), QString::number( layerExtent.yMinimum() ) );
      bBoxElement.setAttribute( QStringLiteral( "maxx" ), QString::number( layerExtent.xMaximum() ) );
      bBoxElement.setAttribute( QStringLiteral( "maxy" ), QString::number( layerExtent.yMaximum() ) );
      layerElem.appendChild( bBoxElement );

      // layer metadata URL
      QString metadataUrl = layer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
        QString metadataUrlType = layer->metadataUrlType();
        metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
        QString metadataUrlFormat = layer->metadataUrlFormat();
        if ( metadataUrlFormat == QLatin1String( "text/xml" ) )
        {
          metaUrlElem.setAttribute( QStringLiteral( "format" ), QStringLiteral( "XML" ) );
        }
        else
        {
          metaUrlElem.setAttribute( QStringLiteral( "format" ), QStringLiteral( "TXT" ) );
        }
        QDomText metaUrlText = doc.createTextNode( metadataUrl );
        metaUrlElem.appendChild( metaUrlText );
        layerElem.appendChild( metaUrlElem );
      }

      parentElement.appendChild( layerElem );
    }
  }
  return;
}

QSet<QString> QgsWfsProjectParser::wfstUpdateLayers() const
{
  QSet<QString> publishedIds = wfsLayerSet();
  QSet<QString> wfsList;
  if ( !mProjectParser->xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( QStringLiteral( "WFSTLayers" ) );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstUpdateLayersElem = wfstLayersElem.firstChildElement( QStringLiteral( "Update" ) );
  if ( wfstUpdateLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstUpdateLayersElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( publishedIds.contains( id ) )
    {
      wfsList.insert( id );
    }
  }
  return wfsList;
}

QSet<QString> QgsWfsProjectParser::wfstInsertLayers() const
{
  QSet<QString> publishedIds = wfsLayerSet();
  QSet<QString> wfsList;
  if ( !mProjectParser->xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( QStringLiteral( "WFSTLayers" ) );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstInsertLayersElem = wfstLayersElem.firstChildElement( QStringLiteral( "Insert" ) );
  if ( wfstInsertLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstInsertLayersElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( publishedIds.contains( id ) )
    {
      wfsList.insert( id );
    }
  }
  return wfsList;
}

QSet<QString> QgsWfsProjectParser::wfstDeleteLayers() const
{
  QSet<QString> publishedIds = wfsLayerSet();
  QSet<QString> wfsList;
  if ( !mProjectParser->xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( QStringLiteral( "WFSTLayers" ) );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstDeleteLayersElem = wfstLayersElem.firstChildElement( QStringLiteral( "Delete" ) );
  if ( wfstDeleteLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstDeleteLayersElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( publishedIds.contains( id ) )
    {
      wfsList.insert( id );
    }
  }
  return wfsList;
}

void QgsWfsProjectParser::describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wfsLayersId = mProjectParser->wfsLayers();
  QStringList typeNameList;
  if ( aTypeName != QLatin1String( "" ) )
  {
    QStringList typeNameSplit = aTypeName.split( QStringLiteral( "," ) );
    Q_FOREACH ( const QString& str, typeNameSplit )
    {
      if ( str.contains( QLatin1String( ":" ) ) )
        typeNameList << str.section( QStringLiteral( ":" ), 1, 1 );
      else
        typeNameList << str;
    }
  }

  Q_FOREACH ( const QDomElement& elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "vector" ) )
    {
      QgsMapLayer* mLayer = mProjectParser->createLayerFromElement( elem );
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mLayer );
      if ( !layer )
        continue;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( layer ) )
      {
        continue;
      }
#endif

      QString typeName = layer->name();
      if ( !layer->shortName().isEmpty() )
        typeName = layer->shortName();
      typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );

      if ( wfsLayersId.contains( layer->id() ) && ( aTypeName == QLatin1String( "" ) || typeNameList.contains( typeName ) ) )
      {
        //do a select with searchRect and go through all the features
        QgsVectorDataProvider* provider = layer->dataProvider();
        if ( !provider )
        {
          continue;
        }

        //hidden attributes for this layer
        const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWfs();

        //xsd:element
        QDomElement elementElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
        elementElem.setAttribute( QStringLiteral( "name" ), typeName );
        elementElem.setAttribute( QStringLiteral( "type" ), "qgs:" + typeName + "Type" );
        elementElem.setAttribute( QStringLiteral( "substitutionGroup" ), QStringLiteral( "gml:_Feature" ) );
        parentElement.appendChild( elementElem );

        //xsd:complexType
        QDomElement complexTypeElem = doc.createElement( QStringLiteral( "complexType" )/*xsd:complexType*/ );
        complexTypeElem.setAttribute( QStringLiteral( "name" ), typeName + "Type" );
        parentElement.appendChild( complexTypeElem );

        //xsd:complexType
        QDomElement complexContentElem = doc.createElement( QStringLiteral( "complexContent" )/*xsd:complexContent*/ );
        complexTypeElem.appendChild( complexContentElem );

        //xsd:extension
        QDomElement extensionElem = doc.createElement( QStringLiteral( "extension" )/*xsd:extension*/ );
        extensionElem.setAttribute( QStringLiteral( "base" ), QStringLiteral( "gml:AbstractFeatureType" ) );
        complexContentElem.appendChild( extensionElem );

        //xsd:sequence
        QDomElement sequenceElem = doc.createElement( QStringLiteral( "sequence" )/*xsd:sequence*/ );
        extensionElem.appendChild( sequenceElem );

        //xsd:element
        if ( layer->hasGeometryType() )
        {
          QDomElement geomElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
          geomElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "geometry" ) );
          if ( provider->name() == QLatin1String( "ogr" ) )
          {
            // because some ogr drivers (e.g. ESRI ShapeFile, GML)
            // are not able to determine the geometry type of a layer.
            // we set to GeometryType
            geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:GeometryPropertyType" ) );
          }
          else
          {
            QgsWkbTypes::Type wkbType = layer->wkbType();
            switch ( wkbType )
            {
              case QgsWkbTypes::Point25D:
              case QgsWkbTypes::Point:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:PointPropertyType" ) );
                break;
              case QgsWkbTypes::LineString25D:
              case QgsWkbTypes::LineString:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:LineStringPropertyType" ) );
                break;
              case QgsWkbTypes::Polygon25D:
              case QgsWkbTypes::Polygon:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:PolygonPropertyType" ) );
                break;
              case QgsWkbTypes::MultiPoint25D:
              case QgsWkbTypes::MultiPoint:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiPointPropertyType" ) );
                break;
              case QgsWkbTypes::MultiLineString25D:
              case QgsWkbTypes::MultiLineString:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiLineStringPropertyType" ) );
                break;
              case QgsWkbTypes::MultiPolygon25D:
              case QgsWkbTypes::MultiPolygon:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:MultiPolygonPropertyType" ) );
                break;
              default:
                geomElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "gml:GeometryPropertyType" ) );
                break;
            }
          }
          geomElem.setAttribute( QStringLiteral( "minOccurs" ), QStringLiteral( "0" ) );
          geomElem.setAttribute( QStringLiteral( "maxOccurs" ), QStringLiteral( "1" ) );
          sequenceElem.appendChild( geomElem );
        }

        //const QgsFields& fields = provider->fields();
        const QgsFields& fields = layer->pendingFields();
        for ( int idx = 0; idx < fields.count(); ++idx )
        {

          QString attributeName = fields.at( idx ).name();
          //skip attribute if excluded from WFS publication
          if ( layerExcludedAttributes.contains( attributeName ) )
          {
            continue;
          }

          //xsd:element
          QDomElement attElem = doc.createElement( QStringLiteral( "element" )/*xsd:element*/ );
          attElem.setAttribute( QStringLiteral( "name" ), attributeName );
          QVariant::Type attributeType = fields.at( idx ).type();
          if ( attributeType == QVariant::Int )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "integer" ) );
          else if ( attributeType == QVariant::LongLong )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "long" ) );
          else if ( attributeType == QVariant::Double )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "double" ) );
          else if ( attributeType == QVariant::Bool )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "boolean" ) );
          else if ( attributeType == QVariant::Date )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "date" ) );
          else if ( attributeType == QVariant::Time )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "time" ) );
          else if ( attributeType == QVariant::DateTime )
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "dateTime" ) );
          else
            attElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "string" ) );

          sequenceElem.appendChild( attElem );

          QString alias = fields.at( idx ).alias();
          if ( !alias.isEmpty() )
          {
            attElem.setAttribute( QStringLiteral( "alias" ), alias );
          }
        }
      }
    }
  }
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  return;
}

QStringList QgsWfsProjectParser::wfsLayers() const
{
  return mProjectParser->wfsLayers();
}

QSet<QString> QgsWfsProjectParser::wfsLayerSet() const
{
  return QSet<QString>::fromList( wfsLayers() );
}

int QgsWfsProjectParser::wfsLayerPrecision( const QString& aLayerId ) const
{
  QStringList wfsLayersId = mProjectParser->wfsLayers();
  if ( !wfsLayersId.contains( aLayerId ) )
  {
    return -1;
  }
  int prec = 8;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement wfsPrecElem = propertiesElem.firstChildElement( QStringLiteral( "WFSLayersPrecision" ) );
    if ( !wfsPrecElem.isNull() )
    {
      QDomElement wfsLayerPrecElem = wfsPrecElem.firstChildElement( aLayerId );
      if ( !wfsLayerPrecElem.isNull() )
      {
        QString precStr = wfsLayerPrecElem.text();
        prec = precStr.toInt();
      }
    }
  }
  return prec;
}

QList<QgsMapLayer*> QgsWfsProjectParser::mapLayerFromTypeName( const QString& aTypeName, bool useCache ) const
{
  Q_UNUSED( useCache );

  QList<QgsMapLayer*> layerList;
  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();

  if ( projectLayerElements.size() < 1 )
  {
    return layerList;
  }
  QStringList wfsLayersId = wfsLayers();

  QStringList typeNameList;
  if ( aTypeName != QLatin1String( "" ) )
  {
    QStringList typeNameSplit = aTypeName.split( QStringLiteral( "," ) );
    Q_FOREACH ( const QString& str, typeNameSplit )
    {
      if ( str.contains( QLatin1String( ":" ) ) )
        typeNameList << str.section( QStringLiteral( ":" ), 1, 1 );
      else
        typeNameList << str;
    }
  }

  Q_FOREACH ( const QDomElement& elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "vector" ) )
    {
      QgsMapLayer* mLayer = mProjectParser->createLayerFromElement( elem );
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mLayer );
      if ( !layer )
        continue;

      QString typeName = layer->name();
      if ( !layer->shortName().isEmpty() )
        typeName = layer->shortName();
      typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );

      if ( wfsLayersId.contains( layer->id() ) && ( aTypeName == QLatin1String( "" ) || typeNameList.contains( typeName ) ) )
        layerList.push_back( mLayer );
    }
  }
  return layerList;
}
