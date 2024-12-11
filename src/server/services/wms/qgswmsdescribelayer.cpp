/***************************************************************************
                              qgswmsdescribelayer.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
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

#include "qgswmsutils.h"
#include "qgswmsrequest.h"
#include "qgswmsserviceexception.h"
#include "qgswmsdescribelayer.h"
#include "qgsserverprojectutils.h"
#include "qgsproject.h"

namespace QgsWms
{

  void writeDescribeLayer( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    const QDomDocument doc = describeLayer( serverIface, project, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  // DescribeLayer is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
  QDomDocument describeLayer( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
  {
    const QgsServerRequest::Parameters parameters = request.parameters();

    if ( !parameters.contains( QStringLiteral( "SLD_VERSION" ) ) )
    {
      throw QgsServiceException( QStringLiteral( "MissingParameterValue" ), QStringLiteral( "SLD_VERSION is mandatory for DescribeLayer operation" ), 400 );
    }
    if ( parameters[QStringLiteral( "SLD_VERSION" )] != QLatin1String( "1.1.0" ) )
    {
      throw QgsServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "SLD_VERSION = %1 is not supported" ).arg( parameters[QStringLiteral( "SLD_VERSION" )] ), 400 );
    }

    if ( !parameters.contains( QStringLiteral( "LAYERS" ) ) && !parameters.contains( QStringLiteral( "LAYER" ) ) )
    {
      throw QgsServiceException( QStringLiteral( "MissingParameterValue" ), QStringLiteral( "LAYERS or LAYER is mandatory for DescribeLayer operation" ), 400 );
    }

    QStringList layersList;

    if ( parameters.contains( QStringLiteral( "LAYERS" ) ) )
    {
      layersList = parameters[QStringLiteral( "LAYERS" )].split( ',', Qt::SkipEmptyParts );
    }
    else
    {
      layersList = parameters[QStringLiteral( "LAYER" )].split( ',', Qt::SkipEmptyParts );
    }
    if ( layersList.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Layers is empty" ), 400 );
    }
    QDomDocument myDocument = QDomDocument();

    const QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
    myDocument.appendChild( header );

    // Create the root element
    QDomElement root = myDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "DescribeLayerResponse" ) );
    root.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/DescribeLayer.xsd" ) );
    root.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
    root.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
    root.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    root.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    myDocument.appendChild( root );

    // store the Version element
    QDomElement versionNode = myDocument.createElement( QStringLiteral( "Version" ) );
    versionNode.appendChild( myDocument.createTextNode( QStringLiteral( "1.1.0" ) ) );
    root.appendChild( versionNode );

    // get the wms service url defined in project or keep the one from the
    // request url
    const QString wmsHrefString = serviceUrl( request, project, *serverIface->serverSettings() ).toString();

    // get the wfs service url defined in project or take the same as the
    // wms service url
    QString wfsHrefString = QgsServerProjectUtils::wfsServiceUrl( *project, request, *serverIface->serverSettings() );
    if ( wfsHrefString.isEmpty() )
    {
      wfsHrefString = wmsHrefString;
    }

    // get the wcs service url defined in project or take the same as the
    // wms service url
    QString wcsHrefString = QgsServerProjectUtils::wcsServiceUrl( *project, request, *serverIface->serverSettings() );
    if ( wcsHrefString.isEmpty() )
    {
      wcsHrefString = wmsHrefString;
    }

    // access control
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void ) serverIface;
#endif
    // Use layer ids
    const bool useLayerIds = QgsServerProjectUtils::wmsUseLayerIds( *project );
    // WMS restricted layers
    const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );
    // WFS layers
    const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    // WCS layers
    const QStringList wcsLayerIds = QgsServerProjectUtils::wcsLayerIds( *project );

    for ( QgsMapLayer *layer : project->mapLayers() )
    {
      QString name = layer->name();
      if ( useLayerIds )
        name = layer->id();
      else if ( !layer->serverProperties()->shortName().isEmpty() )
        name = layer->serverProperties()->shortName();

      if ( !layersList.contains( name ) )
      {
        continue;
      }

      //unpublished layer
      if ( restrictedLayers.contains( layer->name() ) )
      {
        throw QgsSecurityException( QStringLiteral( "You are not allowed to access to this layer" ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( accessControl && !accessControl->layerReadPermission( layer ) )
      {
        throw QgsSecurityException( QStringLiteral( "You are not allowed to access to this layer" ) );
      }
#endif

      // Create the NamedLayer element
      QDomElement layerNode = myDocument.createElement( QStringLiteral( "LayerDescription" ) );
      root.appendChild( layerNode );

      // store the owsType element
      QDomElement typeNode = myDocument.createElement( QStringLiteral( "owsType" ) );
      // store the se:OnlineResource element
      QDomElement oResNode = myDocument.createElement( QStringLiteral( "se:OnlineResource" ) );
      oResNode.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      // store the TypeName element
      QDomElement nameNode = myDocument.createElement( QStringLiteral( "TypeName" ) );
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
        {
          typeNode.appendChild( myDocument.createTextNode( QStringLiteral( "wfs" ) ) );

          if ( wfsLayerIds.indexOf( layer->id() ) != -1 )
          {
            oResNode.setAttribute( QStringLiteral( "xlink:href" ), wfsHrefString );
          }

          // store the se:FeatureTypeName element
          QDomElement typeNameNode = myDocument.createElement( QStringLiteral( "se:FeatureTypeName" ) );
          typeNameNode.appendChild( myDocument.createTextNode( name ) );
          nameNode.appendChild( typeNameNode );
          break;
        }
        case Qgis::LayerType::Raster:
        {
          typeNode.appendChild( myDocument.createTextNode( QStringLiteral( "wcs" ) ) );

          if ( wcsLayerIds.indexOf( layer->id() ) != -1 )
          {
            oResNode.setAttribute( QStringLiteral( "xlink:href" ), wcsHrefString );
          }

          // store the se:CoverageTypeName element
          QDomElement typeNameNode = myDocument.createElement( QStringLiteral( "se:CoverageTypeName" ) );
          typeNameNode.appendChild( myDocument.createTextNode( name ) );
          nameNode.appendChild( typeNameNode );
          break;
        }

        case Qgis::LayerType::Mesh:
        case Qgis::LayerType::VectorTile:
        case Qgis::LayerType::Plugin:
        case Qgis::LayerType::Annotation:
        case Qgis::LayerType::PointCloud:
        case Qgis::LayerType::Group:
        case Qgis::LayerType::TiledScene:
          break;
      }
      layerNode.appendChild( typeNode );
      layerNode.appendChild( oResNode );
      layerNode.appendChild( nameNode );
    }

    return myDocument;
  }

} // namespace QgsWms
