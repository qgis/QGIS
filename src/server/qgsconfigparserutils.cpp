/***************************************************************************
                              qgsconfigparserutils.cpp
                              ------------------------
  begin                : March 28, 2014
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

#include "qgsconfigparserutils.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscrscache.h"
#include "qgsmaplayer.h"
#include "qgsrectangle.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QString>

#include <sqlite3.h>

class QgsCoordinateReferenceSystem;
class QgsRectangle;
class QDomDocument;
class QDomElement;
class QString;
class QStringList;

void QgsConfigParserUtils::appendCRSElementsToLayer( QDomElement& layerElement, QDomDocument& doc,
    const QStringList &crsList, const QStringList& constrainedCrsList )
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
  if ( constrainedCrsList.size() > 0 )
  {
    for ( int i = constrainedCrsList.size() - 1; i >= 0; --i )
    {
      appendCRSElementToLayer( layerElement, CRSPrecedingElement, constrainedCrsList.at( i ), doc );
    }
  }
  else //no crs constraint
  {
    Q_FOREACH ( const QString& crs, crsList )
    {
      appendCRSElementToLayer( layerElement, CRSPrecedingElement, crs, doc );
    }
  }
}

void QgsConfigParserUtils::appendCRSElementToLayer( QDomElement& layerElement, const QDomElement& precedingElement,
    const QString& crsText, QDomDocument& doc )
{
  QString version = doc.documentElement().attribute( "version" );
  QDomElement crsElement = doc.createElement( version == "1.1.1" ? "SRS" : "CRS" );
  QDomText crsTextNode = doc.createTextNode( crsText );
  crsElement.appendChild( crsTextNode );
  layerElement.insertAfter( crsElement, precedingElement );
}

void QgsConfigParserUtils::appendLayerBoundingBoxes( QDomElement& layerElem, QDomDocument& doc, const QgsRectangle& layerExtent,
    const QgsCoordinateReferenceSystem& layerCRS )
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

  QgsRectangle r( layerExtent );
  if ( version != "1.1.1" && layerCRS.axisInverted() )
  {
    r.invert();
  }

  bBoxElement.setAttribute( "minx", QString::number( r.xMinimum() ) );
  bBoxElement.setAttribute( "miny", QString::number( r.yMinimum() ) );
  bBoxElement.setAttribute( "maxx", QString::number( r.xMaximum() ) );
  bBoxElement.setAttribute( "maxy", QString::number( r.yMaximum() ) );

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

QStringList QgsConfigParserUtils::createCRSListForLayer( QgsMapLayer* theMapLayer )
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

void QgsConfigParserUtils::fallbackServiceCapabilities( QDomElement& parentElement, QDomDocument& doc )
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

QList<QgsMapLayer*> QgsConfigParserUtils::layerMapToList( const QMap< int, QgsMapLayer* >& layerMap, bool reverseOrder )
{
  if ( reverseOrder ) //reverse order
  {
    QList<QgsMapLayer*> list;
    QMapIterator< int, QgsMapLayer* > layerMapIt( layerMap );
    layerMapIt.toBack();
    while ( layerMapIt.hasPrevious() )
    {
      layerMapIt.previous();
      list.append( layerMapIt.value() );
    }
    return list;
  }
  return layerMap.values(); //take numbered drawing order
}
