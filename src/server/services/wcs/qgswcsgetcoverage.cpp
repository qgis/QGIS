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
#include "qgswcsutils.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterpipe.h"
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgswcsgetcoverage.h"

#include <QTemporaryFile>

namespace QgsWcs
{

  /**
   * Output WCS DescribeCoverage response
   */
  void writeGetCoverage( QgsServerInterface* serverIface, const QString& version,
                         const QgsServerRequest& request, QgsServerResponse& response )
  {
    Q_UNUSED( version );

    response.write( getCoverageData( serverIface, request ) );
    response.setHeader( "Content-Type", "image/tiff" );
  }

  QByteArray getCoverageData( QgsServerInterface* serverIface, const QgsServerRequest& request )
  {
    QgsWCSProjectParser* configParser = getConfigParser( serverIface );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl* accessControl = serverIface->accessControls();
#endif
    QStringList wcsLayersId = configParser->wcsLayers();

    QList<QgsMapLayer*> layerList;

    QgsServerRequest::Parameters parameters = request.parameters();

    //defining coverage name
    QString coveName;
    //read COVERAGE
    QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "COVERAGE" ) );
    if ( cove_name_it != parameters.constEnd() )
    {
      coveName = cove_name_it.value();
    }
    if ( coveName.isEmpty() )
    {
      QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "IDENTIFIER" ) );
      if ( cove_name_it != parameters.constEnd() )
      {
        coveName = cove_name_it.value();
      }
    }

    if ( coveName.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "COVERAGE is mandatory" ) );
    }

    layerList = configParser->mapLayerFromCoverage( coveName );
    if ( layerList.size() < 1 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "The layer for the COVERAGE '%1' is not found" ).arg( coveName ) );
    }

    double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0;
    // WIDTh and HEIGHT
    int width = 0, height = 0;
    // CRS
    QString crs;

    // read BBOX
    QgsRectangle bbox = parseBbox( parameters.value( QStringLiteral( "BBOX" ) ) );
    if ( !bbox.isEmpty() )
    {
      minx = bbox.xMinimum();
      miny = bbox.yMinimum();
      maxx = bbox.xMaximum();
      maxy = bbox.yMaximum();
    }
    else
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "The BBOX is mandatory and has to be xx.xxx,yy.yyy,xx.xxx,yy.yyy" ) );
    }

    // read WIDTH
    bool conversionSuccess = false;
    width = parameters.value( QStringLiteral( "WIDTH" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      width = 0;
    }
    // read HEIGHT
    height = parameters.value( QStringLiteral( "HEIGHT" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      height = 0;
    }

    if ( width < 0 || height < 0 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "The WIDTH and HEIGHT are mandatory and have to be integer" ) );
    }

    crs = parameters.value( QStringLiteral( "CRS" ) );
    if ( crs.isEmpty() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "The CRS is mandatory" ) );
    }

    QgsCoordinateReferenceSystem requestCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
    if ( !requestCRS.isValid() )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Invalid CRS" ) );
    }

    QgsRectangle rect( minx, miny, maxx, maxy );

    QgsMapLayer* layer = layerList.at( 0 );
    QgsRasterLayer* rLayer = qobject_cast<QgsRasterLayer*>( layer );
    if ( rLayer && wcsLayersId.contains( rLayer->id() ) )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !accessControl->layerReadPermission( rLayer ) )
      {
        throw QgsSecurityAccessException( QStringLiteral( "You are not allowed to access to this coverage" ) );
      }
#endif

      // RESPONSE_CRS
      QgsCoordinateReferenceSystem responseCRS = rLayer->crs();
      crs = parameters.value( QStringLiteral( "RESPONSE_CRS" ) );
      if ( crs.isEmpty() )
      {
        responseCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
        if ( !responseCRS.isValid() )
        {
          responseCRS = rLayer->crs();
        }
      }

      // transform rect
      if ( requestCRS != rLayer->crs() )
      {
        QgsCoordinateTransform t( requestCRS, rLayer->crs() );
        rect = t.transformBoundingBox( rect );
      }

      QTemporaryFile tempFile;
      tempFile.open();
      QgsRasterFileWriter fileWriter( tempFile.fileName() );

      // clone pipe/provider
      QgsRasterPipe pipe;
      if ( !pipe.set( rLayer->dataProvider()->clone() ) )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Cannet set pipe provider" ) );
      }

      // add projector if necessary
      if ( responseCRS != rLayer->crs() )
      {
        QgsRasterProjector * projector = new QgsRasterProjector;
        projector->setCrs( rLayer->crs(), responseCRS );
        if ( !pipe.insert( 2, projector ) )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "Cannot set pipe projector" ) );
        }
      }

      QgsRasterFileWriter::WriterError err = fileWriter.writeRaster( &pipe, width, height, rect, responseCRS );
      if ( err != QgsRasterFileWriter::NoError )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Cannot write raster error code: %1" ).arg( err ) );
      }
      return tempFile.readAll();
    }
    else
    {
      return QByteArray();
    }
  }

} // namespace QgsWcs



