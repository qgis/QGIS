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
#include "qgscsexception.h"
#include "qgsrasterlayer.h"
#include "qgsmapserviceexception.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrol.h"
#endif


QgsWCSProjectParser::QgsWCSProjectParser(
  const QString& filePath
  , const QgsAccessControl* as
)
    : mAccessControl( as )
{
#ifndef HAVE_SERVER_PYTHON_PLUGINS
  Q_UNUSED( mAccessControl );
#endif
  mProjectParser = QgsConfigCache::instance()->serverConfiguration( filePath );
}

QgsWCSProjectParser::~QgsWCSProjectParser()
{
  delete mProjectParser;
}

void QgsWCSProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  mProjectParser->serviceCapabilities( parentElement, doc, QStringLiteral( "WCS" ) );
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
    QDomElement wcsUrlElem = propertiesElem.firstChildElement( QStringLiteral( "WCSUrl" ) );
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

  Q_FOREACH ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "raster" ) )
    {
      QgsMapLayer *layer = mProjectParser->createLayerFromElement( elem );
      if ( layer && wcsLayersId.contains( layer->id() ) )
      {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !mAccessControl->layerReadPermission( layer ) )
        {
          continue;
        }
#endif
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
        layerMap.insert( layer->id(), layer );

        QDomElement layerElem = doc.createElement( QStringLiteral( "CoverageOfferingBrief" ) );
        QDomElement nameElem = doc.createElement( QStringLiteral( "name" ) );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = layer->name();
        if ( !layer->shortName().isEmpty() )
          typeName = layer->shortName();
        typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement labelElem = doc.createElement( QStringLiteral( "label" ) );
        QString titleName = layer->title();
        if ( titleName.isEmpty() )
        {
          titleName = layer->name();
        }
        QDomText labelText = doc.createTextNode( titleName );
        labelElem.appendChild( labelText );
        layerElem.appendChild( labelElem );

        QDomElement descriptionElem = doc.createElement( QStringLiteral( "description" ) );
        QString abstractName = layer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = QLatin1String( "" );
        }
        QDomText descriptionText = doc.createTextNode( abstractName );
        descriptionElem.appendChild( descriptionText );
        layerElem.appendChild( descriptionElem );

        //lonLatEnvelope
        QgsCoordinateReferenceSystem layerCrs = layer->crs();
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
        QDomElement lonLatElem = doc.createElement( QStringLiteral( "lonLatEnvelope" ) );
        lonLatElem.setAttribute( QStringLiteral( "srsName" ), QStringLiteral( "urn:ogc:def:crs:OGC:1.3:CRS84" ) );
        QDomElement lowerPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText lowerPosText = doc.createTextNode( QString::number( BBox.xMinimum() ) + " " +  QString::number( BBox.yMinimum() ) );
        lowerPosElem.appendChild( lowerPosText );
        lonLatElem.appendChild( lowerPosElem );
        QDomElement upperPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
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
  QDomElement wcsLayersElem = propertiesElem.firstChildElement( QStringLiteral( "WCSLayers" ) );
  if ( wcsLayersElem.isNull() )
  {
    return wcsList;
  }
  QDomNodeList valueList = wcsLayersElem.elementsByTagName( QStringLiteral( "value" ) );
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
  if ( aCoveName != QLatin1String( "" ) )
  {
    QStringList coveNameSplit = aCoveName.split( QStringLiteral( "," ) );
    Q_FOREACH ( const QString &str, coveNameSplit )
    {
      coveNameList << str;
    }
  }

  QMap<QString, QgsMapLayer *> layerMap;

  Q_FOREACH ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "raster" ) )
    {
      QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( mProjectParser->createLayerFromElement( elem ) );
      if ( !rLayer )
        continue;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( rLayer ) )
      {
        continue;
      }
#endif

      QString coveName = rLayer->name();
      if ( !rLayer->shortName().isEmpty() )
        coveName = rLayer->shortName();
      coveName = coveName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      if ( wcsLayersId.contains( rLayer->id() ) && ( aCoveName == QLatin1String( "" ) || coveNameList.contains( coveName ) ) )
      {
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( rLayer->id() ) );
        layerMap.insert( rLayer->id(), rLayer );

        QDomElement layerElem = doc.createElement( QStringLiteral( "CoverageOffering" ) );
        QDomElement nameElem = doc.createElement( QStringLiteral( "name" ) );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = rLayer->name();
        if ( !rLayer->shortName().isEmpty() )
          typeName = rLayer->shortName();
        typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement labelElem = doc.createElement( QStringLiteral( "label" ) );
        QString titleName = rLayer->title();
        if ( titleName.isEmpty() )
        {
          titleName = rLayer->name();
        }
        QDomText labelText = doc.createTextNode( titleName );
        labelElem.appendChild( labelText );
        layerElem.appendChild( labelElem );

        QDomElement descriptionElem = doc.createElement( QStringLiteral( "description" ) );
        QString abstractName = rLayer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = QLatin1String( "" );
        }
        QDomText descriptionText = doc.createTextNode( abstractName );
        descriptionElem.appendChild( descriptionText );
        layerElem.appendChild( descriptionElem );

        //lonLatEnvelope
        QgsCoordinateReferenceSystem layerCrs = rLayer->crs();
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

        QDomElement lonLatElem = doc.createElement( QStringLiteral( "lonLatEnvelope" ) );
        lonLatElem.setAttribute( QStringLiteral( "srsName" ), QStringLiteral( "urn:ogc:def:crs:OGC:1.3:CRS84" ) );
        QDomElement lowerPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText lowerPosText = doc.createTextNode( QString::number( BBox.xMinimum() ) + " " +  QString::number( BBox.yMinimum() ) );
        lowerPosElem.appendChild( lowerPosText );
        lonLatElem.appendChild( lowerPosElem );
        QDomElement upperPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText upperPosText = doc.createTextNode( QString::number( BBox.xMaximum() ) + " " +  QString::number( BBox.yMaximum() ) );
        upperPosElem.appendChild( upperPosText );
        lonLatElem.appendChild( upperPosElem );
        layerElem.appendChild( lonLatElem );

        QDomElement domainSetElem = doc.createElement( QStringLiteral( "domainSet" ) );
        layerElem.appendChild( domainSetElem );

        QDomElement spatialDomainElem = doc.createElement( QStringLiteral( "spatialDomain" ) );
        domainSetElem.appendChild( spatialDomainElem );

        QgsRectangle layerBBox = rLayer->extent();
        QDomElement envelopeElem = doc.createElement( QStringLiteral( "gml:Envelope" ) );
        envelopeElem.setAttribute( QStringLiteral( "srsName" ), layerCrs.authid() );
        QDomElement lowerCornerElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText lowerCornerText = doc.createTextNode( QString::number( layerBBox.xMinimum() ) + " " +  QString::number( layerBBox.yMinimum() ) );
        lowerCornerElem.appendChild( lowerCornerText );
        envelopeElem.appendChild( lowerCornerElem );
        QDomElement upperCornerElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText upperCornerText = doc.createTextNode( QString::number( layerBBox.xMaximum() ) + " " +  QString::number( layerBBox.yMaximum() ) );
        upperCornerElem.appendChild( upperCornerText );
        envelopeElem.appendChild( upperCornerElem );
        spatialDomainElem.appendChild( envelopeElem );

        QDomElement rectGridElem = doc.createElement( QStringLiteral( "gml:RectifiedGrid" ) );
        rectGridElem.setAttribute( QStringLiteral( "dimension" ), 2 );
        QDomElement limitsElem = doc.createElement( QStringLiteral( "gml:limits" ) );
        rectGridElem.appendChild( limitsElem );
        QDomElement gridEnvElem = doc.createElement( QStringLiteral( "gml:GridEnvelope" ) );
        limitsElem.appendChild( gridEnvElem );
        QDomElement lowElem = doc.createElement( QStringLiteral( "gml:low" ) );
        QDomText lowText = doc.createTextNode( QStringLiteral( "0 0" ) );
        lowElem.appendChild( lowText );
        gridEnvElem.appendChild( lowElem );
        QDomElement highElem = doc.createElement( QStringLiteral( "gml:high" ) );
        QDomText highText = doc.createTextNode( QString::number( rLayer->width() ) + " " + QString::number( rLayer->height() ) );
        highElem.appendChild( highText );
        gridEnvElem.appendChild( highElem );
        spatialDomainElem.appendChild( rectGridElem );

        QDomElement xAxisElem = doc.createElement( QStringLiteral( "gml:axisName" ) );
        QDomText xAxisText = doc.createTextNode( QStringLiteral( "x" ) );
        xAxisElem.appendChild( xAxisText );
        spatialDomainElem.appendChild( xAxisElem );

        QDomElement yAxisElem = doc.createElement( QStringLiteral( "gml:axisName" ) );
        QDomText yAxisText = doc.createTextNode( QStringLiteral( "y" ) );
        yAxisElem.appendChild( yAxisText );
        spatialDomainElem.appendChild( yAxisElem );

        QDomElement originElem = doc.createElement( QStringLiteral( "gml:origin" ) );
        QDomElement originPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        QDomText originPosText = doc.createTextNode( QString::number( layerBBox.xMinimum() ) + " " +  QString::number( layerBBox.yMaximum() ) );
        originPosElem.appendChild( originPosText );
        spatialDomainElem.appendChild( originElem );

        QDomElement xOffsetElem = doc.createElement( QStringLiteral( "gml:offsetVector" ) );
        QDomText xOffsetText = doc.createTextNode( QString::number( rLayer->rasterUnitsPerPixelX() ) + " 0" );
        xOffsetElem.appendChild( xOffsetText );
        spatialDomainElem.appendChild( xOffsetElem );

        QDomElement yOffsetElem = doc.createElement( QStringLiteral( "gml:offsetVector" ) );
        QDomText yOffsetText = doc.createTextNode( "0 " + QString::number( rLayer->rasterUnitsPerPixelY() ) );
        yOffsetElem.appendChild( yOffsetText );
        spatialDomainElem.appendChild( yOffsetElem );

        QDomElement rangeSetElem = doc.createElement( QStringLiteral( "rangeSet" ) );
        layerElem.appendChild( rangeSetElem );

        QDomElement RangeSetElem = doc.createElement( QStringLiteral( "RangeSet" ) );
        rangeSetElem.appendChild( RangeSetElem );

        QDomElement rsNameElem = doc.createElement( QStringLiteral( "name" ) );
        QDomText rsNameText = doc.createTextNode( QStringLiteral( "Bands" ) );
        rsNameElem.appendChild( rsNameText );
        RangeSetElem.appendChild( rsNameElem );

        QDomElement axisDescElem = doc.createElement( QStringLiteral( "axisDescription" ) );
        RangeSetElem.appendChild( axisDescElem );

        QDomElement AxisDescElem = doc.createElement( QStringLiteral( "AxisDescription" ) );
        axisDescElem.appendChild( AxisDescElem );

        QDomElement adNameElem = doc.createElement( QStringLiteral( "name" ) );
        QDomText adNameText = doc.createTextNode( QStringLiteral( "bands" ) );
        adNameElem.appendChild( adNameText );
        AxisDescElem.appendChild( adNameElem );

        QDomElement adValuesElem = doc.createElement( QStringLiteral( "values" ) );
        for ( int idx = 0; idx < rLayer->bandCount(); ++idx )
        {
          QDomElement adValueElem = doc.createElement( QStringLiteral( "value" ) );
          QDomText adValueText = doc.createTextNode( QString::number( idx + 1 ) );
          adValueElem.appendChild( adValueText );
          adValuesElem.appendChild( adValueElem );
        }
        AxisDescElem.appendChild( adValuesElem );

        QDomElement sCRSElem = doc.createElement( QStringLiteral( "supportedCRSs" ) );
        QDomElement rCRSElem = doc.createElement( QStringLiteral( "requestResponseCRSs" ) );
        QDomText rCRSText = doc.createTextNode( layerCrs.authid() );
        rCRSElem.appendChild( rCRSText );
        sCRSElem.appendChild( rCRSElem );
        QDomElement nCRSElem = doc.createElement( QStringLiteral( "nativeCRSs" ) );
        QDomText nCRSText = doc.createTextNode( layerCrs.authid() );
        nCRSElem.appendChild( nCRSText );
        sCRSElem.appendChild( nCRSElem );
        layerElem.appendChild( sCRSElem );

        QDomElement sFormatsElem = doc.createElement( QStringLiteral( "supportedFormats" ) );
        sFormatsElem.setAttribute( QStringLiteral( "nativeFormat" ), QStringLiteral( "raw binary" ) );
        QDomElement formatsElem = doc.createElement( QStringLiteral( "formats" ) );
        QDomText formatsText = doc.createTextNode( QStringLiteral( "GeoTIFF" ) );
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

  Q_FOREACH ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "raster" ) )
    {
      QgsMapLayer *mLayer = mProjectParser->createLayerFromElement( elem, useCache );
      QgsRasterLayer* layer = qobject_cast<QgsRasterLayer*>( mLayer );
      if ( !layer || !wcsLayersId.contains( layer->id() ) )
        return layerList;

      QString coveName = layer->name();
      if ( !layer->shortName().isEmpty() )
        coveName = layer->shortName();
      coveName = coveName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      if ( cName == coveName )
      {
        layerList.push_back( mLayer );
        return layerList;
      }
    }
  }
  return layerList;
}
