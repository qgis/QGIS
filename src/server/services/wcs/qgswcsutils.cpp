/***************************************************************************
                              qgswcsutils.cpp
                              -------------------------
  begin                : December 9, 2013
  copyright            : (C) 2013 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswcsutils.h"
#include "qgsconfigcache.h"
#include "qgsserverprojectutils.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsrasterlayer.h"
#include "qgsmapserviceexception.h"
#include "qgscoordinatereferencesystem.h"

namespace QgsWcs
{
  QString implementationVersion()
  {
    return QStringLiteral( "1.0.0" );
  }

  QDomElement getCoverageOffering( QDomDocument &doc, const QgsRasterLayer *layer, const QgsProject *project, bool brief )
  {
    QDomElement layerElem;
    if ( brief )
      layerElem = doc.createElement( QStringLiteral( "CoverageOfferingBrief" ) );
    else
      layerElem = doc.createElement( QStringLiteral( "CoverageOffering" ) );

    // create name
    QDomElement nameElem = doc.createElement( QStringLiteral( "name" ) );
    QString name = layer->name();
    if ( !layer->shortName().isEmpty() )
      name = layer->shortName();
    name = name.replace( ' ', '_' );
    QDomText nameText = doc.createTextNode( name );
    nameElem.appendChild( nameText );
    layerElem.appendChild( nameElem );

    // create label
    QDomElement labelElem = doc.createElement( QStringLiteral( "label" ) );
    QString title = layer->title();
    if ( title.isEmpty() )
    {
      title = layer->name();
    }
    QDomText labelText = doc.createTextNode( title );
    labelElem.appendChild( labelText );
    layerElem.appendChild( labelElem );

    //create description
    QString abstract = layer->abstract();
    if ( !abstract.isEmpty() )
    {
      QDomElement descriptionElem = doc.createElement( QStringLiteral( "description" ) );
      QDomText descriptionText = doc.createTextNode( abstract );
      descriptionElem.appendChild( descriptionText );
      layerElem.appendChild( descriptionElem );
    }

    //lonLatEnvelope
    QgsCoordinateReferenceSystem layerCrs = layer->crs();
    QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );
    int wgs84precision = 6;
    QgsCoordinateTransform t( layerCrs, wgs84, project );
    //transform
    QgsRectangle BBox;
    try
    {
      BBox = t.transformBoundingBox( layer->extent() );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugMsg( QStringLiteral( "Transform error caught: %1. Using original layer extent." ).arg( e.what() ) );
      BBox = layer->extent();
    }
    QDomElement lonLatElem = doc.createElement( QStringLiteral( "lonLatEnvelope" ) );
    lonLatElem.setAttribute( QStringLiteral( "srsName" ), QStringLiteral( "urn:ogc:def:crs:OGC:1.3:CRS84" ) );
    QDomElement lowerPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
    QDomText lowerPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( BBox.xMinimum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( BBox.yMinimum(), wgs84precision ), wgs84precision ) );
    lowerPosElem.appendChild( lowerPosText );
    lonLatElem.appendChild( lowerPosElem );
    QDomElement upperPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
    QDomText upperPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( BBox.xMaximum(), wgs84precision ), wgs84precision ) + " " +  qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( BBox.yMaximum(), wgs84precision ), wgs84precision ) );
    upperPosElem.appendChild( upperPosText );
    lonLatElem.appendChild( upperPosElem );
    layerElem.appendChild( lonLatElem );

    if ( brief )
      return layerElem;

    //Defines the spatial-temporal domain set of a coverage offering. The domainSet shall include a SpatialDomain
    // (describing the spatial locations for which coverages can be requested), a TemporalDomain (describing the
    // time instants or inter-vals for which coverages can be requested), or both.
    QDomElement domainSetElem = doc.createElement( QStringLiteral( "domainSet" ) );
    layerElem.appendChild( domainSetElem );

    QDomElement spatialDomainElem = doc.createElement( QStringLiteral( "spatialDomain" ) );
    domainSetElem.appendChild( spatialDomainElem );

    // Define precision
    int precision = 3;
    if ( layer->crs().isGeographic() )
    {
      precision = 6;
    }
    //create Envelope
    QgsRectangle layerBBox = layer->extent();
    QDomElement envelopeElem = doc.createElement( QStringLiteral( "gml:Envelope" ) );
    envelopeElem.setAttribute( QStringLiteral( "srsName" ), layerCrs.authid() );
    QDomElement lowerCornerElem = doc.createElement( QStringLiteral( "gml:pos" ) );
    QDomText lowerCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.xMinimum(), precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.yMinimum(), wgs84precision ), precision ) );
    lowerCornerElem.appendChild( lowerCornerText );
    envelopeElem.appendChild( lowerCornerElem );
    QDomElement upperCornerElem = doc.createElement( QStringLiteral( "gml:pos" ) );
    QDomText upperCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerBBox.xMaximum(), precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerBBox.yMaximum(), wgs84precision ), precision ) );
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
    QDomText highText = doc.createTextNode( QString::number( layer->width() ) + " " + QString::number( layer->height() ) );
    highElem.appendChild( highText );
    gridEnvElem.appendChild( highElem );
    spatialDomainElem.appendChild( rectGridElem );

    QDomElement xAxisElem = doc.createElement( QStringLiteral( "gml:axisName" ) );
    QDomText xAxisText = doc.createTextNode( QStringLiteral( "x" ) );
    xAxisElem.appendChild( xAxisText );
    rectGridElem.appendChild( xAxisElem );

    QDomElement yAxisElem = doc.createElement( QStringLiteral( "gml:axisName" ) );
    QDomText yAxisText = doc.createTextNode( QStringLiteral( "y" ) );
    yAxisElem.appendChild( yAxisText );
    rectGridElem.appendChild( yAxisElem );

    QDomElement originElem = doc.createElement( QStringLiteral( "gml:origin" ) );
    QDomElement originPosElem = doc.createElement( QStringLiteral( "gml:pos" ) );
    originElem.appendChild( originPosElem );
    QDomText originPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.xMinimum(), precision ), precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.yMinimum(), precision ), precision ) );
    originPosElem.appendChild( originPosText );
    rectGridElem.appendChild( originElem );

    QDomElement xOffsetElem = doc.createElement( QStringLiteral( "gml:offsetVector" ) );
    QDomText xOffsetText = doc.createTextNode( QString::number( layer->rasterUnitsPerPixelX() ) + " 0" );
    xOffsetElem.appendChild( xOffsetText );
    rectGridElem.appendChild( xOffsetElem );

    QDomElement yOffsetElem = doc.createElement( QStringLiteral( "gml:offsetVector" ) );
    QDomText yOffsetText = doc.createTextNode( "0 " + QString::number( layer->rasterUnitsPerPixelY() ) );
    yOffsetElem.appendChild( yOffsetText );
    rectGridElem.appendChild( yOffsetElem );

    //GML property containing one RangeSet GML object.
    QDomElement rangeSetElem = doc.createElement( QStringLiteral( "rangeSet" ) );
    layerElem.appendChild( rangeSetElem );

    //Defines the properties (categories, measures, or values) assigned to each location in the domain. Any such
    // property may be a scalar (numeric or text) value, such as population density, or a compound (vector or tensor)
    // value, such as incomes by race, or radiances by wavelength. The semantic of the range set is typically an
    // observable and is referenced by a URI. A rangeSet also has a reference system that is referred by the URI in
    // the refSys attribute. The refSys is either qualitative (classification) or quantitative (uom). The three attributes
    // can be included either here and in each axisDescription. If included in both places, the values in the axisDescription
    // over-ride those included in the RangeSet.
    QDomElement RangeSetElem = doc.createElement( QStringLiteral( "RangeSet" ) );
    rangeSetElem.appendChild( RangeSetElem );

    QDomElement rsNameElem = doc.createElement( QStringLiteral( "name" ) );
    QDomText rsNameText = doc.createTextNode( QStringLiteral( "Bands" ) );
    rsNameElem.appendChild( rsNameText );
    RangeSetElem.appendChild( rsNameElem );

    QDomElement rsLabelElem = doc.createElement( QStringLiteral( "label" ) );
    QDomText rsLabelText = doc.createTextNode( QStringLiteral( "Bands" ) );
    rsLabelElem.appendChild( rsLabelText );
    RangeSetElem.appendChild( rsLabelElem );

    QDomElement axisDescElem = doc.createElement( QStringLiteral( "axisDescription" ) );
    RangeSetElem.appendChild( axisDescElem );

    QDomElement AxisDescElem = doc.createElement( QStringLiteral( "AxisDescription" ) );
    axisDescElem.appendChild( AxisDescElem );

    QDomElement adNameElem = doc.createElement( QStringLiteral( "name" ) );
    QDomText adNameText = doc.createTextNode( QStringLiteral( "bands" ) );
    adNameElem.appendChild( adNameText );
    AxisDescElem.appendChild( adNameElem );

    QDomElement adLabelElem = doc.createElement( QStringLiteral( "label" ) );
    QDomText adLablelText = doc.createTextNode( QStringLiteral( "bands" ) );
    adLabelElem.appendChild( adLablelText );
    AxisDescElem.appendChild( adLabelElem );

    QDomElement adValuesElem = doc.createElement( QStringLiteral( "values" ) );
    for ( int idx = 0; idx < layer->bandCount(); ++idx )
    {
      QDomElement adValueElem = doc.createElement( QStringLiteral( "singleValue" ) );
      QDomText adValueText = doc.createTextNode( QString::number( idx + 1 ) );
      adValueElem.appendChild( adValueText );
      adValuesElem.appendChild( adValueElem );
    }
    AxisDescElem.appendChild( adValuesElem );

    //The coordinate reference system(s) in which the server can accept requests against
    // this coverage offering and produce coverages from it.
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

    //The formats (file encodings) in which the server can produce coverages from this
    // coverage offering.
    QDomElement sFormatsElem = doc.createElement( QStringLiteral( "supportedFormats" ) );
    sFormatsElem.setAttribute( QStringLiteral( "nativeFormat" ), QStringLiteral( "raw binary" ) );
    QDomElement formatsElem = doc.createElement( QStringLiteral( "formats" ) );
    QDomText formatsText = doc.createTextNode( QStringLiteral( "GeoTIFF" ) );
    formatsElem.appendChild( formatsText );
    sFormatsElem.appendChild( formatsElem );
    layerElem.appendChild( sFormatsElem );

    return layerElem;
  }


  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings )
  {
    static QSet< QString > sFilter
    {
      QStringLiteral( "REQUEST" ),
      QStringLiteral( "VERSION" ),
      QStringLiteral( "SERVICE" ),
      QStringLiteral( "_DC" )
    };

    QString href = QgsServerProjectUtils::wcsServiceUrl( project ? *project : *QgsProject::instance(), request, settings );

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.originalUrl();
      QUrlQuery q( url );

      for ( auto param : q.queryItems() )
      {
        if ( sFilter.contains( param.first.toUpper() ) )
          q.removeAllQueryItems( param.first );
      }

      url.setQuery( q );
      href = url.toString();

    }

    return  href;
  }

  QgsRectangle parseBbox( const QString &bboxStr )
  {
    QStringList lst = bboxStr.split( ',' );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( ' ', '+' );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

} // namespace QgsWfs


