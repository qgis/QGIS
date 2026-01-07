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
#include "qgswcsdescribecoverage.h"

#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsserverprojectutils.h"
#include "qgswcsutils.h"

namespace QgsWcs
{

  /**
   * Output WCS DescribeCoverage response
   */
  void writeDescribeCoverage( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif
    QDomDocument doc;
    const QDomDocument *describeDocument = nullptr;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
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
#else
    doc = createDescribeCoverageDocument( serverIface, project, version, request );
    describeDocument = &doc;
#endif
    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( describeDocument->toByteArray() );
  }


  QDomDocument createDescribeCoverageDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request )
  {
    Q_UNUSED( version )

    QDomDocument doc;

    const QgsServerRequest::Parameters parameters = request.parameters();

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void ) serverIface;
#endif

    //wcs:WCS_Capabilities element
    QDomElement coveDescElement = doc.createElement( u"CoverageDescription"_s /*wcs:CoverageDescription*/ );
    coveDescElement.setAttribute( u"xmlns"_s, WCS_NAMESPACE );
    coveDescElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    coveDescElement.setAttribute( u"xsi:schemaLocation"_s, WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/describeCoverage.xsd" );
    coveDescElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
    coveDescElement.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    coveDescElement.setAttribute( u"version"_s, u"1.0.0"_s );
    coveDescElement.setAttribute( u"updateSequence"_s, u"0"_s );
    doc.appendChild( coveDescElement );

    //defining coverage name
    QString coveNames;
    //read COVERAGE
    const QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( u"COVERAGE"_s );
    if ( cove_name_it != parameters.constEnd() )
    {
      coveNames = cove_name_it.value();
    }
    if ( coveNames.isEmpty() )
    {
      const QMap<QString, QString>::const_iterator cove_name_it = parameters.constFind( u"IDENTIFIER"_s );
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

    const QStringList wcsLayersId = QgsServerProjectUtils::wcsLayerIds( *project );
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
