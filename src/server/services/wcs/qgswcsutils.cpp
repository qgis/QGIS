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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsmapserviceexception.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsserverprojectutils.h"

namespace QgsWcs
{
  QString implementationVersion()
  {
    return u"1.0.0"_s;
  }

  QDomElement getCoverageOffering( QDomDocument &doc, const QgsRasterLayer *layer, const QgsProject *project, bool brief )
  {
    QDomElement layerElem;
    if ( brief )
      layerElem = doc.createElement( u"CoverageOfferingBrief"_s );
    else
      layerElem = doc.createElement( u"CoverageOffering"_s );

    // create name
    QDomElement nameElem = doc.createElement( u"name"_s );
    QString name = layer->name();
    if ( !layer->serverProperties()->shortName().isEmpty() )
      name = layer->serverProperties()->shortName();
    name = name.replace( ' ', '_' );
    const QDomText nameText = doc.createTextNode( name );
    nameElem.appendChild( nameText );
    layerElem.appendChild( nameElem );

    // create label
    QDomElement labelElem = doc.createElement( u"label"_s );
    QString title = layer->serverProperties()->title();
    if ( title.isEmpty() )
    {
      title = layer->name();
    }
    const QDomText labelText = doc.createTextNode( title );
    labelElem.appendChild( labelText );
    layerElem.appendChild( labelElem );

    //create description
    const QString abstract = layer->serverProperties()->abstract();
    if ( !abstract.isEmpty() )
    {
      QDomElement descriptionElem = doc.createElement( u"description"_s );
      const QDomText descriptionText = doc.createTextNode( abstract );
      descriptionElem.appendChild( descriptionText );
      layerElem.appendChild( descriptionElem );
    }

    //lonLatEnvelope
    const QgsCoordinateReferenceSystem layerCrs = layer->crs();
    const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( Qgis::geographicCrsAuthId() );
    const int wgs84precision = 6;
    const QgsCoordinateTransform t( layerCrs, wgs84, project );
    //transform
    QgsRectangle BBox;
    try
    {
      BBox = t.transformBoundingBox( layer->extent() );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugError( u"Transform error caught: %1. Using original layer extent."_s.arg( e.what() ) );
      BBox = layer->extent();
    }
    QDomElement lonLatElem = doc.createElement( u"lonLatEnvelope"_s );
    lonLatElem.setAttribute( u"srsName"_s, u"urn:ogc:def:crs:OGC:1.3:CRS84"_s );
    QDomElement lowerPosElem = doc.createElement( u"gml:pos"_s );
    const QDomText lowerPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( BBox.xMinimum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( BBox.yMinimum(), wgs84precision ), wgs84precision ) );
    lowerPosElem.appendChild( lowerPosText );
    lonLatElem.appendChild( lowerPosElem );
    QDomElement upperPosElem = doc.createElement( u"gml:pos"_s );
    const QDomText upperPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( BBox.xMaximum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( BBox.yMaximum(), wgs84precision ), wgs84precision ) );
    upperPosElem.appendChild( upperPosText );
    lonLatElem.appendChild( upperPosElem );
    layerElem.appendChild( lonLatElem );

    if ( brief )
      return layerElem;

    //Defines the spatial-temporal domain set of a coverage offering. The domainSet shall include a SpatialDomain
    // (describing the spatial locations for which coverages can be requested), a TemporalDomain (describing the
    // time instants or inter-vals for which coverages can be requested), or both.
    QDomElement domainSetElem = doc.createElement( u"domainSet"_s );
    layerElem.appendChild( domainSetElem );

    QDomElement spatialDomainElem = doc.createElement( u"spatialDomain"_s );
    domainSetElem.appendChild( spatialDomainElem );

    // Define precision
    int precision = 3;
    if ( layer->crs().isGeographic() )
    {
      precision = 6;
    }
    //create Envelope
    const QgsRectangle layerBBox = layer->extent();
    QDomElement envelopeElem = doc.createElement( u"gml:Envelope"_s );
    envelopeElem.setAttribute( u"srsName"_s, layerCrs.authid() );
    QDomElement lowerCornerElem = doc.createElement( u"gml:pos"_s );
    const QDomText lowerCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.xMinimum(), precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.yMinimum(), wgs84precision ), precision ) );
    lowerCornerElem.appendChild( lowerCornerText );
    envelopeElem.appendChild( lowerCornerElem );
    QDomElement upperCornerElem = doc.createElement( u"gml:pos"_s );
    const QDomText upperCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerBBox.xMaximum(), precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerBBox.yMaximum(), wgs84precision ), precision ) );
    upperCornerElem.appendChild( upperCornerText );
    envelopeElem.appendChild( upperCornerElem );
    spatialDomainElem.appendChild( envelopeElem );

    QDomElement rectGridElem = doc.createElement( u"gml:RectifiedGrid"_s );
    rectGridElem.setAttribute( u"dimension"_s, 2 );
    QDomElement limitsElem = doc.createElement( u"gml:limits"_s );
    rectGridElem.appendChild( limitsElem );
    QDomElement gridEnvElem = doc.createElement( u"gml:GridEnvelope"_s );
    limitsElem.appendChild( gridEnvElem );
    QDomElement lowElem = doc.createElement( u"gml:low"_s );
    const QDomText lowText = doc.createTextNode( u"0 0"_s );
    lowElem.appendChild( lowText );
    gridEnvElem.appendChild( lowElem );
    QDomElement highElem = doc.createElement( u"gml:high"_s );
    const QDomText highText = doc.createTextNode( QString::number( layer->width() ) + " " + QString::number( layer->height() ) );
    highElem.appendChild( highText );
    gridEnvElem.appendChild( highElem );
    spatialDomainElem.appendChild( rectGridElem );

    QDomElement xAxisElem = doc.createElement( u"gml:axisName"_s );
    const QDomText xAxisText = doc.createTextNode( u"x"_s );
    xAxisElem.appendChild( xAxisText );
    rectGridElem.appendChild( xAxisElem );

    QDomElement yAxisElem = doc.createElement( u"gml:axisName"_s );
    const QDomText yAxisText = doc.createTextNode( u"y"_s );
    yAxisElem.appendChild( yAxisText );
    rectGridElem.appendChild( yAxisElem );

    QDomElement originElem = doc.createElement( u"gml:origin"_s );
    QDomElement originPosElem = doc.createElement( u"gml:pos"_s );
    originElem.appendChild( originPosElem );
    const QDomText originPosText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.xMinimum(), precision ), precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerBBox.yMinimum(), precision ), precision ) );
    originPosElem.appendChild( originPosText );
    rectGridElem.appendChild( originElem );

    QDomElement xOffsetElem = doc.createElement( u"gml:offsetVector"_s );
    const QDomText xOffsetText = doc.createTextNode( QString::number( layer->rasterUnitsPerPixelX() ) + " 0" );
    xOffsetElem.appendChild( xOffsetText );
    rectGridElem.appendChild( xOffsetElem );

    QDomElement yOffsetElem = doc.createElement( u"gml:offsetVector"_s );
    const QDomText yOffsetText = doc.createTextNode( "0 " + QString::number( layer->rasterUnitsPerPixelY() ) );
    yOffsetElem.appendChild( yOffsetText );
    rectGridElem.appendChild( yOffsetElem );

    //GML property containing one RangeSet GML object.
    QDomElement rangeSetElem = doc.createElement( u"rangeSet"_s );
    layerElem.appendChild( rangeSetElem );

    //Defines the properties (categories, measures, or values) assigned to each location in the domain. Any such
    // property may be a scalar (numeric or text) value, such as population density, or a compound (vector or tensor)
    // value, such as incomes by race, or radiances by wavelength. The semantic of the range set is typically an
    // observable and is referenced by a URI. A rangeSet also has a reference system that is referred by the URI in
    // the refSys attribute. The refSys is either qualitative (classification) or quantitative (uom). The three attributes
    // can be included either here and in each axisDescription. If included in both places, the values in the axisDescription
    // over-ride those included in the RangeSet.
    QDomElement RangeSetElem = doc.createElement( u"RangeSet"_s );
    rangeSetElem.appendChild( RangeSetElem );

    QDomElement rsNameElem = doc.createElement( u"name"_s );
    const QDomText rsNameText = doc.createTextNode( u"Bands"_s );
    rsNameElem.appendChild( rsNameText );
    RangeSetElem.appendChild( rsNameElem );

    QDomElement rsLabelElem = doc.createElement( u"label"_s );
    const QDomText rsLabelText = doc.createTextNode( u"Bands"_s );
    rsLabelElem.appendChild( rsLabelText );
    RangeSetElem.appendChild( rsLabelElem );

    QDomElement axisDescElem = doc.createElement( u"axisDescription"_s );
    RangeSetElem.appendChild( axisDescElem );

    QDomElement AxisDescElem = doc.createElement( u"AxisDescription"_s );
    axisDescElem.appendChild( AxisDescElem );

    QDomElement adNameElem = doc.createElement( u"name"_s );
    const QDomText adNameText = doc.createTextNode( u"bands"_s );
    adNameElem.appendChild( adNameText );
    AxisDescElem.appendChild( adNameElem );

    QDomElement adLabelElem = doc.createElement( u"label"_s );
    const QDomText adLablelText = doc.createTextNode( u"bands"_s );
    adLabelElem.appendChild( adLablelText );
    AxisDescElem.appendChild( adLabelElem );

    QDomElement adValuesElem = doc.createElement( u"values"_s );
    for ( int idx = 0; idx < layer->bandCount(); ++idx )
    {
      QDomElement adValueElem = doc.createElement( u"singleValue"_s );
      const QDomText adValueText = doc.createTextNode( QString::number( idx + 1 ) );
      adValueElem.appendChild( adValueText );
      adValuesElem.appendChild( adValueElem );
    }
    AxisDescElem.appendChild( adValuesElem );

    //The coordinate reference system(s) in which the server can accept requests against
    // this coverage offering and produce coverages from it.
    QDomElement sCRSElem = doc.createElement( u"supportedCRSs"_s );
    QDomElement rCRSElem = doc.createElement( u"requestResponseCRSs"_s );
    const QDomText rCRSText = doc.createTextNode( layerCrs.authid() );
    rCRSElem.appendChild( rCRSText );
    sCRSElem.appendChild( rCRSElem );
    QDomElement nCRSElem = doc.createElement( u"nativeCRSs"_s );
    const QDomText nCRSText = doc.createTextNode( layerCrs.authid() );
    nCRSElem.appendChild( nCRSText );
    sCRSElem.appendChild( nCRSElem );
    layerElem.appendChild( sCRSElem );

    //The formats (file encodings) in which the server can produce coverages from this
    // coverage offering.
    QDomElement sFormatsElem = doc.createElement( u"supportedFormats"_s );
    sFormatsElem.setAttribute( u"nativeFormat"_s, u"raw binary"_s );
    QDomElement formatsElem = doc.createElement( u"formats"_s );
    const QDomText formatsText = doc.createTextNode( u"GeoTIFF"_s );
    formatsElem.appendChild( formatsText );
    sFormatsElem.appendChild( formatsElem );
    layerElem.appendChild( sFormatsElem );

    return layerElem;
  }


  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings )
  {
    static const QSet<QString> sFilter {
      u"REQUEST"_s,
      u"VERSION"_s,
      u"SERVICE"_s,
      u"_DC"_s
    };

    QString href = QgsServerProjectUtils::wcsServiceUrl( project ? *project : *QgsProject::instance(), request, settings );

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.originalUrl();
      QUrlQuery q( url );

      const QList<QPair<QString, QString>> queryItems = q.queryItems();
      for ( const QPair<QString, QString> &param : queryItems )
      {
        if ( sFilter.contains( param.first.toUpper() ) )
          q.removeAllQueryItems( param.first );
      }

      url.setQuery( q );
      href = url.toString();
    }

    return href;
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

} // namespace QgsWcs
