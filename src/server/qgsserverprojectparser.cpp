/***************************************************************************
                              qgsserverprojectparser.cpp
                              --------------------------
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

#include "qgsserverprojectparser.h"
#include "qgsapplication.h"
#include "qgsconfigcache.h"
#include "qgsconfigparserutils.h"
#include "qgscrscache.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayerregistry.h"
#include "qgsmslayercache.h"
#include "qgsrasterlayer.h"
#include "qgseditorwidgetregistry.h"

#include <QDomDocument>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QUrl>

QgsServerProjectParser::QgsServerProjectParser( QDomDocument* xmlDoc, const QString& filePath )
    : mXMLDoc( xmlDoc )
    , mProjectPath( filePath )
    , mUseLayerIDs( false )
{
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

    mRestrictedLayers = findRestrictedLayers();
    mUseLayerIDs = findUseLayerIDs();
  }
}

QgsServerProjectParser::QgsServerProjectParser()
    : mXMLDoc( 0 )
    , mUseLayerIDs( false )
{
}

QgsServerProjectParser::~QgsServerProjectParser()
{
  delete mXMLDoc;
}

void QgsServerProjectParser::projectLayerMap( QMap<QString, QgsMapLayer*>& layerMap ) const
{
  layerMap.clear();

  QList<QDomElement>::const_iterator layerElemIt = mProjectLayerElements.constBegin();
  for ( ; layerElemIt != mProjectLayerElements.constEnd(); ++layerElemIt )
  {
    QgsMapLayer *layer = createLayerFromElement( *layerElemIt );
    if ( layer )
    {
      layerMap.insert( layer->id(), layer );
    }
  }
}

QString QgsServerProjectParser::convertToAbsolutePath( const QString& file ) const
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

  QStringList srcElems = srcPath.split( "/", QString::SkipEmptyParts );
  QStringList projElems = projPath.split( "/", QString::SkipEmptyParts );

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.prepend( "" );
    projElems.prepend( "" );
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

QgsMapLayer* QgsServerProjectParser::createLayerFromElement( const QDomElement& elem, bool useCache ) const
{
  if ( elem.isNull() || !mXMLDoc )
  {
    return 0;
  }

  addJoinLayersForElement( elem );
  addValueRelationLayersForElement( elem );
  addGetFeatureLayers( elem );

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

    QgsServerProjectParser* otherConfig = QgsConfigCache::instance()->serverConfiguration( project );
    if ( !otherConfig )
    {
      return 0;
    }
    return otherConfig->mapLayerFromLayerId( elem.attribute( "id" ), useCache );
  }

  if ( layer )
  {
    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      // see QgsEditorWidgetRegistry::mapLayerAdded()
      QObject::connect( layer, SIGNAL( readCustomSymbology( const QDomElement&, QString& ) ), QgsEditorWidgetRegistry::instance(), SLOT( readSymbology( const QDomElement&, QString& ) ) );
    }

    layer->readLayerXML( const_cast<QDomElement&>( elem ) ); //should be changed to const in QgsMapLayer
    layer->setLayerName( layerName( elem ) );
    if ( useCache )
    {
      QgsMSLayerCache::instance()->insertLayer( absoluteUri, id, layer, mProjectPath );
    }
    else
    {
      //todo: fixme
      //mLayersToRemove.push_back( layer );
    }
  }
  return layer;
}

QgsMapLayer* QgsServerProjectParser::mapLayerFromLayerId( const QString& lId, bool useCache ) const
{
  QHash< QString, QDomElement >::const_iterator layerIt = mProjectLayerElementsById.find( lId );
  if ( layerIt != mProjectLayerElementsById.constEnd() )
  {
    return createLayerFromElement( layerIt.value(), useCache );
  }
  return 0;
}

QString QgsServerProjectParser::layerIdFromLegendLayer( const QDomElement& legendLayer ) const
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

QString QgsServerProjectParser::layerId( const QDomElement& layerElem ) const
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

QgsRectangle QgsServerProjectParser::projectExtent() const
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

int QgsServerProjectParser::numberOfLayers() const
{
  return mProjectLayerElements.size();
}

bool QgsServerProjectParser::updateLegendDrawingOrder() const
{
  return legendElem().attribute( "updateDrawingOrder", "true" ).compare( "true", Qt::CaseInsensitive ) == 0;
}

void QgsServerProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& service, bool sia2045 ) const
{
  QDomElement propertiesElement = propertiesElem();
  if ( propertiesElement.isNull() )
  {
    QgsConfigParserUtils::fallbackServiceCapabilities( parentElement, doc );
    return;
  }
  QDomElement serviceElem = doc.createElement( "Service" );

  QDomElement serviceCapabilityElem = propertiesElement.firstChildElement( "WMSServiceCapabilities" );
  if ( serviceCapabilityElem.isNull() || serviceCapabilityElem.text().compare( "true", Qt::CaseInsensitive ) != 0 )
  {
    QgsConfigParserUtils::fallbackServiceCapabilities( parentElement, doc );
    return;
  }

  //Service name
  QDomElement wmsNameElem = doc.createElement( "Name" );
  QDomText wmsNameText = doc.createTextNode( service );
  wmsNameElem.appendChild( wmsNameText );
  serviceElem.appendChild( wmsNameElem );

  //WMS title
  QDomElement titleElem = propertiesElement.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    serviceElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElement.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    serviceElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElement.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    QDomElement wmsKeywordElem = doc.createElement( "KeywordList" );
    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      QDomElement keywordElem = doc.createElement( "Keyword" );
      QDomText keywordText = doc.createTextNode( keywordList.at( i ).toElement().text() );
      keywordElem.appendChild( keywordText );
      if ( sia2045 )
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
  QDomElement wmsOnlineResourceElem = propertiesElement.firstChildElement( "WMSOnlineResource" );
  QDomElement onlineResourceElem = doc.createElement( "OnlineResource" );
  onlineResourceElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  onlineResourceElem.setAttribute( "xlink:type", "simple" );
  if ( !wmsOnlineResourceElem.isNull() )
  {
    onlineResourceElem.setAttribute( "xlink:href", wmsOnlineResourceElem.text() );
  }

  serviceElem.appendChild( onlineResourceElem );

  if ( service.compare( "WMS", Qt::CaseInsensitive ) == 0 ) //no contact information in WFS 1.0 and WCS 1.0
  {
    //Contact information
    QDomElement contactInfoElem = doc.createElement( "ContactInformation" );

    //Contact person primary
    QDomElement contactPersonPrimaryElem = doc.createElement( "ContactPersonPrimary" );

    //Contact person
    QDomElement contactPersonElem = propertiesElement.firstChildElement( "WMSContactPerson" );
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
    QDomElement contactOrganizationElem = propertiesElement.firstChildElement( "WMSContactOrganization" );
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
    QDomElement phoneElem = propertiesElement.firstChildElement( "WMSContactPhone" );
    if ( !phoneElem.isNull() )
    {
      QDomElement wmsPhoneElem = doc.createElement( "ContactVoiceTelephone" );
      QDomText wmsPhoneText = doc.createTextNode( phoneElem.text() );
      wmsPhoneElem.appendChild( wmsPhoneText );
      contactInfoElem.appendChild( wmsPhoneElem );
    }

    //mail
    QDomElement mailElem = propertiesElement.firstChildElement( "WMSContactMail" );
    if ( !mailElem.isNull() )
    {
      QDomElement wmsMailElem = doc.createElement( "ContactElectronicMailAddress" );
      QDomText wmsMailText = doc.createTextNode( mailElem.text() );
      wmsMailElem.appendChild( wmsMailText );
      contactInfoElem.appendChild( wmsMailElem );
    }

    serviceElem.appendChild( contactInfoElem );
  }

  //Fees
  QDomElement feesElem = propertiesElement.firstChildElement( "WMSFees" );
  if ( !feesElem.isNull() )
  {
    QDomElement wmsFeesElem = doc.createElement( "Fees" );
    QDomText wmsFeesText = doc.createTextNode( feesElem.text() );
    wmsFeesElem.appendChild( wmsFeesText );
    serviceElem.appendChild( wmsFeesElem );
  }

  //AccessConstraints
  QDomElement accessConstraintsElem = propertiesElement.firstChildElement( "WMSAccessConstraints" );
  if ( !accessConstraintsElem.isNull() )
  {
    QDomElement wmsAccessConstraintsElem = doc.createElement( "AccessConstraints" );
    QDomText wmsAccessConstraintsText = doc.createTextNode( accessConstraintsElem.text() );
    wmsAccessConstraintsElem.appendChild( wmsAccessConstraintsText );
    serviceElem.appendChild( wmsAccessConstraintsElem );
  }

  //max width, max height for WMS
  if ( service.compare( "WMS", Qt::CaseInsensitive ) == 0 )
  {
    QString version = doc.documentElement().attribute( "version" );
    if ( version != "1.1.1" )
    {
      //max width
      QDomElement mwElem = propertiesElement.firstChildElement( "WMSMaxWidth" );
      if ( !mwElem.isNull() )
      {
        QDomElement maxWidthElem = doc.createElement( "MaxWidth" );
        QDomText maxWidthText = doc.createTextNode( mwElem.text() );
        maxWidthElem.appendChild( maxWidthText );
        serviceElem.appendChild( maxWidthElem );
      }
      //max height
      QDomElement mhElem = propertiesElement.firstChildElement( "WMSMaxHeight" );
      if ( !mhElem.isNull() )
      {
        QDomElement maxHeightElem = doc.createElement( "MaxHeight" );
        QDomText maxHeightText = doc.createTextNode( mhElem.text() );
        maxHeightElem.appendChild( maxHeightText );
        serviceElem.appendChild( maxHeightElem );
      }
    }
  }
  parentElement.appendChild( serviceElem );
}

QString QgsServerProjectParser::layerName( const QDomElement& layerElem ) const
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
  return nameElem.text().replace( ",", "%60" ); //commas are not allowed in layer names
}

QString QgsServerProjectParser::serviceUrl() const
{
  QString url;

  if ( !mXMLDoc )
  {
    return url;
  }

  QDomElement propertiesElement = propertiesElem();
  if ( !propertiesElement.isNull() )
  {
    QDomElement wmsUrlElem = propertiesElement.firstChildElement( "WMSUrl" );
    if ( !wmsUrlElem.isNull() )
    {
      url = wmsUrlElem.text();
    }
  }
  return url;
}

QString QgsServerProjectParser::wfsServiceUrl() const
{
  QString url;

  if ( !mXMLDoc )
  {
    return url;
  }

  QDomElement propertiesElement = propertiesElem();
  if ( !propertiesElement.isNull() )
  {
    QDomElement wfsUrlElem = propertiesElement.firstChildElement( "WFSUrl" );
    if ( !wfsUrlElem.isNull() )
    {
      url = wfsUrlElem.text();
    }
  }
  return url;
}

QString QgsServerProjectParser::wcsServiceUrl() const
{
  QString url;

  if ( !mXMLDoc )
  {
    return url;
  }

  QDomElement propertiesElement = propertiesElem();
  if ( !propertiesElement.isNull() )
  {
    QDomElement wcsUrlElem = propertiesElement.firstChildElement( "WCSUrl" );
    if ( !wcsUrlElem.isNull() )
    {
      url = wcsUrlElem.text();
    }
  }
  return url;
}

void QgsServerProjectParser::combineExtentAndCrsOfGroupChildren( QDomElement& groupElem, QDomDocument& doc, bool considerMapExtent ) const
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

  QgsConfigParserUtils::appendCRSElementsToLayer( groupElem, doc, combinedCRSSet.toList(), supportedOutputCrsList() );

  const QgsCoordinateReferenceSystem& groupCRS = projectCRS();
  if ( considerMapExtent )
  {
    QgsRectangle mapRect = mapRectangle();
    if ( !mapRect.isEmpty() )
    {
      combinedBBox = mapRect;
    }
  }
  QgsConfigParserUtils::appendLayerBoundingBoxes( groupElem, doc, combinedBBox, groupCRS );
}

void QgsServerProjectParser::addLayerProjectSettings( QDomElement& layerElem, QDomDocument& doc, QgsMapLayer* currentLayer ) const
{
  if ( !currentLayer )
  {
    return;
  }

  if ( currentLayer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vLayer = static_cast<QgsVectorLayer*>( currentLayer );
    const QSet<QString>& excludedAttributes = vLayer->excludeAttributesWMS();
    int displayFieldIdx = vLayer->fieldNameIndex( vLayer->displayField() );
    QString displayField = displayFieldIdx < 0 ? "maptip" : vLayer->displayField();

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
      // field alias in case of displayField
      if ( idx == displayFieldIdx )
      {
        displayField = vLayer->attributeDisplayName( idx );
      }
      QDomElement attributeElem = doc.createElement( "Attribute" );
      attributeElem.setAttribute( "name", field.name() );
      attributeElem.setAttribute( "type", QVariant::typeToName( field.type() ) );
      attributeElem.setAttribute( "typeName", field.typeName() );
      QString alias = vLayer->attributeAlias( idx );
      if ( !alias.isEmpty() )
      {
        attributeElem.setAttribute( "alias", alias );
      }

      //edit type to text
      attributeElem.setAttribute( "editType", vLayer->editorWidgetV2( idx ) );
      attributeElem.setAttribute( "comment", field.comment() );
      attributeElem.setAttribute( "length", field.length() );
      attributeElem.setAttribute( "precision", field.precision() );
      attributesElem.appendChild( attributeElem );
    }
    //displayfield
    layerElem.setAttribute( "displayField", displayField );
    layerElem.appendChild( attributesElem );
  }
}

QgsRectangle QgsServerProjectParser::layerBoundingBoxInProjectCRS( const QDomElement& layerElem, const QDomDocument &doc ) const
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

bool QgsServerProjectParser::crsSetForLayer( const QDomElement& layerElement, QSet<QString> &crsSet ) const
{
  if ( layerElement.isNull() )
  {
    return false;
  }

  crsSet.clear();

  QDomNodeList crsNodeList;
  crsNodeList = layerElement.elementsByTagName( "CRS" ); // WMS 1.3.0
  for ( int i = 0; i < crsNodeList.size(); ++i )
  {
    crsSet.insert( crsNodeList.at( i ).toElement().text() );
  }

  crsNodeList = layerElement.elementsByTagName( "SRS" ); // WMS 1.1.1
  for ( int i = 0; i < crsNodeList.size(); ++i )
  {
    crsSet.insert( crsNodeList.at( i ).toElement().text() );
  }

  return true;
}

const QgsCoordinateReferenceSystem& QgsServerProjectParser::projectCRS() const
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

QgsRectangle QgsServerProjectParser::mapRectangle() const
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

QStringList QgsServerProjectParser::supportedOutputCrsList() const
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
    if ( !wmsEpsgElem.isNull() )
    {
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
    else
    {
      //no CRS restriction defined in the project. Provide project CRS, wgs84 and pseudo mercator
      QString projectCrsId = projectCRS().authid();
      crsList.append( projectCrsId );
      if ( projectCrsId.compare( "EPSG:4326", Qt::CaseInsensitive ) != 0 )
      {
        crsList.append( QString( "EPSG:%1" ).arg( 4326 ) );
      }
      if ( projectCrsId.compare( "EPSG:3857", Qt::CaseInsensitive ) != 0 )
      {
        crsList.append( QString( "EPSG:%1" ).arg( 3857 ) );
      }
    }
  }

  return crsList;
}

QString QgsServerProjectParser::projectTitle() const
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

QDomElement QgsServerProjectParser::legendElem() const
{
  if ( !mXMLDoc )
  {
    return QDomElement();
  }
  return mXMLDoc->documentElement().firstChildElement( "legend" );
}

QDomElement QgsServerProjectParser::propertiesElem() const
{
  if ( !mXMLDoc )
  {
    return QDomElement();
  }

  return mXMLDoc->documentElement().firstChildElement( "properties" );
}

QSet<QString> QgsServerProjectParser::findRestrictedLayers() const
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

bool QgsServerProjectParser::findUseLayerIDs() const
{
  if ( !mXMLDoc )
    return false;

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
    return false;

  QDomElement wktElem = propertiesElem.firstChildElement( "WMSUseLayerIDs" );
  if ( wktElem.isNull() )
    return false;

  return wktElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
}

void QgsServerProjectParser::layerFromLegendLayer( const QDomElement& legendLayerElem, QMap< int, QgsMapLayer*>& layers, bool useCache ) const
{
  QString id = legendLayerElem.firstChild().firstChild().toElement().attribute( "layerid" );
  int drawingOrder = updateLegendDrawingOrder() ? -1 : legendLayerElem.attribute( "drawingOrder", "-1" ).toInt();

  QHash< QString, QDomElement >::const_iterator layerIt = mProjectLayerElementsById.find( id );
  if ( layerIt != mProjectLayerElementsById.constEnd() )
  {
    QgsMapLayer* layer = createLayerFromElement( layerIt.value(), useCache );
    if ( layer )
    {
      layers.insertMulti( drawingOrder, layer );
    }
  }
}

void QgsServerProjectParser::sublayersOfEmbeddedGroup( const QString& projectFilePath, const QString& groupName, QSet<QString>& layerSet )
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

QStringList QgsServerProjectParser::wfsLayerNames() const
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
        layerNameList.append( mUseLayerIDs ? currentLayer->id() : currentLayer->name() );
      }
    }
  }

  return layerNameList;
}

QStringList QgsServerProjectParser::wcsLayerNames() const
{
  QStringList layerNameList;

  QMap<QString, QgsMapLayer*> layerMap;
  projectLayerMap( layerMap );

  QgsMapLayer* currentLayer = 0;
  QStringList wcsIdList = wcsLayers();
  QStringList::const_iterator wcsIdIt = wcsIdList.constBegin();
  for ( ; wcsIdIt != wcsIdList.constEnd(); ++wcsIdIt )
  {
    QMap<QString, QgsMapLayer*>::const_iterator layerMapIt = layerMap.find( *wcsIdIt );
    if ( layerMapIt != layerMap.constEnd() )
    {
      currentLayer = layerMapIt.value();
      if ( currentLayer )
      {
        layerNameList.append( mUseLayerIDs ? currentLayer->id() : currentLayer->name() );
      }
    }
  }

  return layerNameList;
}

QDomElement QgsServerProjectParser::firstComposerLegendElement() const
{
  if ( !mXMLDoc )
  {
    return QDomElement();
  }

  QDomElement documentElem = mXMLDoc->documentElement();
  if ( documentElem.isNull() )
  {
    return QDomElement();
  }

  QDomElement composerElem = documentElem.firstChildElement( "Composer" );
  if ( composerElem.isNull() )
  {
    return QDomElement();
  }
  QDomElement compositionElem = composerElem.firstChildElement( "Composition" );
  if ( compositionElem.isNull() )
  {
    return QDomElement();
  }
  return compositionElem.firstChildElement( "ComposerLegend" );
}

QList<QDomElement> QgsServerProjectParser::publishedComposerElements() const
{
  QList<QDomElement> composerElemList;
  if ( !mXMLDoc )
  {
    return composerElemList;
  }

  QDomNodeList composerNodeList = mXMLDoc->elementsByTagName( "Composer" );

  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  QDomElement wmsRestrictedComposersElem = propertiesElem.firstChildElement( "WMSRestrictedComposers" );
  if ( wmsRestrictedComposersElem.isNull() )
  {
    for ( unsigned int i = 0; i < composerNodeList.length(); ++i )
    {
      composerElemList.push_back( composerNodeList.at( i ).toElement() );
    }
    return composerElemList;
  }

  QSet<QString> restrictedComposerNames;
  QDomNodeList valueList = wmsRestrictedComposersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    restrictedComposerNames.insert( valueList.at( i ).toElement().text() );
  }

  //remove unpublished composers from list
  QString currentComposerName;
  QDomElement currentElem;
  for ( int i = 0; i < composerNodeList.size(); ++i )
  {
    currentElem = composerNodeList.at( i ).toElement();
    currentComposerName = currentElem.attribute( "title" );
    if ( !restrictedComposerNames.contains( currentComposerName ) )
    {
      composerElemList.push_back( currentElem );
    }
  }

  return composerElemList;
}

QList< QPair< QString, QgsLayerCoordinateTransform > > QgsServerProjectParser::layerCoordinateTransforms() const
{
  QList< QPair< QString, QgsLayerCoordinateTransform > > layerTransformList;

  QDomElement coordTransformInfoElem = mXMLDoc->documentElement().firstChildElement( "mapcanvas" ).firstChildElement( "layer_coordinate_transform_info" );
  if ( coordTransformInfoElem.isNull() )
  {
    return layerTransformList;
  }

  QDomNodeList layerTransformNodeList = coordTransformInfoElem.elementsByTagName( "layer_coordinate_transform" );
  for ( int i = 0; i < layerTransformNodeList.size(); ++i )
  {
    QPair< QString, QgsLayerCoordinateTransform > layerEntry;
    QDomElement layerTransformElem = layerTransformNodeList.at( i ).toElement();
    layerEntry.first = layerTransformElem.attribute( "layerid" );
    QgsLayerCoordinateTransform t;
    t.srcAuthId = layerTransformElem.attribute( "srcAuthId" );
    t.destAuthId = layerTransformElem.attribute( "destAuthId" );
    t.srcDatumTransform = layerTransformElem.attribute( "srcDatumTransform", "-1" ).toInt();
    t.destDatumTransform = layerTransformElem.attribute( "destDatumTransform", "-1" ).toInt();
    layerEntry.second = t;
    layerTransformList.push_back( layerEntry );
  }
  return layerTransformList;
}

QStringList QgsServerProjectParser::wfsLayers() const
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

QStringList QgsServerProjectParser::wcsLayers() const
{
  QStringList wcsList;
  if ( !mXMLDoc )
  {
    return wcsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return wcsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return wcsList;
  }
  QDomElement wcsLayersElem = propertiesElem.firstChildElement( "WCSLayers" );
  if ( wcsLayersElem.isNull() )
  {
    return wcsList;
  }
  QDomNodeList valueList = wcsLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    wcsList << valueList.at( i ).toElement().text();
  }
  return wcsList;
}

void QgsServerProjectParser::addJoinLayersForElement( const QDomElement& layerElem ) const
{
  QDomElement vectorJoinsElem = layerElem.firstChildElement( "vectorjoins" );
  if ( vectorJoinsElem.isNull() )
  {
    return;
  }

  QDomNodeList joinNodeList = vectorJoinsElem.elementsByTagName( "join" );
  if ( joinNodeList.size() > 1 )
  {
    return;
  }

  for ( int i = 0; i < joinNodeList.size(); ++i )
  {
    QString id = joinNodeList.at( i ).toElement().attribute( "joinLayerId" );
    QgsMapLayer* layer = mapLayerFromLayerId( id );
    if ( layer )
    {
      QgsMapLayerRegistry::instance()->addMapLayer( layer, false, false );
    }
  }
}

void QgsServerProjectParser::addValueRelationLayersForElement( const QDomElement& layerElem ) const
{
  QDomElement editTypesElem = layerElem.firstChildElement( "edittypes" );
  if ( editTypesElem.isNull() )
  {
    return;
  }

  QDomNodeList editTypeNodeList = editTypesElem.elementsByTagName( "edittype" );
  for ( int i = 0; i < editTypeNodeList.size(); ++i )
  {
    QDomElement editTypeElem = editTypeNodeList.at( i ).toElement();
    int type = editTypeElem.attribute( "type" ).toInt();
    if ( type == QgsVectorLayer::ValueRelation )
    {
      QString layerId = editTypeElem.attribute( "layer" );
#if 0
      QString keyAttribute = editTypeEleml.attribute( "id" ); //relation attribute in other layer
      QString valueAttribute = editTypeElem.attribute( "value" ); //value attribute in other layer
      QString relationAttribute = editTypeElem.attribute( "name" );
#endif

      QgsMapLayer* layer = mapLayerFromLayerId( layerId );
      if ( layer )
      {
        QgsMapLayerRegistry::instance()->addMapLayer( layer, false, false );
      }
    }
  }
}

void QgsServerProjectParser::addGetFeatureLayers( const QDomElement& layerElem ) const
{
  QString str;
  QTextStream stream( &str );
  layerElem.save( stream, 2 );

  QRegExp rx( "getFeature\\('([^']*)'" );
  int idx = 0;
  while (( idx = rx.indexIn( str, idx ) ) != -1 )
  {
    QString name = rx.cap( 1 );
    QgsMapLayer* ml = 0;
    QHash< QString, QDomElement >::const_iterator layerElemIt = mProjectLayerElementsById.find( name );
    if ( layerElemIt != mProjectLayerElementsById.constEnd() )
    {
      ml = createLayerFromElement( layerElemIt.value() );
    }
    else
    {
      layerElemIt = mProjectLayerElementsByName.find( name );
      if ( layerElemIt != mProjectLayerElementsByName.constEnd() )
      {
        ml = createLayerFromElement( layerElemIt.value() );
      }
    }

    if ( ml )
    {
      QgsMapLayerRegistry::instance()->addMapLayer( ml, false, false );
    }
    idx += rx.matchedLength();
  }
}


