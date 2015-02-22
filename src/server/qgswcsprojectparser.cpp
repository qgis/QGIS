/***************************************************************************
                              qgswcsprojectparser.cpp
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

#include "qgswcsprojectparser.h"
#include "qgsconfigcache.h"
#include "qgsconfigparserutils.h"
#include "qgsconfigcache.h"
#include "qgsrasterlayer.h"

QgsWCSProjectParser::QgsWCSProjectParser( const QString& filePath )
{
  mProjectParser = QgsConfigCache::instance()->serverConfiguration( filePath );
}

QgsWCSProjectParser::~QgsWCSProjectParser()
{
  delete mProjectParser;
}

void QgsWCSProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  mProjectParser->serviceCapabilities( parentElement, doc, "WCS" );
}

QString QgsWCSProjectParser::wcsServiceUrl() const
{
  QString url;

  if ( !mProjectParser->xmlDocument() )
  {
    return url;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement wcsUrlElem = propertiesElem.firstChildElement( "WCSUrl" );
    if ( !wcsUrlElem.isNull() )
    {
      url = wcsUrlElem.text();
    }
  }
  return url;
}

QString QgsWCSProjectParser::serviceUrl() const
{
  return mProjectParser->serviceUrl();
}

void QgsWCSProjectParser::wcsContentMetadata( QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wcsLayersId = wcsLayers();

  QMap<QString, QgsMapLayer *> layerMap;

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "raster" )
    {
      QgsMapLayer *layer = mProjectParser->createLayerFromElement( elem );
      if ( layer && wcsLayersId.contains( layer->id() ) )
      {
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
        layerMap.insert( layer->id(), layer );

        QDomElement layerElem = doc.createElement( "CoverageOfferingBrief" );
        QDomElement nameElem = doc.createElement( "name" );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = layer->name();
        typeName = typeName.replace( " ", "_" );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement labelElem = doc.createElement( "label" );
        QString titleName = layer->title();
        if ( titleName.isEmpty() )
        {
          titleName = layer->name();
        }
        QDomText labelText = doc.createTextNode( titleName );
        labelElem.appendChild( labelText );
        layerElem.appendChild( labelElem );

        QDomElement descriptionElem = doc.createElement( "description" );
        QString abstractName = layer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = "";
        }
        QDomText descriptionText = doc.createTextNode( abstractName );
        descriptionElem.appendChild( descriptionText );
        layerElem.appendChild( descriptionElem );

        //lonLatEnvelope
        const QgsCoordinateReferenceSystem& layerCrs = layer->crs();
        QgsCoordinateTransform t( layerCrs, QgsCoordinateReferenceSystem( 4326 ) );
        //transform
        QgsRectangle BBox;
        try
        {
          BBox = t.transformBoundingBox( layer->extent() );
        }
        catch ( QgsCsException &e )
        {
          QgsDebugMsg( QString( "Transform error caught: %1. Using original layer extent." ).arg( e.what() ) );
          BBox = layer->extent();
        }
        QDomElement lonLatElem = doc.createElement( "lonLatEnvelope" );
        lonLatElem.setAttribute( "srsName", "urn:ogc:def:crs:OGC:1.3:CRS84" );
        QDomElement lowerPosElem = doc.createElement( "gml:pos" );
        QDomText lowerPosText = doc.createTextNode( QString::number( BBox.xMinimum() ) + " " +  QString::number( BBox.yMinimum() ) );
        lowerPosElem.appendChild( lowerPosText );
        lonLatElem.appendChild( lowerPosElem );
        QDomElement upperPosElem = doc.createElement( "gml:pos" );
        QDomText upperPosText = doc.createTextNode( QString::number( BBox.xMaximum() ) + " " +  QString::number( BBox.yMaximum() ) );
        upperPosElem.appendChild( upperPosText );
        lonLatElem.appendChild( upperPosElem );
        layerElem.appendChild( lonLatElem );

        parentElement.appendChild( layerElem );
      }
    }
  }
}

QStringList QgsWCSProjectParser::wcsLayers() const
{
  QStringList wcsList;
  if ( !mProjectParser->xmlDocument() )
  {
    return wcsList;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
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

void QgsWCSProjectParser::describeCoverage( const QString& aCoveName, QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wcsLayersId = wcsLayers();
  QStringList coveNameList;
  if ( aCoveName != "" )
  {
    QStringList coveNameSplit = aCoveName.split( "," );
    foreach ( const QString &str, coveNameSplit )
    {
      coveNameList << str;
    }
  }

  QMap<QString, QgsMapLayer *> layerMap;

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "raster" )
    {
      QgsRasterLayer *rLayer = dynamic_cast<QgsRasterLayer *>( mProjectParser->createLayerFromElement( elem ) );
      if ( !rLayer )
        continue;
      QString coveName = rLayer->name();
      coveName = coveName.replace( " ", "_" );
      if ( wcsLayersId.contains( rLayer->id() ) && ( aCoveName == "" || coveNameList.contains( coveName ) ) )
      {
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( rLayer->id() ) );
        layerMap.insert( rLayer->id(), rLayer );

        QDomElement layerElem = doc.createElement( "CoverageOffering" );
        QDomElement nameElem = doc.createElement( "name" );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = rLayer->name();
        typeName = typeName.replace( " ", "_" );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement labelElem = doc.createElement( "label" );
        QString titleName = rLayer->title();
        if ( titleName.isEmpty() )
        {
          titleName = rLayer->name();
        }
        QDomText labelText = doc.createTextNode( titleName );
        labelElem.appendChild( labelText );
        layerElem.appendChild( labelElem );

        QDomElement descriptionElem = doc.createElement( "description" );
        QString abstractName = rLayer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = "";
        }
        QDomText descriptionText = doc.createTextNode( abstractName );
        descriptionElem.appendChild( descriptionText );
        layerElem.appendChild( descriptionElem );

        //lonLatEnvelope
        const QgsCoordinateReferenceSystem& layerCrs = rLayer->crs();
        QgsCoordinateTransform t( layerCrs, QgsCoordinateReferenceSystem( 4326 ) );
        //transform
        QgsRectangle BBox = rLayer->extent();
        try
        {
          QgsRectangle transformedBox = t.transformBoundingBox( BBox );
          BBox = transformedBox;
        }
        catch ( QgsCsException &e )
        {
          QgsDebugMsg( QString( "Transform error caught: %1" ).arg( e.what() ) );
        }

        QDomElement lonLatElem = doc.createElement( "lonLatEnvelope" );
        lonLatElem.setAttribute( "srsName", "urn:ogc:def:crs:OGC:1.3:CRS84" );
        QDomElement lowerPosElem = doc.createElement( "gml:pos" );
        QDomText lowerPosText = doc.createTextNode( QString::number( BBox.xMinimum() ) + " " +  QString::number( BBox.yMinimum() ) );
        lowerPosElem.appendChild( lowerPosText );
        lonLatElem.appendChild( lowerPosElem );
        QDomElement upperPosElem = doc.createElement( "gml:pos" );
        QDomText upperPosText = doc.createTextNode( QString::number( BBox.xMaximum() ) + " " +  QString::number( BBox.yMaximum() ) );
        upperPosElem.appendChild( upperPosText );
        lonLatElem.appendChild( upperPosElem );
        layerElem.appendChild( lonLatElem );

        QDomElement domainSetElem = doc.createElement( "domainSet" );
        layerElem.appendChild( domainSetElem );

        QDomElement spatialDomainElem = doc.createElement( "spatialDomain" );
        domainSetElem.appendChild( spatialDomainElem );

        QgsRectangle layerBBox = rLayer->extent();
        QDomElement envelopeElem = doc.createElement( "gml:Envelope" );
        envelopeElem.setAttribute( "srsName", layerCrs.authid() );
        QDomElement lowerCornerElem = doc.createElement( "gml:pos" );
        QDomText lowerCornerText = doc.createTextNode( QString::number( layerBBox.xMinimum() ) + " " +  QString::number( layerBBox.yMinimum() ) );
        lowerCornerElem.appendChild( lowerCornerText );
        envelopeElem.appendChild( lowerCornerElem );
        QDomElement upperCornerElem = doc.createElement( "gml:pos" );
        QDomText upperCornerText = doc.createTextNode( QString::number( layerBBox.xMaximum() ) + " " +  QString::number( layerBBox.yMaximum() ) );
        upperCornerElem.appendChild( upperCornerText );
        envelopeElem.appendChild( upperCornerElem );
        spatialDomainElem.appendChild( envelopeElem );

        QDomElement rectGridElem = doc.createElement( "gml:RectifiedGrid" );
        rectGridElem.setAttribute( "dimension", 2 );
        QDomElement limitsElem = doc.createElement( "gml:limits" );
        rectGridElem.appendChild( limitsElem );
        QDomElement gridEnvElem = doc.createElement( "gml:GridEnvelope" );
        limitsElem.appendChild( gridEnvElem );
        QDomElement lowElem = doc.createElement( "gml:low" );
        QDomText lowText = doc.createTextNode( "0 0" );
        lowElem.appendChild( lowText );
        gridEnvElem.appendChild( lowElem );
        QDomElement highElem = doc.createElement( "gml:high" );
        QDomText highText = doc.createTextNode( QString::number( rLayer->width() ) + " " + QString::number( rLayer->height() ) );
        highElem.appendChild( highText );
        gridEnvElem.appendChild( highElem );
        spatialDomainElem.appendChild( rectGridElem );

        QDomElement xAxisElem = doc.createElement( "gml:axisName" );
        QDomText xAxisText = doc.createTextNode( "x" );
        xAxisElem.appendChild( xAxisText );
        spatialDomainElem.appendChild( xAxisElem );

        QDomElement yAxisElem = doc.createElement( "gml:axisName" );
        QDomText yAxisText = doc.createTextNode( "y" );
        yAxisElem.appendChild( yAxisText );
        spatialDomainElem.appendChild( yAxisElem );

        QDomElement originElem = doc.createElement( "gml:origin" );
        QDomElement originPosElem = doc.createElement( "gml:pos" );
        QDomText originPosText = doc.createTextNode( QString::number( layerBBox.xMinimum() ) + " " +  QString::number( layerBBox.yMaximum() ) );
        originPosElem.appendChild( originPosText );
        spatialDomainElem.appendChild( originElem );

        QDomElement xOffsetElem = doc.createElement( "gml:offsetVector" );
        QDomText xOffsetText = doc.createTextNode( QString::number( rLayer->rasterUnitsPerPixelX() ) + " 0" );
        xOffsetElem.appendChild( xOffsetText );
        spatialDomainElem.appendChild( xOffsetElem );

        QDomElement yOffsetElem = doc.createElement( "gml:offsetVector" );
        QDomText yOffsetText = doc.createTextNode( "0 " + QString::number( rLayer->rasterUnitsPerPixelY() ) );
        yOffsetElem.appendChild( yOffsetText );
        spatialDomainElem.appendChild( yOffsetElem );

        QDomElement rangeSetElem = doc.createElement( "rangeSet" );
        layerElem.appendChild( rangeSetElem );

        QDomElement RangeSetElem = doc.createElement( "RangeSet" );
        rangeSetElem.appendChild( RangeSetElem );

        QDomElement rsNameElem = doc.createElement( "name" );
        QDomText rsNameText = doc.createTextNode( "Bands" );
        rsNameElem.appendChild( rsNameText );
        RangeSetElem.appendChild( rsNameElem );

        QDomElement axisDescElem = doc.createElement( "axisDescription" );
        RangeSetElem.appendChild( axisDescElem );

        QDomElement AxisDescElem = doc.createElement( "AxisDescription" );
        axisDescElem.appendChild( AxisDescElem );

        QDomElement adNameElem = doc.createElement( "name" );
        QDomText adNameText = doc.createTextNode( "bands" );
        adNameElem.appendChild( adNameText );
        AxisDescElem.appendChild( adNameElem );

        QDomElement adValuesElem = doc.createElement( "values" );
        for ( int idx = 0; idx < rLayer->bandCount(); ++idx )
        {
          QDomElement adValueElem = doc.createElement( "value" );
          QDomText adValueText = doc.createTextNode( QString::number( idx + 1 ) );
          adValueElem.appendChild( adValueText );
          adValuesElem.appendChild( adValueElem );
        }
        AxisDescElem.appendChild( adValuesElem );

        QDomElement sCRSElem = doc.createElement( "supportedCRSs" );
        QDomElement rCRSElem = doc.createElement( "requestResponseCRSs" );
        QDomText rCRSText = doc.createTextNode( layerCrs.authid() );
        rCRSElem.appendChild( rCRSText );
        sCRSElem.appendChild( rCRSElem );
        QDomElement nCRSElem = doc.createElement( "nativeCRSs" );
        QDomText nCRSText = doc.createTextNode( layerCrs.authid() );
        nCRSElem.appendChild( nCRSText );
        sCRSElem.appendChild( nCRSElem );
        layerElem.appendChild( sCRSElem );

        QDomElement sFormatsElem = doc.createElement( "supportedFormats" );
        sFormatsElem.setAttribute( "nativeFormat", "raw binary" );
        QDomElement formatsElem = doc.createElement( "formats" );
        QDomText formatsText = doc.createTextNode( "GeoTIFF" );
        formatsElem.appendChild( formatsText );
        sFormatsElem.appendChild( formatsElem );
        layerElem.appendChild( sFormatsElem );

        parentElement.appendChild( layerElem );
      }
    }
  }
}

QList<QgsMapLayer*> QgsWCSProjectParser::mapLayerFromCoverage( const QString& cName, bool useCache ) const
{
  QList<QgsMapLayer*> layerList;

  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return layerList;
  }

  QStringList wcsLayersId = wcsLayers();

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "raster" )
    {
      QgsMapLayer *mLayer = mProjectParser->createLayerFromElement( elem, useCache );
      QgsRasterLayer* layer = dynamic_cast<QgsRasterLayer*>( mLayer );
      if ( !layer || !wcsLayersId.contains( layer->id() ) )
        return layerList;

      QString coveName = layer->name();
      coveName = coveName.replace( " ", "_" );
      if ( cName == coveName )
      {
        layerList.push_back( mLayer );
        return layerList;
      }
    }
  }
  return layerList;
}
