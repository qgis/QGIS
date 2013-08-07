/***************************************************************************
                              qgsprojectparser.cpp
                              --------------------
  begin                : May 27, 2010
  copyright            : (C) 2010 by Marco Hugentobler
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

#include "qgsmaplayerregistry.h"

#include "qgsprojectparser.h"
#include "qgsconfigcache.h"
#include "qgscrscache.h"
#include "qgsdatasourceuri.h"
#include "qgsmslayercache.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include "qgscomposition.h"
#include "qgscomposerarrow.h"
#include "qgscomposerattributetable.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposerhtml.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"

#include "QFileInfo"
#include <QSvgRenderer>
#include <QTextDocument>
#include "QTextStream"
#include <QUrl>


QgsProjectParser::QgsProjectParser( QDomDocument* xmlDoc, const QString& filePath ): QgsConfigParser(), mXMLDoc( xmlDoc ), mProjectPath( filePath )
{
  mOutputUnits = QgsMapRenderer::Millimeters;
  setLegendParametersFromProject();
  setSelectionColor();
  setMaxWidthHeight();

  //accelerate the search for layers, groups and the creation of annotation items
  if ( mXMLDoc )
  {
    QDomNodeList layerNodeList = mXMLDoc->elementsByTagName( "maplayer" );
    QDomElement currentElement;
    int nNodes = layerNodeList.size();
    for ( int i = 0; i < nNodes; ++i )
    {
      currentElement = layerNodeList.at( i ).toElement();
      mProjectLayerElements.push_back( currentElement );
      mProjectLayerElementsByName.insert( layerName( currentElement ), currentElement );
      mProjectLayerElementsById.insert( layerId( currentElement ), currentElement );
    }

    QDomElement legendElement = mXMLDoc->documentElement().firstChildElement( "legend" );
    if ( !legendElement.isNull() )
    {
      QDomNodeList groupNodeList = legendElement.elementsByTagName( "legendgroup" );
      for ( int i = 0; i < groupNodeList.size(); ++i )
      {
        mLegendGroupElements.push_back( groupNodeList.at( i ).toElement() );
      }
    }

    mRestrictedLayers = restrictedLayers();
    createTextAnnotationItems();
    createSvgAnnotationItems();
  }
}

QgsProjectParser::QgsProjectParser(): mXMLDoc( 0 )
{
}

QgsProjectParser::~QgsProjectParser()
{
  delete mXMLDoc;
  cleanupTextAnnotationItems();
  cleanupSvgAnnotationItems();
}

int QgsProjectParser::numberOfLayers() const
{
  return mProjectLayerElements.size();
}

void QgsProjectParser::layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings ) const
{
  QStringList nonIdentifiableLayers = identifyDisabledLayers();
  if ( mProjectLayerElements.size() < 1 )
  {
    return;
  }

  if ( fullProjectSettings )
  {
    addDrawingOrder( parentElement, doc );
  }

  QMap<QString, QgsMapLayer *> layerMap;
  projectLayerMap( layerMap );

  //According to the WMS spec, there can be only one toplevel layer.
  //So we create an artificial one here to be in accordance with the schema
  QString projTitle = projectTitle();
  QDomElement layerParentElem = doc.createElement( "Layer" );
  layerParentElem.setAttribute( "queryable", "1" );
  QDomElement layerParentNameElem = doc.createElement( "Name" );
  QDomText layerParentNameText = doc.createTextNode( projTitle );
  layerParentNameElem.appendChild( layerParentNameText );
  layerParentElem.appendChild( layerParentNameElem );
  QDomElement layerParentTitleElem = doc.createElement( "Title" );
  QDomText layerParentTitleText = doc.createTextNode( projTitle );
  layerParentTitleElem.appendChild( layerParentTitleText );
  layerParentElem.appendChild( layerParentTitleElem );

  QDomElement legendElem = mXMLDoc->documentElement().firstChildElement( "legend" );

  addLayers( doc, layerParentElem, legendElem, layerMap, nonIdentifiableLayers, version, fullProjectSettings );

  parentElement.appendChild( layerParentElem );
  combineExtentAndCrsOfGroupChildren( layerParentElem, doc, true );
}

void QgsProjectParser::featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const
{
  QStringList wfsLayersId = wfsLayers();
  QStringList wfstUpdateLayersId = wfstUpdateLayers();
  QStringList wfstInsertLayersId = wfstInsertLayers();
  QStringList wfstDeleteLayersId = wfstDeleteLayers();

  if ( mProjectLayerElements.size() < 1 )
  {
    return;
  }

  QMap<QString, QgsMapLayer *> layerMap;

  foreach ( const QDomElement &elem, mProjectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      //QgsMapLayer *layer = createLayerFromElement( *layerIt );
      QgsMapLayer *layer = createLayerFromElement( elem );
      if ( layer && wfsLayersId.contains( layer->id() ) )
      {
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
        layerMap.insert( layer->id(), layer );

        QDomElement layerElem = doc.createElement( "FeatureType" );
        QDomElement nameElem = doc.createElement( "Name" );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = layer->name();
        typeName = typeName.replace( QString( " " ), QString( "_" ) );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement titleElem = doc.createElement( "Title" );
        QString titleName = layer->title();
        if ( titleName.isEmpty() )
        {
          titleName = layer->name();
        }
        QDomText titleText = doc.createTextNode( titleName );
        titleElem.appendChild( titleText );
        layerElem.appendChild( titleElem );

        QDomElement abstractElem = doc.createElement( "Abstract" );
        QString abstractName = layer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = "";
        }
        QDomText abstractText = doc.createTextNode( abstractName );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );

        //keyword list
        if ( !layer->keywordList().isEmpty() )
        {
          QDomElement keywordsElem = doc.createElement( "Keywords" );
          QDomText keywordsText = doc.createTextNode( layer->keywordList() );
          keywordsElem.appendChild( keywordsText );
          layerElem.appendChild( keywordsElem );
        }

        //appendExGeographicBoundingBox( layerElem, doc, layer->extent(), layer->crs() );

        QDomElement srsElem = doc.createElement( "SRS" );
        QDomText srsText = doc.createTextNode( layer->crs().authid() );
        srsElem.appendChild( srsText );
        layerElem.appendChild( srsElem );

        //wfs:Operations element
        QDomElement operationsElement = doc.createElement( "Operations"/*wfs:Operations*/ );
        //wfs:Query element
        QDomElement queryElement = doc.createElement( "Query"/*wfs:Query*/ );
        operationsElement.appendChild( queryElement );

        QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
        QgsVectorDataProvider* provider = vlayer->dataProvider();
        if (( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
        {
          //wfs:Insert element
          QDomElement insertElement = doc.createElement( "Insert"/*wfs:Insert*/ );
          operationsElement.appendChild( insertElement );
        }
        if (( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
            ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) &&
            wfstUpdateLayersId.contains( layer->id() ) )
        {
          //wfs:Update element
          QDomElement updateElement = doc.createElement( "Update"/*wfs:Update*/ );
          operationsElement.appendChild( updateElement );
        }
        if (( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
        {
          //wfs:Delete element
          QDomElement deleteElement = doc.createElement( "Delete"/*wfs:Delete*/ );
          operationsElement.appendChild( deleteElement );
        }

        layerElem.appendChild( operationsElement );

        QgsRectangle layerExtent = layer->extent();
        QDomElement bBoxElement = doc.createElement( "LatLongBoundingBox" );
        bBoxElement.setAttribute( "minx", QString::number( layerExtent.xMinimum() ) );
        bBoxElement.setAttribute( "miny", QString::number( layerExtent.yMinimum() ) );
        bBoxElement.setAttribute( "maxx", QString::number( layerExtent.xMaximum() ) );
        bBoxElement.setAttribute( "maxy", QString::number( layerExtent.yMaximum() ) );
        layerElem.appendChild( bBoxElement );

        // layer metadata URL
        QString metadataUrl = layer->metadataUrl();
        if ( !metadataUrl.isEmpty() )
        {
          QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
          QString metadataUrlType = layer->metadataUrlType();
          metaUrlElem.setAttribute( "type", metadataUrlType );
          QString metadataUrlFormat = layer->metadataUrlFormat();
          if ( metadataUrlFormat == "text/xml" )
          {
            metaUrlElem.setAttribute( "format", "XML" );
          }
          else
          {
            metaUrlElem.setAttribute( "format", "TXT" );
          }
          QDomText metaUrlText = doc.createTextNode( metadataUrl );
          metaUrlElem.appendChild( metaUrlText );
          layerElem.appendChild( metaUrlElem );
        }

        parentElement.appendChild( layerElem );
      }
    }
  }
  return;
}

void QgsProjectParser::owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const
{
  // set parentElement id
  QFileInfo projectFileInfo( mProjectPath );
  parentElement.setAttribute( "id", "ows-context-" + projectFileInfo.baseName() );

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  // OWSContext General element
  QDomElement generalElem = doc.createElement( "General" );

  QDomElement windowElem = doc.createElement( "Window" );
  windowElem.setAttribute( "height", "600" );
  windowElem.setAttribute( "width", "800" );
  generalElem.appendChild( windowElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "ows:Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    generalElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "ows:Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    generalElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    bool siaFormat = featureInfoFormatSIA2045();

    QDomElement keywordsElem = doc.createElement( "ows:Keywords" );
    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      QDomElement keywordElem = doc.createElement( "ows:Keyword" );
      QDomText keywordText = doc.createTextNode( keywordList.at( i ).toElement().text() );
      keywordElem.appendChild( keywordText );
      if ( siaFormat )
      {
        keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
      }
      keywordsElem.appendChild( keywordElem );
    }

    if ( keywordList.size() > 0 )
    {
      generalElem.appendChild( keywordsElem );
    }
  }

  parentElement.appendChild( generalElem );

  // OWSContext ResourceList element
  QStringList nonIdentifiableLayers = identifyDisabledLayers();
  if ( mProjectLayerElements.size() < 1 )
  {
    return;
  }

  QgsRectangle combinedBBox;
  QMap<QString, QgsMapLayer *> layerMap;
  projectLayerMap( layerMap );

  QDomElement legendElem = mXMLDoc->documentElement().firstChildElement( "legend" );

  QDomElement resourceListElem = doc.createElement( "ResourceList" );

  addOWSLayers( doc, resourceListElem, legendElem, layerMap, nonIdentifiableLayers, strHref, combinedBBox, "" );

  parentElement.appendChild( resourceListElem );

  QgsRectangle mapRect = mapRectangle();
  if ( !mapRect.isEmpty() )
  {
    combinedBBox = mapRect;
  }
  const QgsCoordinateReferenceSystem& projectCrs = projectCRS();
  QDomElement bboxElem = doc.createElement( "ows:BoundingBox" );
  bboxElem.setAttribute( "crs", projectCrs.authid() );
  if ( projectCrs.axisInverted() )
  {
    combinedBBox.invert();
  }
  QDomElement lowerCornerElem = doc.createElement( "ows:LowerCorner" );
  QDomText lowerCornerText = doc.createTextNode( QString::number( combinedBBox.xMinimum() ) + " " +  QString::number( combinedBBox.yMinimum() ) );
  lowerCornerElem.appendChild( lowerCornerText );
  bboxElem.appendChild( lowerCornerElem );
  QDomElement upperCornerElem = doc.createElement( "ows:UpperCorner" );
  QDomText upperCornerText = doc.createTextNode( QString::number( combinedBBox.xMaximum() ) + " " +  QString::number( combinedBBox.yMaximum() ) );
  upperCornerElem.appendChild( upperCornerText );
  bboxElem.appendChild( upperCornerElem );
  generalElem.appendChild( bboxElem );

}

void QgsProjectParser::describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( mProjectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wfsLayersId = wfsLayers();
  QStringList typeNameList;
  if ( aTypeName != "" )
  {
    QStringList typeNameSplit = aTypeName.split( "," );
    foreach ( const QString &str, typeNameSplit )
    {
      if ( str.contains( ":" ) )
        typeNameList << str.section( ":", 1, 1 );
      else
        typeNameList << str;
    }
  }

  foreach ( const QDomElement &elem, mProjectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      QgsMapLayer *mLayer = createLayerFromElement( elem );
      QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );

      QString typeName = layer->name();
      typeName = typeName.replace( QString( " " ), QString( "_" ) );

      if ( layer && wfsLayersId.contains( layer->id() ) && ( aTypeName == "" || typeNameList.contains( typeName ) ) )
      {
        //do a select with searchRect and go through all the features
        QgsVectorDataProvider* provider = layer->dataProvider();
        if ( !provider )
        {
          continue;
        }
        if ( layer->vectorJoins().size() > 0 )
        {
          QList<QgsMapLayer *> joinLayers;
          //JoinBuffer is based on qgsmaplayerregistry!!!!!
          //insert existing join info
          const QList< QgsVectorJoinInfo >& joins = layer->vectorJoins();
          for ( int i = 0; i < joins.size(); ++i )
          {
            QgsMapLayer* joinLayer = mapLayerFromLayerId( joins[i].joinLayerId );
            if ( joinLayer )
            {
              joinLayers << joinLayer;
            }
            QgsMapLayerRegistry::instance()->addMapLayers( joinLayers, false, true );
          }
          layer->updateFields();
        }

        //hidden attributes for this layer
        const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWFS();

        //xsd:element
        QDomElement elementElem = doc.createElement( "element"/*xsd:element*/ );
        elementElem.setAttribute( "name", typeName );
        elementElem.setAttribute( "type", "qgs:" + typeName + "Type" );
        elementElem.setAttribute( "substitutionGroup", "gml:_Feature" );
        parentElement.appendChild( elementElem );

        //xsd:complexType
        QDomElement complexTypeElem = doc.createElement( "complexType"/*xsd:complexType*/ );
        complexTypeElem.setAttribute( "name", typeName + "Type" );
        parentElement.appendChild( complexTypeElem );

        //xsd:complexType
        QDomElement complexContentElem = doc.createElement( "complexContent"/*xsd:complexContent*/ );
        complexTypeElem.appendChild( complexContentElem );

        //xsd:extension
        QDomElement extensionElem = doc.createElement( "extension"/*xsd:extension*/ );
        extensionElem.setAttribute( "base", "gml:AbstractFeatureType" );
        complexContentElem.appendChild( extensionElem );

        //xsd:sequence
        QDomElement sequenceElem = doc.createElement( "sequence"/*xsd:sequence*/ );
        extensionElem.appendChild( sequenceElem );

        //xsd:element
        QDomElement geomElem = doc.createElement( "element"/*xsd:element*/ );
        geomElem.setAttribute( "name", "geometry" );
        QGis::WkbType wkbType = layer->wkbType();
        if ( wkbType != QGis::WKBNoGeometry )
        {
          switch ( wkbType )
          {
            case QGis::WKBPoint25D:
            case QGis::WKBPoint:
              geomElem.setAttribute( "type", "gml:PointPropertyType" );
              break;
            case QGis::WKBLineString25D:
            case QGis::WKBLineString:
              geomElem.setAttribute( "type", "gml:LineStringPropertyType" );
              break;
            case QGis::WKBPolygon25D:
            case QGis::WKBPolygon:
              geomElem.setAttribute( "type", "gml:PolygonPropertyType" );
              break;
            case QGis::WKBMultiPoint25D:
            case QGis::WKBMultiPoint:
              geomElem.setAttribute( "type", "gml:MultiPointPropertyType" );
              break;
            case QGis::WKBMultiLineString25D:
            case QGis::WKBMultiLineString:
              geomElem.setAttribute( "type", "gml:MultiLineStringPropertyType" );
              break;
            case QGis::WKBMultiPolygon25D:
            case QGis::WKBMultiPolygon:
              geomElem.setAttribute( "type", "gml:MultiPolygonPropertyType" );
              break;
            default:
              geomElem.setAttribute( "type", "gml:GeometryPropertyType" );
              break;
          }
          geomElem.setAttribute( "minOccurs", "0" );
          geomElem.setAttribute( "maxOccurs", "1" );
          sequenceElem.appendChild( geomElem );
        }

        //const QgsFields& fields = provider->fields();
        const QgsFields& fields = layer->pendingFields();
        for ( int idx = 0; idx < fields.count(); ++idx )
        {

          QString attributeName = fields[idx].name();
          //skip attribute if excluded from WFS publication
          if ( layerExcludedAttributes.contains( attributeName ) )
          {
            continue;
          }

          //xsd:element
          QDomElement geomElem = doc.createElement( "element"/*xsd:element*/ );
          geomElem.setAttribute( "name", attributeName );
          QVariant::Type attributeType = fields[idx].type();
          if ( attributeType == QVariant::Int )
            geomElem.setAttribute( "type", "integer" );
          else if ( attributeType == QVariant::Double )
            geomElem.setAttribute( "type", "double" );
          else
            geomElem.setAttribute( "type", "string" );

          sequenceElem.appendChild( geomElem );

          QString alias = layer->attributeAlias( idx );
          if ( !alias.isEmpty() )
          {
            geomElem.setAttribute( "alias", alias );
          }
        }
      }
    }
  }
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  return;
}

QList<QgsMapLayer*> QgsProjectParser::mapLayerFromTypeName( const QString& tName, bool useCache ) const
{
  QList<QgsMapLayer*> layerList;

  if ( mProjectLayerElements.size() < 1 )
  {
    return layerList;
  }

  QStringList wfsLayersId = wfsLayers();

  foreach ( const QDomElement &elem, mProjectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      QgsMapLayer *mLayer = createLayerFromElement( elem, useCache );
      QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );

      QString typeName = layer->name();
      typeName = typeName.replace( QString( " " ), QString( "_" ) );
      if ( tName == typeName )
      {
        layerList.push_back( mLayer );
        return layerList;
      }
    }
  }
  return layerList;
}

void QgsProjectParser::addLayers( QDomDocument &doc,
                                  QDomElement &parentElem,
                                  const QDomElement &legendElem,
                                  const QMap<QString, QgsMapLayer *> &layerMap,
                                  const QStringList &nonIdentifiableLayers,
                                  QString version,
                                  bool fullProjectSettings ) const
{
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    QDomElement layerElem = doc.createElement( "Layer" );
    if ( fullProjectSettings )
    {
      layerElem.setAttribute( "visible", !( currentChildElem.attribute( "checked" ) == "Qt::Unchecked" ) );
    }


    if ( currentChildElem.tagName() == "legendgroup" )
    {
      layerElem.setAttribute( "queryable", "1" );
      QString name = currentChildElem.attribute( "name" );
      if ( mRestrictedLayers.contains( name ) ) //unpublished group
      {
        continue;
      }
      QDomElement nameElem = doc.createElement( "Name" );
      QDomText nameText = doc.createTextNode( name );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QDomText titleText = doc.createTextNode( name );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      if ( currentChildElem.attribute( "embedded" ) == "1" )
      {
        //add layers from other project files and embed into this group
        QString project = convertToAbsolutePath( currentChildElem.attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( "name" );
        QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
        QList<QDomElement> embeddedGroupElements = p->mLegendGroupElements;
        if ( p )
        {
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          foreach ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( "name" ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          QList<QDomElement> embeddedProjectLayerElements = p->mProjectLayerElements;
          foreach ( const QDomElement &elem, embeddedProjectLayerElements )
          {
            pLayerMap.insert( layerId( elem ), p->createLayerFromElement( elem ) );
          }

          p->addLayers( doc, layerElem, embeddedGroupElem, pLayerMap, pIdDisabled, version, fullProjectSettings );
        }
      }
      else //normal (not embedded) legend group
      {
        addLayers( doc, layerElem, currentChildElem, layerMap, nonIdentifiableLayers, version, fullProjectSettings );
      }

      // combine bounding boxes of children (groups/layers)
      combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      QString id = layerIdFromLegendLayer( currentChildElem );

      if ( !layerMap.contains( id ) )
      {
        QgsDebugMsg( QString( "layer %1 not found in map - layer cache to small?" ).arg( id ) );
        continue;
      }

      QgsMapLayer *currentLayer = layerMap[ id ];
      if ( !currentLayer )
      {
        QgsDebugMsg( QString( "layer %1 not found" ).arg( id ) );
        continue;
      }

      if ( mRestrictedLayers.contains( currentLayer->name() ) ) //unpublished layer
      {
        continue;
      }
      //vector layer without geometry
      if ( currentLayer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
        QGis::WkbType wkbType = vectorLayer->wkbType();
        if ( wkbType == QGis::WKBNoGeometry )
        {
          continue;
        }
      }
      // queryable layer
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( "queryable", "0" );
      }
      else
      {
        layerElem.setAttribute( "queryable", "1" );
      }

      QDomElement nameElem = doc.createElement( "Name" );
      //We use the layer name even though it might not be unique.
      //Because the id sometimes contains user/pw information and the name is more descriptive
      QDomText nameText = doc.createTextNode( currentLayer->name() );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QString titleName = currentLayer->title();
      if ( titleName.isEmpty() )
      {
        titleName = currentLayer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QString abstract = currentLayer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( "Abstract" );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( "," );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordListElem = doc.createElement( "KeywordList" );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( "Keyword" );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
          }
          keywordListElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordListElem );
      }

      //CRS
      QStringList crsList = createCRSListForLayer( currentLayer );
      appendCRSElementsToLayer( layerElem, doc, crsList );

      //Ex_GeographicBoundingBox
      appendLayerBoundingBoxes( layerElem, doc, currentLayer->extent(), currentLayer->crs() );

      //only one default style in project file mode
      QDomElement styleElem = doc.createElement( "Style" );
      QDomElement styleNameElem = doc.createElement( "Name" );
      QDomText styleNameText = doc.createTextNode( "default" );
      styleNameElem.appendChild( styleNameText );
      QDomElement styleTitleElem = doc.createElement( "Title" );
      QDomText styleTitleText = doc.createTextNode( "default" );
      styleTitleElem.appendChild( styleTitleText );
      styleElem.appendChild( styleNameElem );
      styleElem.appendChild( styleTitleElem );
      layerElem.appendChild( styleElem );

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        QString minScaleString = QString::number( currentLayer->minimumScale() );
        QString maxScaleString = QString::number( currentLayer->maximumScale() );

        if ( version == "1.1.1" )
        {
          QDomElement scaleHintElem = doc.createElement( "ScaleHint" );
          scaleHintElem.setAttribute( "min", minScaleString );
          scaleHintElem.setAttribute( "max", maxScaleString );
          layerElem.appendChild( scaleHintElem );
        }
        else
        {
          QDomElement minScaleElem = doc.createElement( "MinScaleDenominator" );
          QDomText minScaleText = doc.createTextNode( minScaleString );
          minScaleElem.appendChild( minScaleText );
          layerElem.appendChild( minScaleElem );
          QDomElement maxScaleElem = doc.createElement( "MaxScaleDenominator" );
          QDomText maxScaleText = doc.createTextNode( maxScaleString );
          maxScaleElem.appendChild( maxScaleText );
          layerElem.appendChild( maxScaleElem );
        }
      }

      // layer attribution
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( "DataURL" );
        QDomElement dataUrlFormatElem = doc.createElement( "Format" );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        QDomText dataUrlFormatText = doc.createTextNode( dataUrlFormat );
        dataUrlFormatElem.appendChild( dataUrlFormatText );
        dataUrlElem.appendChild( dataUrlFormatElem );
        QDomElement dataORElem = doc.createElement( "OnlineResource" );
        dataORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        dataORElem.setAttribute( "xlink:type", "simple" );
        dataORElem.setAttribute( "xlink:href", dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer attribution
      QString attribution = currentLayer->attribution();
      if ( !attribution.isEmpty() )
      {
        QDomElement attribElem = doc.createElement( "Attribution" );
        QDomElement attribTitleElem = doc.createElement( "Title" );
        QDomText attribText = doc.createTextNode( attribution );
        attribTitleElem.appendChild( attribText );
        attribElem.appendChild( attribTitleElem );
        QString attributionUrl = currentLayer->attributionUrl();
        if ( !attributionUrl.isEmpty() )
        {
          QDomElement attribORElem = doc.createElement( "OnlineResource" );
          attribORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
          attribORElem.setAttribute( "xlink:type", "simple" );
          attribORElem.setAttribute( "xlink:href", attributionUrl );
          attribElem.appendChild( attribORElem );
        }
        layerElem.appendChild( attribElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
        QString metadataUrlType = currentLayer->metadataUrlType();
        if ( version == "1.1.1" )
        {
          metaUrlElem.setAttribute( "type", metadataUrlType );
        }
        else if ( metadataUrlType == "FGDC" )
        {
          metaUrlElem.setAttribute( "type", "FGDC:1998" );
        }
        else if ( metadataUrlType == "TC211" )
        {
          metaUrlElem.setAttribute( "type", "ISO19115:2003" );
        }
        else
        {
          metaUrlElem.setAttribute( "type", metadataUrlType );
        }
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        if ( !metadataUrlFormat.isEmpty() )
        {
          QDomElement metaUrlFormatElem = doc.createElement( "Format" );
          QDomText metaUrlFormatText = doc.createTextNode( metadataUrlFormat );
          metaUrlFormatElem.appendChild( metaUrlFormatText );
          metaUrlElem.appendChild( metaUrlFormatElem );
        }
        QDomElement metaUrlORElem = doc.createElement( "OnlineResource" );
        metaUrlORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        metaUrlORElem.setAttribute( "xlink:type", "simple" );
        metaUrlORElem.setAttribute( "xlink:href", metadataUrl );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }

      // if the layer is published in WFS
      // add a FeatureListURL element
      // with an URL to the GML
      QStringList wfsLayersId = wfsLayers();
      if ( wfsLayersId.contains( currentLayer->id() ) )
      {
        QDomElement featListUrlElem = doc.createElement( "FeatureListURL" );

        QDomElement featListUrlFormatElem = doc.createElement( "Format" );
        QDomText featListUrlFormatText = doc.createTextNode( "text/xml" );
        featListUrlFormatElem.appendChild( featListUrlFormatText );
        featListUrlElem.appendChild( featListUrlFormatElem );

        QString hrefString = wfsServiceUrl();
        if ( hrefString.isEmpty() )
        {
          hrefString = serviceUrl();
        }
        if ( hrefString.isEmpty() )
        {
          QDomNodeList getCapNodeList = doc.elementsByTagName( "GetCapabilities" );
          if ( getCapNodeList.count() > 0 )
          {
            QDomElement getCapElem = getCapNodeList.at( 0 ).toElement();
            QDomNodeList getCapORNodeList = getCapElem.elementsByTagName( "OnlineResource" );
            if ( getCapORNodeList.count() > 0 )
            {
              hrefString = getCapORNodeList.at( 0 ).toElement().attribute( "xlink:href", "" );
            }
          }
        }
        if ( !hrefString.isEmpty() )
        {
          QUrl mapUrl( hrefString );
          mapUrl.addQueryItem( "SERVICE", "WFS" );
          mapUrl.addQueryItem( "VERSION", "1.0.0" );
          mapUrl.addQueryItem( "REQUEST", "GetFeature" );
          mapUrl.addQueryItem( "TYPENAME", currentLayer->name() );
          mapUrl.addQueryItem( "OUTPUTFORMAT", "GML2" );
          hrefString = mapUrl.toString();
          QDomElement featListUrlORElem = doc.createElement( "OnlineResource" );
          featListUrlORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
          featListUrlORElem.setAttribute( "xlink:type", "simple" );
          featListUrlORElem.setAttribute( "xlink:href", hrefString );
          featListUrlElem.appendChild( featListUrlORElem );
        }

        layerElem.appendChild( featListUrlElem );
      }

      if ( fullProjectSettings )
      {
        addLayerProjectSettings( layerElem, doc, currentLayer );
      }
    }
    else
    {
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

    parentElem.appendChild( layerElem );
  }
}

void QgsProjectParser::addOWSLayers( QDomDocument &doc,
                                     QDomElement &parentElem,
                                     const QDomElement &legendElem,
                                     const QMap<QString, QgsMapLayer *> &layerMap,
                                     const QStringList &nonIdentifiableLayers,
                                     const QString& strHref,
                                     QgsRectangle& combinedBBox,
                                     QString strGroup ) const
{
  const QgsCoordinateReferenceSystem& projectCrs = projectCRS();
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();

    if ( currentChildElem.tagName() == "legendgroup" )
    {
      QString name = currentChildElem.attribute( "name" );
      if ( mRestrictedLayers.contains( name ) ) //unpublished group
      {
        continue;
      }
      QString group;
      if ( strGroup.isEmpty() )
      {
        group = name;
      }
      else
      {
        group = strGroup + "/" + name;
      }

      if ( currentChildElem.attribute( "embedded" ) == "1" )
      {
        //add layers from other project files and embed into this group
        QString project = convertToAbsolutePath( currentChildElem.attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( "name" );
        QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
        QList<QDomElement> embeddedGroupElements = p->mLegendGroupElements;
        if ( p )
        {
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          foreach ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( "name" ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          QList<QDomElement> embeddedProjectLayerElements = p->mProjectLayerElements;
          foreach ( const QDomElement &elem, embeddedProjectLayerElements )
          {
            pLayerMap.insert( layerId( elem ), p->createLayerFromElement( elem ) );
          }

          p->addOWSLayers( doc, parentElem, embeddedGroupElem, pLayerMap, pIdDisabled, strHref, combinedBBox, group );
        }
      }
      else //normal (not embedded) legend group
      {
        addOWSLayers( doc, parentElem, currentChildElem, layerMap, nonIdentifiableLayers, strHref, combinedBBox, group );
      }

      // combine bounding boxes of children (groups/layers)
      // combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      QDomElement layerElem = doc.createElement( "Layer" );
      QString id = layerIdFromLegendLayer( currentChildElem );

      if ( !layerMap.contains( id ) )
      {
        QgsDebugMsg( QString( "layer %1 not found in map - layer cache to small?" ).arg( id ) );
        continue;
      }

      QgsMapLayer *currentLayer = layerMap[ id ];
      if ( !currentLayer )
      {
        QgsDebugMsg( QString( "layer %1 not found" ).arg( id ) );
        continue;
      }

      if ( mRestrictedLayers.contains( currentLayer->name() ) ) //unpublished layer
      {
        continue;
      }
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( "queryable", "false" );
      }
      else
      {
        layerElem.setAttribute( "queryable", "true" );
      }

      // is the layer visible ?
      if ( currentChildElem.firstChildElement().firstChildElement().attribute( "visible" ) == "1" )
      {
        layerElem.setAttribute( "hidden", "false" );
      }
      else
      {
        layerElem.setAttribute( "hidden", "true" );
      }

      if ( !strGroup.isEmpty() )
      {
        layerElem.setAttribute( "group", strGroup );
      }
      // Because Layer transparency is used for the rendering
      // OWSContext Layer opacity is set to 1
      layerElem.setAttribute( "opacity", 1 );

      QString lyrname = currentLayer->name();
      layerElem.setAttribute( "name", lyrname );

      // define an id based on layer name
      layerElem.setAttribute( "id", lyrname.replace( QRegExp( "[\\W]" ), "_" ) );

      QDomElement titleElem = doc.createElement( "ows:Title" );
      QString titleName = currentLayer->title();
      if ( titleName.isEmpty() )
      {
        titleName = currentLayer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QDomElement formatElem = doc.createElement( "ows:OutputFormat" );
      QDomText formatText = doc.createTextNode( "image/png" );
      formatElem.appendChild( formatText );
      layerElem.appendChild( formatElem );

      QDomElement serverElem = doc.createElement( "Server" );
      serverElem.setAttribute( "service", "urn:ogc:serviceType:WMS" );
      serverElem.setAttribute( "version", "1.3.0" );
      serverElem.setAttribute( "default", "true" );
      QDomElement orServerElem = doc.createElement( "OnlineResource" );
      orServerElem.setAttribute( "xlink:href", strHref );
      serverElem.appendChild( orServerElem );
      layerElem.appendChild( serverElem );

      QString abstract = currentLayer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( "ows:Abstract" );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        QString minScaleString = QString::number( currentLayer->minimumScale() );
        QString maxScaleString = QString::number( currentLayer->maximumScale() );
        QDomElement minScaleElem = doc.createElement( "sld:MinScaleDenominator" );
        QDomText minScaleText = doc.createTextNode( minScaleString );
        minScaleElem.appendChild( minScaleText );
        layerElem.appendChild( minScaleElem );
        QDomElement maxScaleElem = doc.createElement( "sld:MaxScaleDenominator" );
        QDomText maxScaleText = doc.createTextNode( maxScaleString );
        maxScaleElem.appendChild( maxScaleText );
        layerElem.appendChild( maxScaleElem );
      }

      /*
      //CRS
      QStringList crsList = createCRSListForLayer( currentLayer );
      appendCRSElementsToLayer( layerElem, doc, crsList );

      //Ex_GeographicBoundingBox
      appendLayerBoundingBoxes( layerElem, doc, currentLayer->extent(), currentLayer->crs() );
      */
      //get project crs
      const QgsCoordinateReferenceSystem& layerCrs = currentLayer->crs();
      QgsCoordinateTransform t( layerCrs, projectCrs );

      //transform
      QgsRectangle BBox = t.transformBoundingBox( currentLayer->extent() );
      if ( combinedBBox.isEmpty() )
      {
        combinedBBox = BBox;
      }
      else
      {
        combinedBBox.combineExtentWith( &BBox );
      }

      QDomElement styleListElem = doc.createElement( "StyleList" );
      //only one default style in project file mode
      QDomElement styleElem = doc.createElement( "Style" );
      styleElem.setAttribute( "current", "true" );
      QDomElement styleNameElem = doc.createElement( "Name" );
      QDomText styleNameText = doc.createTextNode( "default" );
      styleNameElem.appendChild( styleNameText );
      QDomElement styleTitleElem = doc.createElement( "Title" );
      QDomText styleTitleText = doc.createTextNode( "default" );
      styleTitleElem.appendChild( styleTitleText );
      styleElem.appendChild( styleNameElem );
      styleElem.appendChild( styleTitleElem );
      styleListElem.appendChild( styleElem );
      layerElem.appendChild( styleListElem );

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( "," );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordsElem = doc.createElement( "ows:Keywords" );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( "ows:Keyword" );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
          }
          keywordsElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordsElem );
      }

      // layer data URL
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( "DataURL" );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        dataUrlElem.setAttribute( "format", dataUrlFormat );
        QDomElement dataORElem = doc.createElement( "OnlineResource" );
        dataORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        dataORElem.setAttribute( "xlink:type", "simple" );
        dataORElem.setAttribute( "xlink:href", dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        metaUrlElem.setAttribute( "format", metadataUrlFormat );
        QDomElement metaUrlORElem = doc.createElement( "OnlineResource" );
        metaUrlORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        metaUrlORElem.setAttribute( "xlink:type", "simple" );
        metaUrlORElem.setAttribute( "xlink:href", metadataUrl );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }

      parentElem.appendChild( layerElem );
    }
    else
    {
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

  }
}

void QgsProjectParser::addLayerProjectSettings( QDomElement& layerElem, QDomDocument& doc, QgsMapLayer* currentLayer )
{
  if ( !currentLayer )
  {
    return;
  }

  if ( currentLayer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vLayer = static_cast<QgsVectorLayer*>( currentLayer );
    const QSet<QString>& excludedAttributes = vLayer->excludeAttributesWMS();

    //displayfield
    layerElem.setAttribute( "displayField", vLayer->displayField() );

    //attributes
    QDomElement attributesElem = doc.createElement( "Attributes" );
    const QgsFields& layerFields = vLayer->pendingFields();
    for ( int idx = 0; idx < layerFields.count(); ++idx )
    {
      const QgsField& field = layerFields[idx];
      if ( excludedAttributes.contains( field.name() ) )
      {
        continue;
      }
      QDomElement attributeElem = doc.createElement( "Attribute" );
      attributeElem.setAttribute( "name", vLayer->attributeDisplayName( idx ) );
      attributeElem.setAttribute( "type", QVariant::typeToName( field.type() ) );

      //edit type to text
      QgsVectorLayer::EditType typeEnum = vLayer->editType( idx );
      attributeElem.setAttribute( "editType", editTypeString( typeEnum ) );
      attributeElem.setAttribute( "comment", field.comment() );
      attributeElem.setAttribute( "length", field.length() );
      attributeElem.setAttribute( "precision", field.precision() );
      attributesElem.appendChild( attributeElem );
    }
    layerElem.appendChild( attributesElem );
  }
}

//not very nice, needs to be kept in sync with QgsVectorLayer class...
QString QgsProjectParser::editTypeString( QgsVectorLayer::EditType type )
{
  switch ( type )
  {
    case QgsVectorLayer::LineEdit:
      return "LineEdit";
    case QgsVectorLayer::UniqueValues:
      return "UniqueValues";
    case QgsVectorLayer::UniqueValuesEditable:
      return "UniqueValuesEditable";
    case QgsVectorLayer::ValueMap:
      return "ValueMap";
    case QgsVectorLayer::Classification:
      return "Classification";
    case QgsVectorLayer::EditRange:
      return "EditRange";
    case QgsVectorLayer::SliderRange:
      return "SliderRange";
    case QgsVectorLayer::CheckBox:
      return "CheckBox";
    case QgsVectorLayer::FileName:
      return "FileName";
    case QgsVectorLayer::Enumeration:
      return "Enumeration";
    case QgsVectorLayer::Immutable:
      return "Immutable";
    case QgsVectorLayer::Hidden:
      return "Hidden";
    case QgsVectorLayer::TextEdit:
      return "TextEdit";
    case QgsVectorLayer::Calendar:
      return "Calendar";
    case QgsVectorLayer::DialRange:
      return "DialRange";
    case QgsVectorLayer::ValueRelation:
      return "ValueRelation";
    case QgsVectorLayer::UuidGenerator:
      return "UuidGenerator";
    default:
      return "Unknown";
  }
}

void QgsProjectParser::combineExtentAndCrsOfGroupChildren( QDomElement& groupElem, QDomDocument& doc, bool considerMapExtent ) const
{
  QgsRectangle combinedBBox;
  QSet<QString> combinedCRSSet;
  bool firstBBox = true;
  bool firstCRSSet = true;

  QDomNodeList layerChildren = groupElem.childNodes();
  for ( int j = 0; j < layerChildren.size(); ++j )
  {
    QDomElement childElem = layerChildren.at( j ).toElement();

    if ( childElem.tagName() != "Layer" )
      continue;

    QgsRectangle bbox = layerBoundingBoxInProjectCRS( childElem, doc );
    if ( !bbox.isEmpty() )
    {
      if ( firstBBox )
      {
        combinedBBox = bbox;
        firstBBox = false;
      }
      else
      {
        combinedBBox.combineExtentWith( &bbox );
      }
    }

    //combine crs set
    QSet<QString> crsSet;
    if ( crsSetForLayer( childElem, crsSet ) )
    {
      if ( firstCRSSet )
      {
        combinedCRSSet = crsSet;
        firstCRSSet = false;
      }
      else
      {
        combinedCRSSet.intersect( crsSet );
      }
    }
  }

  appendCRSElementsToLayer( groupElem, doc, combinedCRSSet.toList() );

  const QgsCoordinateReferenceSystem& groupCRS = projectCRS();
  if ( considerMapExtent )
  {
    QgsRectangle mapRect = mapRectangle();
    if ( !mapRect.isEmpty() )
    {
      combinedBBox = mapRect;
    }
  }
  appendLayerBoundingBoxes( groupElem, doc, combinedBBox, groupCRS );
}

QList<QgsMapLayer*> QgsProjectParser::mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache ) const
{
  Q_UNUSED( styleName );
  QList<QgsMapLayer*> layerList;

  //first check if the layer name refers an unpublished layer / group
  if ( mRestrictedLayers.contains( lName ) )
  {
    return layerList;
  }

  if ( !mXMLDoc )
  {
    return layerList;
  }

  //does lName refer to a leaf layer
  QHash< QString, QDomElement >::const_iterator layerElemIt = mProjectLayerElementsByName.find( lName );
  if ( layerElemIt != mProjectLayerElementsByName.constEnd() )
  {
    QgsMapLayer* layer = createLayerFromElement( layerElemIt.value(), useCache );
    if ( layer )
    {
      layerList.push_back( layer );
      return layerList;
    }
  }

  //group or project name
  QDomElement groupElement;
  if ( lName == projectTitle() )
  {
    groupElement = mXMLDoc->documentElement().firstChildElement( "legend" );
  }
  else
  {
    QList<QDomElement>::const_iterator groupIt = mLegendGroupElements.constBegin();
    for ( ; groupIt != mLegendGroupElements.constEnd(); ++groupIt )
    {
      if ( groupIt->attribute( "name" ) == lName )
      {
        groupElement = *groupIt;
        break;
      }
    }
  }

  if ( !groupElement.isNull() )
  {
    //embedded group has no children in this project file
    if ( groupElement.attribute( "embedded" ) == "1" )
    {
      addLayersFromGroup( groupElement, layerList, useCache );
      return layerList;
    }

    //group element found, iterate children and call addLayersFromGroup / addLayerFromLegendLayer for each
    QDomNodeList childList = groupElement.childNodes();
    for ( uint i = 0; i < childList.length(); ++i )
    {
      QDomElement childElem = childList.at( i ).toElement();
      if ( childElem.tagName() == "legendgroup" )
      {
        addLayersFromGroup( childElem, layerList, useCache );
      }
      else if ( childElem.tagName() == "legendlayer" )
      {
        addLayerFromLegendLayer( childElem, layerList, useCache );
      }
    }
    return layerList;
  }

  //still not found. Check if it is a single embedded layer (embedded layers are not contained in mProjectLayerElementsByName)
  QDomElement legendElement = mXMLDoc->documentElement().firstChildElement( "legend" );
  QDomNodeList legendLayerList = legendElement.elementsByTagName( "legendlayer" );
  for ( int i = 0; i < legendLayerList.size(); ++i )
  {
    QDomElement legendLayerElem = legendLayerList.at( i ).toElement();
    if ( legendLayerElem.attribute( "name" ) == lName )
    {
      addLayerFromLegendLayer( legendLayerElem, layerList, useCache );
    }
  }

  //Still not found. Probably it is a layer or a subgroup in an embedded group
  //go through all groups
  //check if they are embedded
  //if yes, request leaf layers and groups from project parser
  QList<QDomElement>::const_iterator legendIt = mLegendGroupElements.constBegin();
  for ( ; legendIt != mLegendGroupElements.constEnd(); ++legendIt )
  {
    if ( legendIt->attribute( "embedded" ) == "1" )
    {
      QString project = convertToAbsolutePath( legendIt->attribute( "project" ) );
      QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
      if ( p )
      {
        const QHash< QString, QDomElement >& pLayerByName = p->mProjectLayerElementsByName;
        QHash< QString, QDomElement >::const_iterator pLayerNameIt = pLayerByName.find( lName );
        if ( pLayerNameIt != pLayerByName.constEnd() )
        {
          layerList.push_back( p->createLayerFromElement( pLayerNameIt.value(), useCache ) );
          break;
        }

        QList<QDomElement>::const_iterator pLegendGroupIt = p->mLegendGroupElements.constBegin();
        for ( ; pLegendGroupIt != p->mLegendGroupElements.constEnd(); ++pLegendGroupIt )
        {
          if ( pLegendGroupIt->attribute( "name" ) == lName )
          {
            p->addLayersFromGroup( *pLegendGroupIt, layerList, useCache );
            break;
          }
        }
      }
    }
  }

  return layerList;
}

QgsMapLayer* QgsProjectParser::mapLayerFromLayerId( const QString& lId ) const
{
  QHash< QString, QDomElement >::const_iterator layerIt = mProjectLayerElementsById.find( lId );
  if ( layerIt != mProjectLayerElementsById.constEnd() )
  {
    return createLayerFromElement( layerIt.value(), true );
  }
  return 0;
}

void QgsProjectParser::addLayersFromGroup( const QDomElement& legendGroupElem, QList<QgsMapLayer*>& layerList, bool useCache ) const
{
  if ( legendGroupElem.attribute( "embedded" ) == "1" ) //embedded group
  {
    //get project parser
    //get group elements from project parser, find the group
    //iterate over layers and add them (embedding in embedded groups does not work)
    QString groupName = legendGroupElem.attribute( "name" );
    QString project = convertToAbsolutePath( legendGroupElem.attribute( "project" ) );
    QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
    if ( !p )
    {
      return;
    }

    QList<QDomElement>  pLegendGroupElems = p->mLegendGroupElements;
    QList<QDomElement>::const_iterator pGroupIt = pLegendGroupElems.constBegin();
    for ( ; pGroupIt != pLegendGroupElems.constEnd(); ++pGroupIt )
    {
      if ( pGroupIt->attribute( "name" ) == groupName )
      {
        p->addLayersFromGroup( *pGroupIt, layerList, useCache );
        return;
      }
    }
  }
  else //normal group
  {
    QDomNodeList groupElemChildren = legendGroupElem.childNodes();
    for ( int i = 0; i < groupElemChildren.size(); ++i )
    {
      QDomElement elem = groupElemChildren.at( i ).toElement();
      if ( elem.tagName() == "legendgroup" )
      {
        addLayersFromGroup( elem, layerList, useCache );
      }
      else if ( elem.tagName() == "legendlayer" )
      {
        addLayerFromLegendLayer( elem, layerList, useCache );
      }
    }
  }
}

void QgsProjectParser::addLayerFromLegendLayer( const QDomElement& legendLayerElem, QList<QgsMapLayer*>& layerList, bool useCache ) const
{
  //get layer id
  //search dom element for <maplayer> element
  //call createLayerFromElement()

  QString id = legendLayerElem.firstChild().firstChild().toElement().attribute( "layerid" );
  QHash< QString, QDomElement >::const_iterator layerIt = mProjectLayerElementsById.find( id );
  if ( layerIt != mProjectLayerElementsById.constEnd() )
  {
    QgsMapLayer* layer = createLayerFromElement( layerIt.value(), useCache );
    if ( layer )
    {
      layerList.append( layer );
    }
  }
}

int QgsProjectParser::layersAndStyles( QStringList& layers, QStringList& styles ) const
{
  layers.clear();
  styles.clear();

  QList<QDomElement>::const_iterator elemIt = mProjectLayerElements.constBegin();

  QString currentLayerName;

  for ( ; elemIt != mProjectLayerElements.constEnd(); ++elemIt )
  {
    currentLayerName = layerName( *elemIt );
    if ( !currentLayerName.isNull() )
    {
      layers << currentLayerName;
      styles << "default";
    }
  }
  return 0;
}

QDomDocument QgsProjectParser::getStyle( const QString& styleName, const QString& layerName ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" );
  myDocument.appendChild( header );

  // Create the root element
  QDomElement root = myDocument.createElementNS( "http://www.opengis.net/sld", "StyledLayerDescriptor" );
  root.setAttribute( "version", "1.1.0" );
  root.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" );
  root.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
  root.setAttribute( "xmlns:se", "http://www.opengis.net/se" );
  root.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  root.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  myDocument.appendChild( root );

  QList<QgsMapLayer*> layerList = mapLayerFromStyle( layerName, styleName );
  if ( layerList.size() < 1 )
  {
    throw QgsMapServiceException( "Error", QString( "The layer for the TypeName '%1' is not found" ).arg( layerName ) );
  }

  QgsMapLayer* currentLayer = layerList.at( 0 );
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  if ( !layer )
  {
    throw QgsMapServiceException( "Error", QString( "Could not get style because:\n%1" ).arg( "Non-vector layers not supported yet" ) );
  }
  // Create the NamedLayer element
  QDomElement namedLayerNode = myDocument.createElement( "NamedLayer" );
  root.appendChild( namedLayerNode );

  QString errorMsg;
  if ( !layer->writeSld( namedLayerNode, myDocument, errorMsg ) )
  {
    throw QgsMapServiceException( "Error", QString( "Could not get style because:\n%1" ).arg( errorMsg ) );
  }
  return myDocument;
}

QgsMapRenderer::OutputUnits QgsProjectParser::outputUnits() const
{
  return QgsMapRenderer::Millimeters;
}

QStringList QgsProjectParser::identifyDisabledLayers() const
{
  QStringList disabledList;
  if ( !mXMLDoc )
  {
    return disabledList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return disabledList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return disabledList;
  }
  QDomElement identifyElem = propertiesElem.firstChildElement( "Identify" );
  if ( identifyElem.isNull() )
  {
    return disabledList;
  }
  QDomElement disabledLayersElem = identifyElem.firstChildElement( "disabledLayers" );
  if ( disabledLayersElem.isNull() )
  {
    return disabledList;
  }
  QDomNodeList valueList = disabledLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    disabledList << valueList.at( i ).toElement().text();
  }
  return disabledList;
}

QStringList QgsProjectParser::wfsLayers() const
{
  QStringList wfsList;
  if ( !mXMLDoc )
  {
    return wfsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return wfsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfsLayersElem = propertiesElem.firstChildElement( "WFSLayers" );
  if ( wfsLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfsLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    wfsList << valueList.at( i ).toElement().text();
  }
  return wfsList;
}

QStringList QgsProjectParser::wfstUpdateLayers() const
{
  QStringList publiedIds = wfsLayers();
  QStringList wfsList;
  if ( !mXMLDoc )
  {
    return wfsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return wfsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstUpdateLayersElem = wfstLayersElem.firstChildElement( "Update" );
  if ( wfstUpdateLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstUpdateLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( publiedIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}

QStringList QgsProjectParser::wfstInsertLayers() const
{
  QStringList updateIds = wfstUpdateLayers();
  QStringList wfsList;
  if ( !mXMLDoc )
  {
    return wfsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return wfsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstInsertLayersElem = wfstLayersElem.firstChildElement( "Insert" );
  if ( wfstInsertLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstInsertLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( updateIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}

QStringList QgsProjectParser::wfstDeleteLayers() const
{
  QStringList insertIds = wfstInsertLayers();
  QStringList wfsList;
  if ( !mXMLDoc )
  {
    return wfsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return wfsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstDeleteLayersElem = wfstLayersElem.firstChildElement( "Delete" );
  if ( wfstDeleteLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstDeleteLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( insertIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}

QStringList QgsProjectParser::supportedOutputCrsList() const
{
  QStringList crsList;
  if ( !mXMLDoc )
  {
    return crsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return crsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return crsList;
  }
  QDomElement wmsCrsElem = propertiesElem.firstChildElement( "WMSCrsList" );
  if ( !wmsCrsElem.isNull() )
  {
    QDomNodeList valueList = wmsCrsElem.elementsByTagName( "value" );
    for ( int i = 0; i < valueList.size(); ++i )
    {
      crsList.append( valueList.at( i ).toElement().text() );
    }
  }
  else
  {
    QDomElement wmsEpsgElem = propertiesElem.firstChildElement( "WMSEpsgList" );
    if ( wmsEpsgElem.isNull() )
    {
      return crsList;
    }
    QDomNodeList valueList = wmsEpsgElem.elementsByTagName( "value" );
    bool conversionOk;
    for ( int i = 0; i < valueList.size(); ++i )
    {
      int epsgNr = valueList.at( i ).toElement().text().toInt( &conversionOk );
      if ( conversionOk )
      {
        crsList.append( QString( "EPSG:%1" ).arg( epsgNr ) );
      }
    }
  }

  return crsList;
}

bool QgsProjectParser::featureInfoWithWktGeometry() const
{
  if ( !mXMLDoc )
  {
    return false;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return false;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return false;
  }
  QDomElement wktElem = propertiesElem.firstChildElement( "WMSAddWktGeometry" );
  if ( wktElem.isNull() )
  {
    return false;
  }

  return ( wktElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
}

QgsRectangle QgsProjectParser::mapRectangle() const
{
  if ( !mXMLDoc )
  {
    return QgsRectangle();
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomElement extentElem = propertiesElem.firstChildElement( "WMSExtent" );
  if ( extentElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomNodeList valueNodeList = extentElem.elementsByTagName( "value" );
  if ( valueNodeList.size() < 4 )
  {
    return QgsRectangle();
  }

  //order of value elements must be xmin, ymin, xmax, ymax
  double xmin = valueNodeList.at( 0 ).toElement().text().toDouble();
  double ymin = valueNodeList.at( 1 ).toElement().text().toDouble();
  double xmax = valueNodeList.at( 2 ).toElement().text().toDouble();
  double ymax = valueNodeList.at( 3 ).toElement().text().toDouble();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QString QgsProjectParser::mapAuthid() const
{
  if ( !mXMLDoc )
  {
    return QString::null;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QString::null;
  }

  QDomElement mapCanvasElem = qgisElem.firstChildElement( "mapcanvas" );
  if ( mapCanvasElem.isNull() )
  {
    return QString::null;
  }

  QDomElement srsElem = mapCanvasElem.firstChildElement( "destinationsrs" );
  if ( srsElem.isNull() )
  {
    return QString::null;
  }

  QDomNodeList authIdNodes = srsElem.elementsByTagName( "authid" );
  if ( authIdNodes.size() < 1 )
  {
    return QString::null;
  }

  return authIdNodes.at( 0 ).toElement().text();
}

QString QgsProjectParser::projectTitle() const
{
  if ( !mXMLDoc )
  {
    return QString();
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QString();
  }

  QDomElement titleElem = qgisElem.firstChildElement( "title" );
  if ( !titleElem.isNull() )
  {
    QString title = titleElem.text();
    if ( !title.isEmpty() )
    {
      return title;
    }
  }

  //no title element or not project title set. Use project filename without extension
  QFileInfo projectFileInfo( mProjectPath );
  return projectFileInfo.baseName();
}

QgsMapLayer* QgsProjectParser::createLayerFromElement( const QDomElement& elem, bool useCache ) const
{
  if ( elem.isNull() || !mXMLDoc )
  {
    return 0;
  }

  QDomElement dataSourceElem = elem.firstChildElement( "datasource" );
  QString uri = dataSourceElem.text();
  QString absoluteUri;
  if ( !dataSourceElem.isNull() )
  {
    //convert relative pathes to absolute ones if necessary
    if ( uri.startsWith( "dbname" ) ) //database
    {
      QgsDataSourceURI dsUri( uri );
      if ( dsUri.host().isEmpty() ) //only convert path for file based databases
      {
        QString dbnameUri = dsUri.database();
        QString dbNameUriAbsolute = convertToAbsolutePath( dbnameUri );
        if ( dbnameUri != dbNameUriAbsolute )
        {
          dsUri.setDatabase( dbNameUriAbsolute );
          absoluteUri = dsUri.uri();
          QDomText absoluteTextNode = mXMLDoc->createTextNode( absoluteUri );
          dataSourceElem.replaceChild( absoluteTextNode, dataSourceElem.firstChild() );
        }
      }
    }
    else if ( uri.startsWith( "file:" ) ) //a file based datasource in url notation (e.g. delimited text layer)
    {
      QString filePath = uri.mid( 5, uri.indexOf( "?" ) - 5 );
      QString absoluteFilePath = convertToAbsolutePath( filePath );
      if ( filePath != absoluteFilePath )
      {
        QUrl destUrl = QUrl::fromEncoded( uri.toAscii() );
        destUrl.setScheme( "file" );
        destUrl.setPath( absoluteFilePath );
        absoluteUri = destUrl.toEncoded();
        QDomText absoluteTextNode = mXMLDoc->createTextNode( absoluteUri );
        dataSourceElem.replaceChild( absoluteTextNode, dataSourceElem.firstChild() );
      }
      else
      {
        absoluteUri = uri;
      }
    }
    else //file based data source
    {
      absoluteUri = convertToAbsolutePath( uri );
      if ( uri != absoluteUri )
      {
        QDomText absoluteTextNode = mXMLDoc->createTextNode( absoluteUri );
        dataSourceElem.replaceChild( absoluteTextNode, dataSourceElem.firstChild() );
      }
    }
  }

  QString id = layerId( elem );
  QgsMapLayer* layer = 0;
  if ( useCache )
  {
    layer = QgsMSLayerCache::instance()->searchLayer( absoluteUri, id );
  }

  if ( layer )
  {
    return layer;
  }

  QString type = elem.attribute( "type" );
  if ( type == "vector" )
  {
    layer = new QgsVectorLayer();
  }
  else if ( type == "raster" )
  {
    layer = new QgsRasterLayer();
  }
  else if ( elem.attribute( "embedded" ) == "1" ) //layer is embedded from another project file
  {
    QString project = convertToAbsolutePath( elem.attribute( "project" ) );
    QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
    QgsProjectParser* otherConfig = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
    if ( !otherConfig )
    {
      return 0;
    }

    QHash< QString, QDomElement >::const_iterator layerIt = otherConfig->mProjectLayerElementsById.find( elem.attribute( "id" ) );
    if ( layerIt == otherConfig->mProjectLayerElementsById.constEnd() )
    {
      return 0;
    }
    return otherConfig->createLayerFromElement( layerIt.value() );
  }

  if ( layer )
  {
    layer->readLayerXML( const_cast<QDomElement&>( elem ) ); //should be changed to const in QgsMapLayer
    layer->setLayerName( layerName( elem ) );
    if ( useCache )
    {
      QgsMSLayerCache::instance()->insertLayer( absoluteUri, id, layer, mProjectPath );
    }
    else
    {
      mLayersToRemove.push_back( layer );
    }
  }
  return layer;
}

QString QgsProjectParser::layerId( const QDomElement& layerElem ) const
{
  if ( layerElem.isNull() )
  {
    return QString();
  }

  QDomElement idElem = layerElem.firstChildElement( "id" );
  if ( idElem.isNull() )
  {
    //embedded layer have id attribute instead of id child element
    return layerElem.attribute( "id" );
  }
  return idElem.text();
}

QString QgsProjectParser::layerName( const QDomElement& layerElem ) const
{
  if ( layerElem.isNull() )
  {
    return QString();
  }

  QDomElement nameElem = layerElem.firstChildElement( "layername" );
  if ( nameElem.isNull() )
  {
    return QString();
  }
  return nameElem.text().replace( "," , "%60" ); //commas are not allowed in layer names
}

void QgsProjectParser::setLegendParametersFromProject()
{
  if ( !mXMLDoc )
  {
    return;
  }

  QDomElement documentElem = mXMLDoc->documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }

  QDomElement composerElem = documentElem.firstChildElement( "Composer" );
  if ( composerElem.isNull() )
  {
    return;
  }
  QDomElement composerLegendElem = composerElem.firstChildElement( "ComposerLegend" );
  if ( composerLegendElem.isNull() )
  {
    return;
  }

  mLegendBoxSpace = composerLegendElem.attribute( "boxSpace" ).toDouble();
  mLegendLayerSpace = composerLegendElem.attribute( "layerSpace" ).toDouble();
  mLegendSymbolSpace = composerLegendElem.attribute( "symbolSpace" ).toDouble();
  mLegendIconLabelSpace = composerLegendElem.attribute( "iconLabelSpace" ).toDouble();
  mLegendSymbolWidth = composerLegendElem.attribute( "symbolWidth" ).toDouble();
  mLegendSymbolHeight = composerLegendElem.attribute( "symbolHeight" ).toDouble();
  mLegendLayerFont.fromString( composerLegendElem.attribute( "layerFont" ) );
  mLegendItemFont.fromString( composerLegendElem.attribute( "itemFont" ) );
}

QList< GroupLayerInfo > QgsProjectParser::groupLayerRelationshipFromProject() const
{
  QList< GroupLayerInfo > resultList;
  if ( !mXMLDoc )
  {
    return resultList;
  }

  QDomElement documentElem = mXMLDoc->documentElement();
  if ( documentElem.isNull() )
  {
    return resultList;
  }
  QDomElement legendElem = documentElem.firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return resultList;
  }

  QDomNodeList legendChildren = legendElem.childNodes();
  QDomElement currentChildElem;
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QList< QString > layerIdList;
    currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.isNull() )
    {
      continue;
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      layerIdList.push_back( layerIdFromLegendLayer( currentChildElem ) );
      resultList.push_back( qMakePair( QString(), layerIdList ) );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      QDomNodeList childLayerList = currentChildElem.elementsByTagName( "legendlayer" );
      QString groupName = currentChildElem.attribute( "name" ).replace( "," , "%60" );
      QString currentLayerId;

      for ( int j = 0; j < childLayerList.size(); ++j )
      {
        layerIdList.push_back( layerIdFromLegendLayer( childLayerList.at( j ).toElement() ) );
      }
      resultList.push_back( qMakePair( groupName, layerIdList ) );
    }
  }

  return resultList;
}

QString QgsProjectParser::layerIdFromLegendLayer( const QDomElement& legendLayer ) const
{
  if ( legendLayer.isNull() )
  {
    return QString();
  }

  QDomNodeList legendLayerFileList = legendLayer.elementsByTagName( "legendlayerfile" );
  if ( legendLayerFileList.size() < 1 )
  {
    return QString();
  }

  return legendLayerFileList.at( 0 ).toElement().attribute( "layerid" );
}

QgsComposition* QgsProjectParser::initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlList ) const
{
  //Create composition from xml
  QDomElement composerElem = composerByName( composerTemplate );
  if ( composerElem.isNull() )
  {
    throw QgsMapServiceException( "Error", "Composer template not found" );
  }

  QDomElement compositionElem = composerElem.firstChildElement( "Composition" );
  if ( compositionElem.isNull() )
  {
    return 0;
  }

  QgsComposition* composition = new QgsComposition( mapRenderer ); //set resolution, paper size from composer element attributes
  if ( !composition->readXML( compositionElem, *mXMLDoc ) )
  {
    delete composition;
    return 0;
  }

  composition->addItemsFromXML( compositionElem, *mXMLDoc );

  labelList.clear();
  mapList.clear();
  htmlList.clear();

  QList<QgsComposerItem* > itemList;
  composition->composerItems( itemList );
  QList<QgsComposerItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerLabel* label = dynamic_cast< QgsComposerLabel *>( *itemIt );
    if ( label )
    {
      labelList.push_back( label );
      continue;
    }
    QgsComposerMap* map = dynamic_cast< QgsComposerMap *>( *itemIt );
    if ( map )
    {
      mapList.push_back( map );
      continue;
    }
    QgsComposerPicture* pic = dynamic_cast< QgsComposerPicture *>( *itemIt );
    if ( pic )
    {
      pic->setPictureFile( convertToAbsolutePath(( pic )->pictureFile() ) );
      continue;
    }
    const QgsComposerHtml* html = composition->getComposerHtmlByItem( *itemIt );
    if ( html )
    {
      htmlList.push_back( html );
      continue;
    }
  }

  return composition;
}

void QgsProjectParser::printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( !mXMLDoc )
  {
    return;
  }

  QList<QDomElement> composerElemList = publishedComposerElements();
  if ( composerElemList.size() < 1 )
  {
    return;
  }

  QDomElement composerTemplatesElem = doc.createElement( "ComposerTemplates" );

  QList<QDomElement>::const_iterator composerElemIt = composerElemList.constBegin();
  for ( ; composerElemIt != composerElemList.constEnd(); ++composerElemIt )
  {
    QDomElement composerTemplateElem = doc.createElement( "ComposerTemplate" );
    QDomElement currentComposerElem = *composerElemIt;
    if ( currentComposerElem.isNull() )
    {
      continue;
    }

    composerTemplateElem.setAttribute( "name", currentComposerElem.attribute( "title" ) );

    //get paper width and hight in mm from composition
    QDomElement compositionElem = currentComposerElem.firstChildElement( "Composition" );
    if ( compositionElem.isNull() )
    {
      continue;
    }
    composerTemplateElem.setAttribute( "width", compositionElem.attribute( "paperWidth" ) );
    composerTemplateElem.setAttribute( "height", compositionElem.attribute( "paperHeight" ) );


    //add available composer maps and their size in mm
    QDomNodeList composerMapList = currentComposerElem.elementsByTagName( "ComposerMap" );
    for ( int j = 0; j < composerMapList.size(); ++j )
    {
      QDomElement cmap = composerMapList.at( j ).toElement();
      QDomElement citem = cmap.firstChildElement( "ComposerItem" );
      if ( citem.isNull() )
      {
        continue;
      }

      QDomElement composerMapElem = doc.createElement( "ComposerMap" );
      composerMapElem.setAttribute( "name", "map" + cmap.attribute( "id" ) );
      composerMapElem.setAttribute( "width", citem.attribute( "width" ) );
      composerMapElem.setAttribute( "height", citem.attribute( "height" ) );
      composerTemplateElem.appendChild( composerMapElem );
    }

    //add available composer labels
    QDomNodeList composerLabelList = currentComposerElem.elementsByTagName( "ComposerLabel" );
    for ( int j = 0; j < composerLabelList.size(); ++j )
    {
      QDomElement citem = composerLabelList.at( j ).firstChildElement( "ComposerItem" );
      QString id = citem.attribute( "id" );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerLabelElem = doc.createElement( "ComposerLabel" );
      composerLabelElem.setAttribute( "name", id );
      composerTemplateElem.appendChild( composerLabelElem );
    }

    //add available composer HTML
    QDomNodeList composerHtmlList = currentComposerElem.elementsByTagName( "ComposerHtml" );
    for ( int j = 0; j < composerHtmlList.size(); ++j )
    {
      QDomElement citem = composerHtmlList.at( j ).firstChildElement( "ComposerFrame" ).firstChildElement( "ComposerItem" );
      QString id = citem.attribute( "id" );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerHtmlElem = doc.createElement( "ComposerHtml" );
      composerHtmlElem.setAttribute( "name", id );
      composerTemplateElem.appendChild( composerHtmlElem );
    }

    composerTemplatesElem.appendChild( composerTemplateElem );
  }
  parentElement.appendChild( composerTemplatesElem );
}

QDomElement QgsProjectParser::composerByName( const QString& composerName ) const
{
  QDomElement composerElem;
  if ( !mXMLDoc )
  {
    return composerElem;
  }

  QList<QDomElement> composerElemList = publishedComposerElements();
  QList<QDomElement>::const_iterator composerIt = composerElemList.constBegin();
  for ( ; composerIt != composerElemList.constEnd(); ++composerIt )
  {
    QDomElement currentComposerElem = *composerIt;
    if ( currentComposerElem.attribute( "title" ) == composerName )
    {
      return currentComposerElem;
    }
  }

  return composerElem;
}

QList<QDomElement> QgsProjectParser::publishedComposerElements() const
{
  QList<QDomElement> composerElemList;
  if ( !mXMLDoc )
  {
    return composerElemList;
  }

  QDomNodeList composerNodeList = mXMLDoc->elementsByTagName( "Composer" );

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  QDomElement wmsComposerListElem = propertiesElem.firstChildElement( "WMSComposerList" );
  if ( wmsComposerListElem.isNull() )
  {
    for ( unsigned int i = 0; i < composerNodeList.length(); ++i )
    {
      composerElemList.push_back( composerNodeList.at( i ).toElement() );
    }
    return composerElemList;
  }

  QSet<QString> publishedComposerNames;
  QDomNodeList valueList = wmsComposerListElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    publishedComposerNames.insert( valueList.at( i ).toElement().text() );
  }

  //remove unpublished composers from list
  QString currentComposerName;
  QDomElement currentElem;
  for ( int i = 0; i < composerNodeList.size(); ++i )
  {
    currentElem = composerNodeList.at( i ).toElement();
    currentComposerName = currentElem.attribute( "title" );
    if ( publishedComposerNames.contains( currentComposerName ) )
    {
      composerElemList.push_back( currentElem );
    }
  }

  return composerElemList;
}

void QgsProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  QString docElementTagName = doc.documentElement().tagName();
  if ( docElementTagName == "WFS_Capabilities" )
  {
    serviceWFSCapabilities( parentElement, doc );
    return;
  }

  QDomElement serviceElem = doc.createElement( "Service" );

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  QDomElement serviceCapabilityElem = propertiesElem.firstChildElement( "WMSServiceCapabilities" );
  if ( serviceCapabilityElem.isNull() || serviceCapabilityElem.text().compare( "true", Qt::CaseInsensitive ) != 0 )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  //Service name is always WMS
  QDomElement wmsNameElem = doc.createElement( "Name" );
  QDomText wmsNameText = doc.createTextNode( "WMS" );
  wmsNameElem.appendChild( wmsNameText );
  serviceElem.appendChild( wmsNameElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    serviceElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    serviceElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    bool siaFormat = featureInfoFormatSIA2045();

    QDomElement wmsKeywordElem = doc.createElement( "KeywordList" );
    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      QDomElement keywordElem = doc.createElement( "Keyword" );
      QDomText keywordText = doc.createTextNode( keywordList.at( i ).toElement().text() );
      keywordElem.appendChild( keywordText );
      if ( siaFormat )
      {
        keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
      }
      wmsKeywordElem.appendChild( keywordElem );
    }

    if ( keywordList.size() > 0 )
    {
      serviceElem.appendChild( wmsKeywordElem );
    }
  }

  //OnlineResource element is mandatory according to the WMS specification
  QDomElement wmsOnlineResourceElem = propertiesElem.firstChildElement( "WMSOnlineResource" );
  QDomElement onlineResourceElem = doc.createElement( "OnlineResource" );
  onlineResourceElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  onlineResourceElem.setAttribute( "xlink:type", "simple" );
  if ( !wmsOnlineResourceElem.isNull() )
  {
    onlineResourceElem.setAttribute( "xlink:href", wmsOnlineResourceElem.text() );
  }

  serviceElem.appendChild( onlineResourceElem );

  //Contact information
  QDomElement contactInfoElem = doc.createElement( "ContactInformation" );

  //Contact person primary
  QDomElement contactPersonPrimaryElem = doc.createElement( "ContactPersonPrimary" );

  //Contact person
  QDomElement contactPersonElem = propertiesElem.firstChildElement( "WMSContactPerson" );
  QString contactPersonString;
  if ( !contactPersonElem.isNull() )
  {
    contactPersonString = contactPersonElem.text();
  }
  QDomElement wmsContactPersonElem = doc.createElement( "ContactPerson" );
  QDomText contactPersonText = doc.createTextNode( contactPersonString );
  wmsContactPersonElem.appendChild( contactPersonText );
  contactPersonPrimaryElem.appendChild( wmsContactPersonElem );


  //Contact organisation
  QDomElement contactOrganizationElem = propertiesElem.firstChildElement( "WMSContactOrganization" );
  QString contactOrganizationString;
  if ( !contactOrganizationElem.isNull() )
  {
    contactOrganizationString = contactOrganizationElem.text();
  }
  QDomElement wmsContactOrganizationElem = doc.createElement( "ContactOrganization" );
  QDomText contactOrganizationText = doc.createTextNode( contactOrganizationString );
  wmsContactOrganizationElem.appendChild( contactOrganizationText );
  contactPersonPrimaryElem.appendChild( wmsContactOrganizationElem );
  contactInfoElem.appendChild( contactPersonPrimaryElem );

  //phone
  QDomElement phoneElem = propertiesElem.firstChildElement( "WMSContactPhone" );
  if ( !phoneElem.isNull() )
  {
    QDomElement wmsPhoneElem = doc.createElement( "ContactVoiceTelephone" );
    QDomText wmsPhoneText = doc.createTextNode( phoneElem.text() );
    wmsPhoneElem.appendChild( wmsPhoneText );
    contactInfoElem.appendChild( wmsPhoneElem );
  }

  //mail
  QDomElement mailElem = propertiesElem.firstChildElement( "WMSContactMail" );
  if ( !mailElem.isNull() )
  {
    QDomElement wmsMailElem = doc.createElement( "ContactElectronicMailAddress" );
    QDomText wmsMailText = doc.createTextNode( mailElem.text() );
    wmsMailElem.appendChild( wmsMailText );
    contactInfoElem.appendChild( wmsMailElem );
  }

  serviceElem.appendChild( contactInfoElem );

  //Fees
  QDomElement feesElem = propertiesElem.firstChildElement( "WMSFees" );
  if ( !feesElem.isNull() )
  {
    QDomElement wmsFeesElem = doc.createElement( "Fees" );
    QDomText wmsFeesText = doc.createTextNode( feesElem.text() );
    wmsFeesElem.appendChild( wmsFeesText );
    serviceElem.appendChild( wmsFeesElem );
  }

  //AccessConstraints
  QDomElement accessConstraintsElem = propertiesElem.firstChildElement( "WMSAccessConstraints" );
  if ( !accessConstraintsElem.isNull() )
  {
    QDomElement wmsAccessConstraintsElem = doc.createElement( "AccessConstraints" );
    QDomText wmsAccessConstraintsText = doc.createTextNode( accessConstraintsElem.text() );
    wmsAccessConstraintsElem.appendChild( wmsAccessConstraintsText );
    serviceElem.appendChild( wmsAccessConstraintsElem );
  }

  //MaxWidth / MaxHeight for WMS 1.3
  QString version = doc.documentElement().attribute( "version" );
  if ( version != "1.1.1" )
  {
    if ( mMaxWidth != -1 )
    {
      QDomElement maxWidthElem = doc.createElement( "MaxWidth" );
      QDomText maxWidthText = doc.createTextNode( QString::number( mMaxWidth ) );
      maxWidthElem.appendChild( maxWidthText );
      serviceElem.appendChild( maxWidthElem );
    }
    if ( mMaxHeight != -1 )
    {
      QDomElement maxHeightElem = doc.createElement( "MaxHeight" );
      QDomText maxHeightText = doc.createTextNode( QString::number( mMaxHeight ) );
      maxHeightElem.appendChild( maxHeightText );
      serviceElem.appendChild( maxHeightElem );
    }
  }

  parentElement.appendChild( serviceElem );
}

void QgsProjectParser::serviceWFSCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  QDomElement serviceElem = doc.createElement( "Service" );

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  QDomElement serviceCapabilityElem = propertiesElem.firstChildElement( "WMSServiceCapabilities" );
  if ( serviceCapabilityElem.isNull() || serviceCapabilityElem.text().compare( "true", Qt::CaseInsensitive ) != 0 )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  //Service name is always WMS
  QDomElement wmsNameElem = doc.createElement( "Name" );
  QDomText wmsNameText = doc.createTextNode( "WFS" );
  wmsNameElem.appendChild( wmsNameText );
  serviceElem.appendChild( wmsNameElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    serviceElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    serviceElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    bool siaFormat = featureInfoFormatSIA2045();

    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    QStringList keywords;
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      keywords << keywordList.at( i ).toElement().text();
    }

    if ( keywordList.size() > 0 )
    {
      QDomElement wfsKeywordElem = doc.createElement( "Keywords" );
      QDomText keywordText = doc.createTextNode( keywords.join( ", " ) );
      wfsKeywordElem.appendChild( keywordText );
      if ( siaFormat )
      {
        wfsKeywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
      }
      serviceElem.appendChild( wfsKeywordElem );
    }
  }

  //OnlineResource element is mandatory according to the WMS specification
  QDomElement wmsOnlineResourceElem = propertiesElem.firstChildElement( "WMSOnlineResource" );
  QDomElement onlineResourceElem = doc.createElement( "OnlineResource" );
  onlineResourceElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  onlineResourceElem.setAttribute( "xlink:type", "simple" );
  if ( !wmsOnlineResourceElem.isNull() )
  {
    onlineResourceElem.setAttribute( "xlink:href", wmsOnlineResourceElem.text() );
  }

  serviceElem.appendChild( onlineResourceElem );

  //Fees
  QDomElement feesElem = propertiesElem.firstChildElement( "WMSFees" );
  if ( !feesElem.isNull() )
  {
    QDomElement wmsFeesElem = doc.createElement( "Fees" );
    QDomText wmsFeesText = doc.createTextNode( feesElem.text() );
    wmsFeesElem.appendChild( wmsFeesText );
    serviceElem.appendChild( wmsFeesElem );
  }

  //AccessConstraints
  QDomElement accessConstraintsElem = propertiesElem.firstChildElement( "WMSAccessConstraints" );
  if ( !accessConstraintsElem.isNull() )
  {
    QDomElement wmsAccessConstraintsElem = doc.createElement( "AccessConstraints" );
    QDomText wmsAccessConstraintsText = doc.createTextNode( accessConstraintsElem.text() );
    wmsAccessConstraintsElem.appendChild( wmsAccessConstraintsText );
    serviceElem.appendChild( wmsAccessConstraintsElem );
  }
  parentElement.appendChild( serviceElem );
}

QString QgsProjectParser::serviceUrl() const
{
  QString url;

  if ( !mXMLDoc )
  {
    return url;
  }

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( !propertiesElem.isNull() )
  {
    QDomElement wmsUrlElem = propertiesElem.firstChildElement( "WMSUrl" );
    if ( !wmsUrlElem.isNull() )
    {
      url = wmsUrlElem.text();
    }
  }
  return url;
}

QString QgsProjectParser::wfsServiceUrl() const
{
  QString url;

  if ( !mXMLDoc )
  {
    return url;
  }

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( !propertiesElem.isNull() )
  {
    QDomElement wfsUrlElem = propertiesElem.firstChildElement( "WFSUrl" );
    if ( !wfsUrlElem.isNull() )
    {
      url = wfsUrlElem.text();
    }
  }
  return url;
}

QStringList QgsProjectParser::wfsLayerNames() const
{
  QStringList layerNameList;

  QMap<QString, QgsMapLayer*> layerMap;
  projectLayerMap( layerMap );

  QgsMapLayer* currentLayer = 0;
  QStringList wfsIdList = wfsLayers();
  QStringList::const_iterator wfsIdIt = wfsIdList.constBegin();
  for ( ; wfsIdIt != wfsIdList.constEnd(); ++wfsIdIt )
  {
    QMap<QString, QgsMapLayer*>::const_iterator layerMapIt = layerMap.find( *wfsIdIt );
    if ( layerMapIt != layerMap.constEnd() )
    {
      currentLayer = layerMapIt.value();
      if ( currentLayer )
      {
        layerNameList.append( currentLayer->name() );
      }
    }
  }

  return layerNameList;
}

QHash<QString, QString> QgsProjectParser::featureInfoLayerAliasMap() const
{
  QHash<QString, QString> aliasMap;
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return aliasMap;
  }

  //WMSFeatureInfoAliasLayers
  QStringList aliasLayerStringList;
  QDomElement featureInfoAliasLayersElem = propertiesElem.firstChildElement( "WMSFeatureInfoAliasLayers" );
  if ( featureInfoAliasLayersElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList aliasLayerValueList = featureInfoAliasLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < aliasLayerValueList.size(); ++i )
  {
    aliasLayerStringList << aliasLayerValueList.at( i ).toElement().text();
  }

  //WMSFeatureInfoLayerAliases
  QStringList layerAliasStringList;
  QDomElement featureInfoLayerAliasesElem = propertiesElem.firstChildElement( "WMSFeatureInfoLayerAliases" );
  if ( featureInfoLayerAliasesElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList layerAliasesValueList = featureInfoLayerAliasesElem.elementsByTagName( "value" );
  for ( int i = 0; i < layerAliasesValueList.size(); ++i )
  {
    layerAliasStringList << layerAliasesValueList.at( i ).toElement().text();
  }

  int nMapEntries = qMin( aliasLayerStringList.size(), layerAliasStringList.size() );
  for ( int i = 0; i < nMapEntries; ++i )
  {
    aliasMap.insert( aliasLayerStringList.at( i ), layerAliasStringList.at( i ) );
  }

  return aliasMap;
}

QString QgsProjectParser::featureInfoDocumentElement( const QString& defaultValue ) const
{
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return defaultValue;
  }
  QDomElement featureInfoDocumentElem = propertiesElem.firstChildElement( "WMSFeatureInfoDocumentElement" );
  if ( featureInfoDocumentElem.isNull() )
  {
    return defaultValue;
  }
  return featureInfoDocumentElem.text();
}

QString QgsProjectParser::featureInfoDocumentElementNS() const
{
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return "";
  }
  QDomElement featureInfoDocumentNSElem = propertiesElem.firstChildElement( "WMSFeatureInfoDocumentElementNS" );
  if ( featureInfoDocumentNSElem.isNull() )
  {
    return "";
  }
  return featureInfoDocumentNSElem.text();
}

QString QgsProjectParser::featureInfoSchema() const
{
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return "";
  }
  QDomElement featureInfoSchemaElem = propertiesElem.firstChildElement( "WMSFeatureInfoSchema" );
  if ( featureInfoSchemaElem.isNull() )
  {
    return "";
  }
  return featureInfoSchemaElem.text();
}

bool QgsProjectParser::featureInfoFormatSIA2045() const
{
  if ( !mXMLDoc )
  {
    return false;
  }

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return false;
  }

  QDomElement sia2045Elem = propertiesElem.firstChildElement( "WMSInfoFormatSIA2045" );
  if ( sia2045Elem.isNull() )
  {
    return false;
  }

  if ( sia2045Elem.text().compare( "enabled", Qt::CaseInsensitive ) == 0
       || sia2045Elem.text().compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    return true;
  }
  return false;
}

QString QgsProjectParser::convertToAbsolutePath( const QString& file ) const
{
  if ( !file.startsWith( "./" ) && !file.startsWith( "../" ) )
  {
    return file;
  }

  QString srcPath = file;
  QString projPath = mProjectPath;

#if defined(Q_OS_WIN)
  srcPath.replace( "\\", "/" );
  projPath.replace( "\\", "/" );

  bool uncPath = projPath.startsWith( "//" );
#endif

  QStringList srcElems = file.split( "/", QString::SkipEmptyParts );
  QStringList projElems = mProjectPath.split( "/", QString::SkipEmptyParts );

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.insert( 0, "" );
    projElems.insert( 0, "" );
  }
#endif

  // remove project file element
  projElems.removeLast();

  // append source path elements
  projElems << srcElems;
  projElems.removeAll( "." );

  // resolve ..
  int pos;
  while (( pos = projElems.indexOf( ".." ) ) > 0 )
  {
    // remove preceding element and ..
    projElems.removeAt( pos - 1 );
    projElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  projElems.prepend( "" );
#endif

  return projElems.join( "/" );
}

void QgsProjectParser::setSelectionColor()
{
  int red = 255;
  int green = 255;
  int blue = 0;
  int alpha = 255;

  //overwrite default selection color with settings from the project
  if ( mXMLDoc )
  {
    QDomElement qgisElem = mXMLDoc->documentElement();
    if ( !qgisElem.isNull() )
    {
      QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
      if ( !propertiesElem.isNull() )
      {
        QDomElement guiElem = propertiesElem.firstChildElement( "Gui" );
        if ( !guiElem.isNull() )
        {
          QDomElement redElem = guiElem.firstChildElement( "SelectionColorRedPart" );
          if ( !redElem.isNull() )
          {
            red = redElem.text().toInt();
          }
          QDomElement greenElem = guiElem.firstChildElement( "SelectionColorGreenPart" );
          if ( !greenElem.isNull() )
          {
            green = greenElem.text().toInt();
          }
          QDomElement blueElem = guiElem.firstChildElement( "SelectionColorBluePart" );
          if ( !blueElem.isNull() )
          {
            blue = blueElem.text().toInt();
          }
          QDomElement alphaElem = guiElem.firstChildElement( "SelectionColorAlphaPart" );
          if ( !alphaElem.isNull() )
          {
            alpha = alphaElem.text().toInt();
          }
        }
      }
    }
  }

  mSelectionColor = QColor( red, green, blue, alpha );
}

void QgsProjectParser::setMaxWidthHeight()
{
  if ( mXMLDoc )
  {
    QDomElement qgisElem = mXMLDoc->documentElement();
    if ( !qgisElem.isNull() )
    {
      QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
      if ( !propertiesElem.isNull() )
      {
        QDomElement maxWidthElem = propertiesElem.firstChildElement( "WMSMaxWidth" );
        if ( !maxWidthElem.isNull() )
        {
          mMaxWidth = maxWidthElem.text().toInt();
        }
        QDomElement maxHeightElem = propertiesElem.firstChildElement( "WMSMaxHeight" );
        if ( !maxHeightElem.isNull() )
        {
          mMaxHeight = maxHeightElem.text().toInt();
        }
      }
    }
  }
}

const QgsCoordinateReferenceSystem& QgsProjectParser::projectCRS() const
{
  //mapcanvas->destinationsrs->spatialrefsys->authid
  if ( mXMLDoc )
  {
    QDomElement authIdElem = mXMLDoc->documentElement().firstChildElement( "mapcanvas" ).firstChildElement( "destinationsrs" ).
                             firstChildElement( "spatialrefsys" ).firstChildElement( "authid" );
    if ( !authIdElem.isNull() )
    {
      return QgsCRSCache::instance()->crsByAuthId( authIdElem.text() );
    }
  }
  return QgsCRSCache::instance()->crsByEpsgId( GEO_EPSG_CRS_ID );
}

QgsRectangle QgsProjectParser::layerBoundingBoxInProjectCRS( const QDomElement& layerElem, const QDomDocument &doc ) const
{
  QgsRectangle BBox;
  if ( layerElem.isNull() )
  {
    return BBox;
  }

  //read box coordinates and layer auth. id
  QDomElement boundingBoxElem = layerElem.firstChildElement( "BoundingBox" );
  if ( boundingBoxElem.isNull() )
  {
    return BBox;
  }

  double minx, miny, maxx, maxy;
  bool conversionOk;
  minx = boundingBoxElem.attribute( "minx" ).toDouble( &conversionOk );
  if ( !conversionOk )
  {
    return BBox;
  }
  miny = boundingBoxElem.attribute( "miny" ).toDouble( &conversionOk );
  if ( !conversionOk )
  {
    return BBox;
  }
  maxx = boundingBoxElem.attribute( "maxx" ).toDouble( &conversionOk );
  if ( !conversionOk )
  {
    return BBox;
  }
  maxy = boundingBoxElem.attribute( "maxy" ).toDouble( &conversionOk );
  if ( !conversionOk )
  {
    return BBox;
  }


  QString version = doc.documentElement().attribute( "version" );

  //create layer crs
  const QgsCoordinateReferenceSystem& layerCrs = QgsCRSCache::instance()->crsByAuthId( boundingBoxElem.attribute( version == "1.1.1" ? "SRS" : "CRS" ) );
  if ( !layerCrs.isValid() )
  {
    return BBox;
  }

  BBox.setXMinimum( minx );
  BBox.setXMaximum( maxx );
  BBox.setYMinimum( miny );
  BBox.setYMaximum( maxy );

  if ( version != "1.1.1" && layerCrs.axisInverted() )
  {
    BBox.invert();
  }

  //get project crs
  const QgsCoordinateReferenceSystem& projectCrs = projectCRS();
  QgsCoordinateTransform t( layerCrs, projectCrs );

  //transform
  BBox = t.transformBoundingBox( BBox );
  return BBox;
}

void QgsProjectParser::addDrawingOrder( QDomElement elem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const
{
  if ( elem.isNull() )
  {
    return;
  }

  if ( elem.tagName() == "legendgroup" )
  {
    if ( elem.attribute( "embedded" ) == "1" )
    {
      addDrawingOrderEmbeddedGroup( elem, orderedLayerList, useDrawingOrder );
    }
    else
    {
      QDomNodeList groupChildren = elem.childNodes();
      for ( int i = 0; i < groupChildren.size(); ++i )
      {
        addDrawingOrder( groupChildren.at( i ).toElement(), useDrawingOrder, orderedLayerList );
      }
    }
  }
  else if ( elem.tagName() == "legendlayer" )
  {
    QString layerName = elem.attribute( "name" );
    if ( useDrawingOrder )
    {
      int drawingOrder = elem.attribute( "drawingOrder", "-1" ).toInt();
      orderedLayerList.insert( drawingOrder, layerName );
    }
    else
    {
      orderedLayerList.insert( orderedLayerList.size(), layerName );
    }
  }
}

void QgsProjectParser::addDrawingOrder( QDomElement& parentElem, QDomDocument& doc ) const
{
  if ( !mXMLDoc )
  {
    return;
  }

  //find legend section
  QDomElement legendElement = mXMLDoc->documentElement().firstChildElement( "legend" );
  if ( legendElement.isNull() )
  {
    return;
  }

  bool useDrawingOrder = ( legendElement.attribute( "updateDrawingOrder" ) == "false" );
  QMap<int, QString> orderedLayerNames;

  QDomNodeList legendChildren = legendElement.childNodes();
  QDomElement childElem;
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    addDrawingOrder( legendChildren.at( i ).toElement(), useDrawingOrder, orderedLayerNames );
  }

  QStringList layerList;
  QMap<int, QString>::const_iterator nameIt = orderedLayerNames.constBegin();
  for ( ; nameIt != orderedLayerNames.constEnd(); ++nameIt )
  {
    layerList.prepend( nameIt.value() );
  }

  QDomElement layerDrawingOrderElem = doc.createElement( "LayerDrawingOrder" );
  QDomText drawingOrderText = doc.createTextNode( layerList.join( "," ) );
  layerDrawingOrderElem.appendChild( drawingOrderText );
  parentElem.appendChild( layerDrawingOrderElem );
}

void QgsProjectParser::projectLayerMap( QMap<QString, QgsMapLayer*>& layerMap ) const
{
  layerMap.clear();
  foreach ( const QDomElement &elem, mProjectLayerElements )
  {
    QgsMapLayer *layer = createLayerFromElement( elem );
    if ( layer )
    {
      QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
      layerMap.insert( layer->id(), layer );
    }
  }
}

QSet<QString> QgsProjectParser::restrictedLayers() const
{
  QSet<QString> restrictedLayerSet;

  if ( !mXMLDoc )
  {
    return restrictedLayerSet;
  }

  //names of unpublished layers / groups
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( !propertiesElem.isNull() )
  {
    QDomElement wmsLayerRestrictionElem = propertiesElem.firstChildElement( "WMSRestrictedLayers" );
    if ( !wmsLayerRestrictionElem.isNull() )
    {
      QStringList restrictedLayersAndGroups;
      QDomNodeList wmsLayerRestrictionValues = wmsLayerRestrictionElem.elementsByTagName( "value" );
      for ( int i = 0; i < wmsLayerRestrictionValues.size(); ++i )
      {
        restrictedLayerSet.insert( wmsLayerRestrictionValues.at( i ).toElement().text() );
      }
    }
  }

  //get legend dom element
  if ( restrictedLayerSet.size() < 1 || !mXMLDoc )
  {
    return restrictedLayerSet;
  }

  QDomElement legendElem = mXMLDoc->documentElement().firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return restrictedLayerSet;
  }

  //go through all legend groups and insert names of subgroups / sublayers if there is a match
  QDomNodeList legendGroupList = legendElem.elementsByTagName( "legendgroup" );
  for ( int i = 0; i < legendGroupList.size(); ++i )
  {
    //get name
    QDomElement groupElem = legendGroupList.at( i ).toElement();
    QString groupName = groupElem.attribute( "name" );
    if ( restrictedLayerSet.contains( groupName ) ) //match: add names of subgroups and sublayers to set
    {
      //embedded group? -> also get names of subgroups and sublayers from embedded projects
      if ( groupElem.attribute( "embedded" ) == "1" )
      {
        sublayersOfEmbeddedGroup( convertToAbsolutePath( groupElem.attribute( "project" ) ), groupName, restrictedLayerSet );
      }
      else //local group
      {
        QDomNodeList subgroupList = groupElem.elementsByTagName( "legendgroup" );
        for ( int j = 0; j < subgroupList.size(); ++j )
        {
          restrictedLayerSet.insert( subgroupList.at( j ).toElement().attribute( "name" ) );
        }
        QDomNodeList sublayerList = groupElem.elementsByTagName( "legendlayer" );
        for ( int k = 0; k < sublayerList.size(); ++k )
        {
          restrictedLayerSet.insert( sublayerList.at( k ).toElement().attribute( "name" ) );
        }
      }
    }
  }
  return restrictedLayerSet;
}

void QgsProjectParser::sublayersOfEmbeddedGroup( const QString& projectFilePath, const QString& groupName, QSet<QString>& layerSet )
{
  QFile projectFile( projectFilePath );
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument xmlDoc;
  if ( !xmlDoc.setContent( &projectFile ) )
  {
    return;
  }

  //go to legend node
  QDomElement legendElem = xmlDoc.documentElement().firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return;
  }

  //get group node list of embedded project
  QDomNodeList groupNodes = legendElem.elementsByTagName( "legendgroup" );
  QDomElement groupElem;
  for ( int i = 0; i < groupNodes.size(); ++i )
  {
    groupElem = groupNodes.at( i ).toElement();
    if ( groupElem.attribute( "name" ) == groupName )
    {
      //get all subgroups and sublayers and add to layerSet
      QDomElement subElem;
      QDomNodeList subGroupList = groupElem.elementsByTagName( "legendgroup" );
      for ( int j = 0; j < subGroupList.size(); ++j )
      {
        subElem = subGroupList.at( j ).toElement();
        layerSet.insert( subElem.attribute( "name" ) );
      }
      QDomNodeList subLayerList = groupElem.elementsByTagName( "legendlayer" );
      for ( int j = 0; j < subLayerList.size(); ++j )
      {
        subElem = subLayerList.at( j ).toElement();
        layerSet.insert( subElem.attribute( "name" ) );
      }
    }
  }
}

QgsRectangle QgsProjectParser::projectExtent() const
{
  QgsRectangle extent;
  if ( !mXMLDoc )
  {
    return extent;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  QDomElement mapCanvasElem = qgisElem.firstChildElement( "mapcanvas" );
  if ( mapCanvasElem.isNull() )
  {
    return extent;
  }

  QDomElement extentElem = mapCanvasElem.firstChildElement( "extent" );
  bool xminOk, xmaxOk, yminOk, ymaxOk;
  double xMin = extentElem.firstChildElement( "xmin" ).text().toDouble( &xminOk );
  double xMax = extentElem.firstChildElement( "xmax" ).text().toDouble( &xmaxOk );
  double yMin = extentElem.firstChildElement( "ymin" ).text().toDouble( &yminOk );
  double yMax = extentElem.firstChildElement( "ymax" ).text().toDouble( &ymaxOk );

  if ( xminOk && xmaxOk && yminOk && ymaxOk )
  {
    extent = QgsRectangle( xMin, yMin, xMax, yMax );
  }

  return extent;
}

void QgsProjectParser::drawOverlays( QPainter* p, int dpi, int width, int height ) const
{
  Q_UNUSED( width );
  Q_UNUSED( height );

  //consider DPI
  double scaleFactor = dpi / 88.0; //assume 88 as standard dpi
  QgsRectangle prjExtent = projectExtent();

  //text annotations
  QList< QPair< QTextDocument*, QDomElement > >::const_iterator textIt = mTextAnnotationItems.constBegin();
  for ( ; textIt != mTextAnnotationItems.constEnd(); ++textIt )
  {
    QDomElement annotationElem = textIt->second;
    if ( annotationElem.isNull() )
    {
      continue;
    }

    int itemWidth = annotationElem.attribute( "frameWidth", "0" ).toInt();
    int itemHeight = annotationElem.attribute( "frameHeight", "0" ).toInt();

    //calculate item position
    double xPos, yPos;
    if ( !annotationPosition( annotationElem, scaleFactor, xPos, yPos ) )
    {
      continue;
    }

    drawAnnotationRectangle( p, annotationElem, scaleFactor, xPos, yPos, itemWidth, itemHeight );

    //draw annotation contents
    p->translate( xPos, yPos );
    p->scale( scaleFactor, scaleFactor );
    textIt->first->drawContents( p, QRectF( 0, 0, itemWidth / scaleFactor, itemHeight / scaleFactor ) );
    p->restore();
  }

  //svg annotations
  QList< QPair< QSvgRenderer*, QDomElement > >::const_iterator svgIt = mSvgAnnotationElems.constBegin();
  QDomElement annotationElem;
  for ( ; svgIt != mSvgAnnotationElems.constEnd(); ++svgIt )
  {
    annotationElem = svgIt->second;
    int itemWidth = annotationElem.attribute( "frameWidth", "0" ).toInt() * scaleFactor;
    int itemHeight = annotationElem.attribute( "frameHeight", "0" ).toInt() * scaleFactor;

    //calculate item position
    double xPos, yPos;
    if ( !annotationPosition( annotationElem, scaleFactor, xPos, yPos ) )
    {
      continue;
    }

    drawAnnotationRectangle( p, annotationElem, scaleFactor, xPos, yPos, itemWidth, itemHeight );

    //keep width/height ratio of svg
    QRect viewBox = svgIt->first->viewBox();
    if ( viewBox.isValid() )
    {
      double widthRatio = ( double )( itemWidth ) / ( double )( viewBox.width() );
      double heightRatio = ( double )( itemHeight ) / ( double )( viewBox.height() );
      double renderWidth = 0;
      double renderHeight = 0;
      if ( widthRatio <= heightRatio )
      {
        renderWidth = itemWidth;
        renderHeight = viewBox.height() * itemWidth / viewBox.width() ;
      }
      else
      {
        renderHeight = itemHeight;
        renderWidth = viewBox.width() * itemHeight / viewBox.height() ;
      }

      svgIt->first->render( p, QRectF( xPos, yPos, renderWidth,
                                       renderHeight ) );
    }
  }
}

void QgsProjectParser::createTextAnnotationItems()
{
  cleanupTextAnnotationItems();

  if ( !mXMLDoc )
  {
    return;
  }

  //text annotations
  QDomElement qgisElem = mXMLDoc->documentElement();
  QDomNodeList textAnnotationList = qgisElem.elementsByTagName( "TextAnnotationItem" );
  QDomElement textAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < textAnnotationList.size(); ++i )
  {
    textAnnotationElem = textAnnotationList.at( i ).toElement();
    annotationElem = textAnnotationElem.firstChildElement( "AnnotationItem" );
    if ( !annotationElem.isNull() && annotationElem.attribute( "mapPositionFixed" ) != "1" )
    {
      QTextDocument* textDoc = new QTextDocument();
      textDoc->setHtml( textAnnotationElem.attribute( "document" ) );
      mTextAnnotationItems.push_back( qMakePair( textDoc, annotationElem ) );
    }
  }
}

void QgsProjectParser::createSvgAnnotationItems()
{
  mSvgAnnotationElems.clear();
  if ( !mXMLDoc )
  {
    return;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  QDomNodeList svgAnnotationList = qgisElem.elementsByTagName( "SVGAnnotationItem" );
  QDomElement svgAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < svgAnnotationList.size(); ++i )
  {
    svgAnnotationElem = svgAnnotationList.at( i ).toElement();
    annotationElem = svgAnnotationElem.firstChildElement( "AnnotationItem" );
    if ( !annotationElem.isNull() && annotationElem.attribute( "mapPositionFixed" ) != "1" )
    {
      QSvgRenderer* svg = new QSvgRenderer();
      if ( svg->load( convertToAbsolutePath( svgAnnotationElem.attribute( "file" ) ) ) )
      {
        mSvgAnnotationElems.push_back( qMakePair( svg, annotationElem ) );
      }
      else
      {
        delete svg;
      }
    }
  }
}

void QgsProjectParser::cleanupSvgAnnotationItems()
{
  QList< QPair< QSvgRenderer*, QDomElement > >::const_iterator it = mSvgAnnotationElems.constBegin();
  for ( ; it != mSvgAnnotationElems.constEnd(); ++it )
  {
    delete it->first;
  }
  mSvgAnnotationElems.clear();
}

void QgsProjectParser::cleanupTextAnnotationItems()
{
  QList< QPair< QTextDocument*, QDomElement > >::const_iterator it = mTextAnnotationItems.constBegin();
  for ( ; it != mTextAnnotationItems.constEnd(); ++it )
  {
    delete it->first;
  }
  mTextAnnotationItems.clear();
}

bool QgsProjectParser::annotationPosition( const QDomElement& elem, double scaleFactor,
    double& xPos, double& yPos )
{
  Q_UNUSED( scaleFactor );

  xPos = elem.attribute( "canvasPosX" ).toDouble() / scaleFactor;
  yPos = elem.attribute( "canvasPosY" ).toDouble() / scaleFactor;
  return true;
}

void QgsProjectParser::drawAnnotationRectangle( QPainter* p, const QDomElement& elem, double scaleFactor, double xPos, double yPos, int itemWidth, int itemHeight )
{
  Q_UNUSED( scaleFactor );
  if ( !p )
  {
    return;
  }

  QColor backgroundColor( elem.attribute( "frameBackgroundColor", "#000000" ) );
  backgroundColor.setAlpha( elem.attribute( "frameBackgroundColorAlpha", "255" ).toInt() );
  p->setBrush( QBrush( backgroundColor ) );
  QColor frameColor( elem.attribute( "frameColor", "#000000" ) );
  frameColor.setAlpha( elem.attribute( "frameColorAlpha", "255" ).toInt() );
  QPen framePen( frameColor );
  framePen.setWidth( elem.attribute( "frameBorderWidth", "1" ).toInt() );
  p->setPen( framePen );

  p->drawRect( QRectF( xPos, yPos, itemWidth, itemHeight ) );
}

void QgsProjectParser::addDrawingOrderEmbeddedGroup( const QDomElement& groupElem, QMap<int, QString>& orderedLayerList, bool useDrawingOrder ) const
{
  if ( groupElem.isNull() )
  {
    return;
  }

  QString project = convertToAbsolutePath( groupElem.attribute( "project" ) );
  if ( project.isEmpty() )
  {
    return;
  }

  int embedDrawingOrder = groupElem.attribute( "drawingOrder", "-1" ).toInt();
  QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
  if ( !p )
  {
    return;
  }

  QDomDocument* doc = p->mXMLDoc;
  if ( !doc )
  {
    return;
  }

  QDomNodeList layerNodeList = doc->elementsByTagName( "legendlayer" );
  QDomElement layerElem;
  QStringList layerNames;
  QString layerName;
  for ( int i = 0; i < layerNodeList.size(); ++i )
  {
    layerElem = layerNodeList.at( i ).toElement();
    layerName = layerElem.attribute( "name" );
    if ( useDrawingOrder )
    {
      layerNames.push_back( layerName );
    }
    else
    {
      orderedLayerList.insert( orderedLayerList.size(), layerName );
    }
  }

  if ( useDrawingOrder )
  {
    for ( int i = layerNames.size() - 1; i >= 0; --i )
    {
      if ( useDrawingOrder )
      {
        orderedLayerList.insertMulti( embedDrawingOrder, layerNames.at( i ) );
      }
    }
  }
}

void QgsProjectParser::loadLabelSettings( QgsLabelingEngineInterface* lbl )
{
  //pal labeling engine?
  QgsPalLabeling* pal = dynamic_cast<QgsPalLabeling*>( lbl );
  if ( pal )
  {
    QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
    if ( propertiesElem.isNull() )
    {
      return;
    }

    QDomElement palElem = propertiesElem.firstChildElement( "PAL" );
    if ( palElem.isNull() )
    {
      return;
    }

    //pal::Pal p;
    int candPoint = 8; //p.getPointP();
    int candLine = 8; //p.getLineP();
    int candPoly = 8; //p.getPolyP();

    //mCandPoint
    QDomElement candPointElem = palElem.firstChildElement( "CandidatesPoint" );
    if ( !candPointElem.isNull() )
    {
      candPoint = candPointElem.text().toInt();
    }

    //mCandLine
    QDomElement candLineElem = palElem.firstChildElement( "CandidatesLine" );
    if ( !candLineElem.isNull() )
    {
      candLine = candLineElem.text().toInt();
    }

    //mCandPolygon
    QDomElement candPolyElem = palElem.firstChildElement( "CandidatesPolygon" );
    if ( !candPolyElem.isNull() )
    {
      candPoly = candPolyElem.text().toInt();
    }

    pal->setNumCandidatePositions( candPoint, candLine, candPoly );

    //mShowingCandidates
    QDomElement showCandElem = palElem.firstChildElement( "ShowingCandidates" );
    if ( !showCandElem.isNull() )
    {
      pal->setShowingCandidates( showCandElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
    }

    //mShowingAllLabels
    QDomElement showAllLabelsElem = palElem.firstChildElement( "ShowingAllLabels" );
    if ( !showAllLabelsElem.isNull() )
    {
      pal->setShowingAllLabels( showAllLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
    }
  }
}
