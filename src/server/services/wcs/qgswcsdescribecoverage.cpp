/***************************************************************************
                              qgswcsdescribecoverage.cpp
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
#include "qgsserverprojectutils.h"
#include "qgswcsdescribecoverage.h"

#include "qgsproject.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"

namespace QgsWcs
{

  /**
   * Output WCS DescribeCoverage response
   */
  void writeDescribeCoverage( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                              const QgsServerRequest &request, QgsServerResponse &response )
  {
    QgsAccessControl *accessControl = nullptr;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    accessControl = serverIface->accessControls();
#endif
    QDomDocument doc;
    const QDomDocument *describeDocument = nullptr;

    QgsServerCacheManager *cacheManager = nullptr;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    cacheManager = serverIface->cacheManager();
#endif
    if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
    {
      describeDocument = &doc;
    }
    else //describe feature xml not in cache. Create a new one
    {
      doc = createDescribeCoverageDocument( serverIface, project, version, request );

      if ( cacheManager )
      {
        cacheManager->setCachedDocument( &doc, project, request, accessControl );
      }
      describeDocument = &doc;
    }

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( describeDocument->toByteArray() );
  }


  QDomDocument createDescribeCoverageDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
      const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsServerRequest::Parameters parameters = request.parameters();

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif

    //wcs:WCS_Capabilities element
    QDomElement coveDescElement = doc.createElement( QStringLiteral( "CoverageDescription" )/*wcs:CoverageDescription*/ );
    coveDescElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    coveDescElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/describeCoverage.xsd" );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    coveDescElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    coveDescElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
    coveDescElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( coveDescElement );

    //defining coverage name
    QString coveNames;
    //read COVERAGE
    QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "COVERAGE" ) );
    if ( cove_name_it != parameters.constEnd() )
    {
      coveNames = cove_name_it.value();
    }
    if ( coveNames.isEmpty() )
    {
      QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( QStringLiteral( "IDENTIFIER" ) );
      if ( cove_name_it != parameters.constEnd() )
      {
        coveNames = cove_name_it.value();
      }
    }

    QStringList coveNameList;
    if ( !coveNames.isEmpty() )
    {
      coveNameList = coveNames.split( ',' );
      for ( int i = 0; i < coveNameList.size(); ++i )
      {
        coveNameList.replace( i, coveNameList.at( i ).trimmed() );
      }
    }

    QStringList wcsLayersId = QgsServerProjectUtils::wcsLayerIds( *project );
    for ( int i = 0; i < wcsLayersId.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wcsLayersId.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayer::LayerType::RasterLayer )
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
      if ( !layer->shortName().isEmpty() )
        name = layer->shortName();
      name = name.replace( ' ', '_' );

      if ( coveNameList.size() == 0 || coveNameList.contains( name ) )
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );
        coveDescElement.appendChild( getCoverageOffering( doc, const_cast<QgsRasterLayer *>( rLayer ), project ) );
      }
    }
    return doc;
  }

} // namespace QgsWcs



