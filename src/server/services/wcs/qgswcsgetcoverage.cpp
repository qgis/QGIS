/***************************************************************************
                              qgswcsgetcoverage.cpp
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswcsgetcoverage.h"

#include "qgsproject.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpipe.h"
#include "qgsrasterprojector.h"
#include "qgsserverprojectutils.h"
#include "qgswcsutils.h"

#include <QTemporaryFile>

namespace QgsWcs
{

  /**
   * Output WCS DescribeCoverage response
   */
  void writeGetCoverage( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response )
  {
    Q_UNUSED( version )

    response.write( getCoverageData( serverIface, project, request ) );
    response.setHeader( "Content-Type", "image/tiff" );
  }

  QByteArray getCoverageData( QgsServerInterface *serverIface, const QgsProject *project, const QgsServerRequest &request )
  {
    const QgsServerRequest::Parameters parameters = request.parameters();

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void ) serverIface;
#endif
    //defining coverage name
    QString coveName;
    //read COVERAGE
    const QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( u"COVERAGE"_s );
    if ( cove_name_it != parameters.constEnd() )
    {
      coveName = cove_name_it.value();
    }
    if ( coveName.isEmpty() )
    {
      const QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( u"IDENTIFIER"_s );
      if ( cove_name_it != parameters.constEnd() )
      {
        coveName = cove_name_it.value();
      }
    }

    if ( coveName.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( u"COVERAGE is mandatory"_s );
    }

    //get the raster layer
    const QStringList wcsLayersId = QgsServerProjectUtils::wcsLayerIds( *project );

    QgsRasterLayer *rLayer = nullptr;
    for ( int i = 0; i < wcsLayersId.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wcsLayersId.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != Qgis::LayerType::Raster )
      {
        continue;
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !accessControl->layerReadPermission( layer ) )
      {
        continue;
      }
#endif
      QString name = layer->name();
      if ( !layer->serverProperties()->shortName().isEmpty() )
        name = layer->serverProperties()->shortName();
      name = name.replace( " "_L1, "_"_L1 );

      if ( name == coveName )
      {
        rLayer = qobject_cast<QgsRasterLayer *>( layer );
        break;
      }
    }
    if ( !rLayer )
    {
      throw QgsRequestNotWellFormedException( u"The layer for the COVERAGE '%1' is not found"_s.arg( coveName ) );
    }

    double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0;
    // WIDTh and HEIGHT
    int width = 0, height = 0;
    // CRS
    QString crs;

    // read BBOX
    const QgsRectangle bbox = parseBbox( parameters.value( u"BBOX"_s ) );
    if ( !bbox.isEmpty() )
    {
      minx = bbox.xMinimum();
      miny = bbox.yMinimum();
      maxx = bbox.xMaximum();
      maxy = bbox.yMaximum();
    }
    else
    {
      throw QgsRequestNotWellFormedException( u"The BBOX is mandatory and has to be xx.xxx,yy.yyy,xx.xxx,yy.yyy"_s );
    }

    // read WIDTH
    bool conversionSuccess = false;
    width = parameters.value( u"WIDTH"_s, u"0"_s ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      width = 0;
    }
    // read HEIGHT
    height = parameters.value( u"HEIGHT"_s, u"0"_s ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      height = 0;
    }

    if ( width < 0 || height < 0 )
    {
      throw QgsRequestNotWellFormedException( u"The WIDTH and HEIGHT are mandatory and have to be integer"_s );
    }

    crs = parameters.value( u"CRS"_s );
    if ( crs.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( u"The CRS is mandatory"_s );
    }

    const QgsCoordinateReferenceSystem requestCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
    if ( !requestCRS.isValid() )
    {
      throw QgsRequestNotWellFormedException( u"Invalid CRS"_s );
    }

    QgsRectangle rect( minx, miny, maxx, maxy );

    // transform rect
    if ( requestCRS != rLayer->crs() )
    {
      const QgsCoordinateTransform t( requestCRS, rLayer->crs(), project );
      rect = t.transformBoundingBox( rect );
    }

    // RESPONSE_CRS
    QgsCoordinateReferenceSystem responseCRS = rLayer->crs();
    crs = parameters.value( u"RESPONSE_CRS"_s );
    if ( !crs.isEmpty() )
    {
      responseCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( !responseCRS.isValid() )
      {
        responseCRS = rLayer->crs();
      }
    }

    QTemporaryFile tempFile;
    if ( !tempFile.open() )
    {
      throw QgsRequestNotWellFormedException( u"Cannot open temporary file"_s );
    }
    QgsRasterFileWriter fileWriter( tempFile.fileName() );

    // clone pipe/provider
    QgsRasterPipe pipe;
    if ( !pipe.set( rLayer->dataProvider()->clone() ) )
    {
      throw QgsRequestNotWellFormedException( u"Cannot set pipe provider"_s );
    }

    // add projector if necessary
    if ( responseCRS != rLayer->crs() )
    {
      QgsRasterProjector *projector = new QgsRasterProjector;
      projector->setCrs( rLayer->crs(), responseCRS, rLayer->transformContext() );
      if ( !pipe.insert( 2, projector ) )
      {
        throw QgsRequestNotWellFormedException( u"Cannot set pipe projector"_s );
      }
    }

    const Qgis::RasterFileWriterResult err = fileWriter.writeRaster( &pipe, width, height, rect, responseCRS, rLayer->transformContext() );
    if ( err != Qgis::RasterFileWriterResult::Success )
    {
      throw QgsRequestNotWellFormedException( u"Cannot write raster error code: %1"_s.arg( qgsEnumValueToKey( err ) ) );
    }
    return tempFile.readAll();
  }

} // namespace QgsWcs
