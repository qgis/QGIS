/***************************************************************************
                              qgsconfigparser.cpp
                              -------------------
  begin                : May 26, 2010
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

#include "qgsconfigparser.h"
#include "qgscrscache.h"
#include "qgsapplication.h"
#include "qgscomposerlabel.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <sqlite3.h>
#include <QFile>


QgsConfigParser::QgsConfigParser()
    : mFallbackParser( 0 )
    , mScaleDenominator( 0 )
    , mOutputUnits( QgsMapRenderer::Millimeters )
{
  setDefaultLegendSettings();
  mSelectionColor = QColor( 255, 255, 0 ); //yellow opaque is default selection color
}

QgsConfigParser::~QgsConfigParser()
{
  //remove the external GML datasets
  foreach( QDomDocument *doc, mExternalGMLDatasets.values() )
  {
    delete doc;
  }

  //remove the temporary files
  foreach( QTemporaryFile *file, mFilesToRemove )
  {
    delete file;
  }

  //and also those the temporary file paths
  foreach( QString path, mFilePathsToRemove )
  {
    QFile::remove( path );
  }

  //delete the layers in the list
  foreach( QgsMapLayer *layer, mLayersToRemove )
  {
    delete layer;
  }
}

void QgsConfigParser::setDefaultLegendSettings()
{
  mLegendBoxSpace = 2;
  mLegendLayerSpace = 3;
  mLegendSymbolSpace = 2;
  mLegendIconLabelSpace = 2;
  mLegendSymbolWidth = 7;
  mLegendSymbolHeight = 4;
}

void QgsConfigParser::setFallbackParser( QgsConfigParser* p )
{
  if ( !p )
  {
    return;
  }
  delete mFallbackParser;
  mFallbackParser = p;
}

void QgsConfigParser::addExternalGMLData( const QString& layerName, QDomDocument* gmlDoc )
{
  mExternalGMLDatasets.insert( layerName, gmlDoc );
}

void QgsConfigParser::appendLayerBoundingBoxes( QDomElement& layerElem,
    QDomDocument& doc,
    const QgsRectangle& layerExtent,
    const QgsCoordinateReferenceSystem& layerCRS ) const
{
  if ( layerElem.isNull() )
  {
    return;
  }

  const QgsCoordinateReferenceSystem& wgs84 = QgsCRSCache::instance()->crsByAuthId( GEO_EPSG_CRS_AUTHID );

  QString version = doc.documentElement().attribute( "version" );

  //Ex_GeographicBoundingBox
  QDomElement ExGeoBBoxElement;
  //transform the layers native CRS into WGS84
  QgsCoordinateTransform exGeoTransform( layerCRS, wgs84 );
  QgsRectangle wgs84BoundingRect = exGeoTransform.transformBoundingBox( layerExtent );
  if ( version == "1.1.1" )   // WMS Version 1.1.1
  {
    ExGeoBBoxElement = doc.createElement( "LatLonBoundingBox" );
    ExGeoBBoxElement.setAttribute( "minx", QString::number( wgs84BoundingRect.xMinimum() ) );
    ExGeoBBoxElement.setAttribute( "maxx", QString::number( wgs84BoundingRect.xMaximum() ) );
    ExGeoBBoxElement.setAttribute( "miny", QString::number( wgs84BoundingRect.yMinimum() ) );
    ExGeoBBoxElement.setAttribute( "maxy", QString::number( wgs84BoundingRect.yMaximum() ) );
  }
  else // WMS Version 1.3.0
  {
    ExGeoBBoxElement = doc.createElement( "EX_GeographicBoundingBox" );
    QDomElement wBoundLongitudeElement = doc.createElement( "westBoundLongitude" );
    QDomText wBoundLongitudeText = doc.createTextNode( QString::number( wgs84BoundingRect.xMinimum() ) );
    wBoundLongitudeElement.appendChild( wBoundLongitudeText );
    ExGeoBBoxElement.appendChild( wBoundLongitudeElement );
    QDomElement eBoundLongitudeElement = doc.createElement( "eastBoundLongitude" );
    QDomText eBoundLongitudeText = doc.createTextNode( QString::number( wgs84BoundingRect.xMaximum() ) );
    eBoundLongitudeElement.appendChild( eBoundLongitudeText );
    ExGeoBBoxElement.appendChild( eBoundLongitudeElement );
    QDomElement sBoundLatitudeElement = doc.createElement( "southBoundLatitude" );
    QDomText sBoundLatitudeText = doc.createTextNode( QString::number( wgs84BoundingRect.yMinimum() ) );
    sBoundLatitudeElement.appendChild( sBoundLatitudeText );
    ExGeoBBoxElement.appendChild( sBoundLatitudeElement );
    QDomElement nBoundLatitudeElement = doc.createElement( "northBoundLatitude" );
    QDomText nBoundLatitudeText = doc.createTextNode( QString::number( wgs84BoundingRect.yMaximum() ) );
    nBoundLatitudeElement.appendChild( nBoundLatitudeText );
    ExGeoBBoxElement.appendChild( nBoundLatitudeElement );
  }


  //BoundingBox element
  QDomElement bBoxElement = doc.createElement( "BoundingBox" );
  if ( layerCRS.isValid() )
  {
    bBoxElement.setAttribute( version == "1.1.1" ? "SRS" : "CRS", layerCRS.authid() );
  }

  bBoxElement.setAttribute( "minx", QString::number( layerExtent.xMinimum() ) );
  bBoxElement.setAttribute( "miny", QString::number( layerExtent.yMinimum() ) );
  bBoxElement.setAttribute( "maxx", QString::number( layerExtent.xMaximum() ) );
  bBoxElement.setAttribute( "maxy", QString::number( layerExtent.yMaximum() ) );

  QDomElement lastCRSElem = layerElem.lastChildElement( version == "1.1.1" ? "SRS" : "CRS" );
  if ( !lastCRSElem.isNull() )
  {
    layerElem.insertAfter( ExGeoBBoxElement, lastCRSElem );
    layerElem.insertAfter( bBoxElement, ExGeoBBoxElement );
  }
  else
  {
    layerElem.appendChild( ExGeoBBoxElement );
    layerElem.appendChild( bBoxElement );
  }
}

QStringList QgsConfigParser::createCRSListForLayer( QgsMapLayer* theMapLayer ) const
{
  QStringList crsNumbers;
  QString myDatabaseFileName = QgsApplication::srsDbFilePath();
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  //check the db is available
  myResult = sqlite3_open( myDatabaseFileName.toLocal8Bit().data(), &myDatabase );
  if ( myResult && theMapLayer )
  {
    //if the database cannot be opened, add at least the epsg number of the source coordinate system
    crsNumbers.push_back( theMapLayer->crs().authid() );
    return crsNumbers;
  };
  QString mySql = "select upper(auth_name||':'||auth_id) from tbl_srs";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.length(), &myPreparedStatement, &myTail );
  if ( myResult == SQLITE_OK )
  {
    while ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      crsNumbers.push_back( QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) ) );
    }
  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return crsNumbers;
}

#if 0
bool QgsConfigParser::latlonGeographicBoundingBox( const QDomElement& layerElement, QgsRectangle& rect ) const //WMS 1.1.1?
{
  if ( layerElement.isNull() )
  {
    return false;
  }

  QDomElement latlonGeogElem = layerElement.firstChildElement( "LatLonBoundingBox" );
  if ( latlonGeogElem.isNull() )
  {
    return false;
  }

  bool ok = true;
  //minx
  QString westBoundElem = latlonGeogElem.attribute( "minx" );
  if ( westBoundElem.isNull() )
  {
    return false;
  }
  double minx = westBoundElem.toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //maxx
  QString eastBoundElem = latlonGeogElem.attribute( "maxx" );
  if ( eastBoundElem.isNull() )
  {
    return false;
  }
  double maxx = eastBoundElem.toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //miny
  QString southBoundElem = latlonGeogElem.attribute( "miny" );
  if ( southBoundElem.isNull() )
  {
    return false;
  }
  double miny = southBoundElem.toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //maxy
  QString northBoundElem = latlonGeogElem.attribute( "maxy" );
  if ( northBoundElem.isNull() )
  {
    return false;
  }
  double maxy = northBoundElem.toDouble( &ok );
  if ( !ok )
  {
    return false;
  }

  rect.setXMinimum( minx );
  rect.setXMaximum( maxx );
  rect.setYMinimum( miny );
  rect.setYMaximum( maxy );

  return true;
}

bool QgsConfigParser::exGeographicBoundingBox( const QDomElement& layerElement, QgsRectangle& rect ) const //WMS 1.3.0
{
  if ( layerElement.isNull() )
  {
    return false;
  }

  QDomElement exGeogElem = layerElement.firstChildElement( "EX_GeographicBoundingBox" );
  if ( exGeogElem.isNull() )
  {
    return false;
  }

  bool ok = true;
  //minx
  QDomElement westBoundElem = exGeogElem.firstChildElement( "westBoundLongitude" );
  if ( westBoundElem.isNull() )
  {
    return false;
  }
  double minx = westBoundElem.text().toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //maxx
  QDomElement eastBoundElem = exGeogElem.firstChildElement( "eastBoundLongitude" );
  if ( eastBoundElem.isNull() )
  {
    return false;
  }
  double maxx = eastBoundElem.text().toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //miny
  QDomElement southBoundElem = exGeogElem.firstChildElement( "southBoundLatitude" );
  if ( southBoundElem.isNull() )
  {
    return false;
  }
  double miny = southBoundElem.text().toDouble( &ok );
  if ( !ok )
  {
    return false;
  }
  //maxy
  QDomElement northBoundElem = exGeogElem.firstChildElement( "northBoundLatitude" );
  if ( northBoundElem.isNull() )
  {
    return false;
  }
  double maxy = northBoundElem.text().toDouble( &ok );
  if ( !ok )
  {
    return false;
  }

  rect.setXMinimum( minx );
  rect.setXMaximum( maxx );
  rect.setYMinimum( miny );
  rect.setYMaximum( maxy );

  return true;
}
#endif


bool QgsConfigParser::crsSetForLayer( const QDomElement& layerElement, QSet<QString> &crsSet ) const
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

void QgsConfigParser::appendCRSElementsToLayer( QDomElement& layerElement, QDomDocument& doc, const QStringList &crsList ) const
{
  if ( layerElement.isNull() )
  {
    return;
  }

  //insert the CRS elements after the title element to be in accordance with the WMS 1.3 specification
  QDomElement titleElement = layerElement.firstChildElement( "Title" );
  QDomElement abstractElement = layerElement.firstChildElement( "Abstract" );
  QDomElement CRSPrecedingElement = abstractElement.isNull() ? titleElement : abstractElement; //last element before the CRS elements

  //In case the number of advertised CRS is constrained
  QStringList constrainedCrsList = supportedOutputCrsList();
  if ( constrainedCrsList.size() > 0 )
  {
    for ( int i = constrainedCrsList.size() - 1; i >= 0; --i )
    {
      appendCRSElementToLayer( layerElement, CRSPrecedingElement, constrainedCrsList.at( i ), doc );
    }
  }
  else //no crs constraint
  {
    foreach( QString crs, crsList )
    {
      appendCRSElementToLayer( layerElement, CRSPrecedingElement, crs, doc );
    }
  }
}

void QgsConfigParser::appendCRSElementToLayer( QDomElement& layerElement, const QDomElement& precedingElement, const QString& crsText, QDomDocument& doc ) const
{
  QString version = doc.documentElement().attribute( "version" );
  QDomElement crsElement = doc.createElement( version == "1.1.1" ? "SRS" : "CRS" );
  QDomText crsTextNode = doc.createTextNode( crsText );
  crsElement.appendChild( crsTextNode );
  layerElement.insertAfter( crsElement, precedingElement );
}

QgsComposition* QgsConfigParser::createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const
{
  QList<QgsComposerMap*> composerMaps;
  QList<QgsComposerLabel*> composerLabels;

  QgsComposition* c = initComposition( composerTemplate, mapRenderer, composerMaps, composerLabels );
  if ( !c )
  {
    return 0;
  }

  QString dpi = parameterMap.value( "DPI" );
  if ( !dpi.isEmpty() )
  {
    c->setPrintResolution( dpi.toInt() );
  }

  //replace composer map parameters
  foreach( QgsComposerMap* currentMap, composerMaps )
  {
    if ( !currentMap )
    {
      continue;
    }

    QString mapId = "MAP" + QString::number( currentMap->id() );

    QString extent = parameterMap.value( mapId + ":EXTENT" );
    if ( extent.isEmpty() ) //map extent is mandatory
    {
      //remove map from composition if not referenced by the request
      c->removeItem( currentMap ); delete currentMap; continue;
    }

    QStringList coordList = extent.split( "," );
    if ( coordList.size() < 4 )
    {
      c->removeItem( currentMap ); delete currentMap; continue; //need at least four coordinates
    }

    bool xMinOk, yMinOk, xMaxOk, yMaxOk;
    double xmin = coordList.at( 0 ).toDouble( &xMinOk );
    double ymin = coordList.at( 1 ).toDouble( &yMinOk );
    double xmax = coordList.at( 2 ).toDouble( &xMaxOk );
    double ymax = coordList.at( 3 ).toDouble( &yMaxOk );
    if ( !xMinOk || !yMinOk || !xMaxOk || !yMaxOk )
    {
      c->removeItem( currentMap ); delete currentMap; continue;
    }

    //Change x- and y- of extent for WMS 1.3.0 if axis inverted
    QString version = parameterMap.value( "VERSION" );
    if ( !version.isEmpty() )
    {
      if ( mapRenderer && version == "1.3.0" && mapRenderer->destinationCrs().axisInverted() )
      {
        //switch coordinates of extent
        double tmp;
        tmp = xmin;
        xmin = ymin; ymin = tmp;
        tmp = xmax;
        xmax = ymax; ymax = tmp;
      }
    }
    currentMap->setNewExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );

    //scale
    QString scaleString = parameterMap.value( mapId + ":SCALE" );
    if ( !scaleString.isEmpty() )
    {
      bool scaleOk;
      double scale = scaleString.toDouble( &scaleOk );
      if ( scaleOk )
      {
        currentMap->setNewScale( scale );
      }
    }

    //rotation
    QString rotationString = parameterMap.value( mapId + ":ROTATION" );
    if ( !rotationString.isEmpty() )
    {
      bool rotationOk;
      double rotation = rotationString.toDouble( &rotationOk );
      if ( rotationOk )
      {
        currentMap->setMapRotation( rotation );
      }
    }

    //layers / styles
    QString layers = parameterMap.value( mapId + ":LAYERS" );
    QString styles = parameterMap.value( mapId + ":STYLES" );
    if ( !layers.isEmpty() )
    {
      QStringList layerSet;
      QStringList wmsLayerList = layers.split( "," );
      QStringList wmsStyleList;

      if ( !styles.isEmpty() )
      {
        wmsStyleList = styles.split( "," );
      }

      for ( int i = 0; i < wmsLayerList.size(); ++i )
      {
        QString styleName;
        if ( wmsStyleList.size() > i )
        {
          styleName = wmsStyleList.at( i );
        }

        foreach( QgsMapLayer *layer, mapLayerFromStyle( wmsLayerList.at( i ), styleName ) )
        {
          if ( layer )
          {
            layerSet.push_back( layer->id() );
          }
        }
      }

      currentMap->setLayerSet( layerSet );
      currentMap->setKeepLayerSet( true );
    }

    //grid space x / y
    currentMap->setGridIntervalX( parameterMap.value( mapId + ":GRID_INTERVAL_X" ).toDouble() );
    currentMap->setGridIntervalY( parameterMap.value( mapId + ":GRID_INTERVAL_Y" ).toDouble() );
  }

  //replace label text
  foreach( QgsComposerLabel *currentLabel, composerLabels )
  {
    QString title = parameterMap.value( currentLabel->id().toUpper() );

    if ( title.isEmpty() )
    {
      //remove exported labels not referenced in the request
      if ( !currentLabel->id().isEmpty() )
      {
        c->removeItem( currentLabel );
        delete currentLabel;
      }
      continue;
    }

    currentLabel->setText( title );
    currentLabel->adjustSizeToText();
  }

  return c;
}

void QgsConfigParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  Q_UNUSED( doc );
  QFile wmsService( "wms_metadata.xml" );
  if ( wmsService.open( QIODevice::ReadOnly ) )
  {
    QDomDocument externServiceDoc;
    QString parseError;
    int errorLineNo;
    if ( externServiceDoc.setContent( &wmsService, false, &parseError, &errorLineNo ) )
    {
      wmsService.close();
      QDomElement service = externServiceDoc.firstChildElement();
      parentElement.appendChild( service );
    }
  }
}
