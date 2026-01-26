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

#include "qgswmsdescribelayer.h"

#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgswmsrequest.h"
#include "qgswmsserviceexception.h"
#include "qgswmsutils.h"

namespace QgsWms
{

  void writeDescribeLayer( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    const QDomDocument doc = describeLayer( serverIface, project, request );
    response.setHeader( u"Content-Type"_s, u"text/xml; charset=utf-8"_s );
    response.write( doc.toByteArray() );
  }

  // DescribeLayer is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
  QDomDocument describeLayer( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
  {
    const QgsServerRequest::Parameters parameters = request.parameters();

    if ( !parameters.contains( u"SLD_VERSION"_s ) )
    {
      throw QgsServiceException( u"MissingParameterValue"_s, u"SLD_VERSION is mandatory for DescribeLayer operation"_s, 400 );
    }
    if ( parameters[u"SLD_VERSION"_s] != "1.1.0"_L1 )
    {
      throw QgsServiceException( u"InvalidParameterValue"_s, u"SLD_VERSION = %1 is not supported"_s.arg( parameters[u"SLD_VERSION"_s] ), 400 );
    }

    if ( !parameters.contains( u"LAYERS"_s ) && !parameters.contains( u"LAYER"_s ) )
    {
      throw QgsServiceException( u"MissingParameterValue"_s, u"LAYERS or LAYER is mandatory for DescribeLayer operation"_s, 400 );
    }

    QStringList layersList;

    if ( parameters.contains( u"LAYERS"_s ) )
    {
      layersList = parameters[u"LAYERS"_s].split( ',', Qt::SkipEmptyParts );
    }
    else
    {
      layersList = parameters[u"LAYER"_s].split( ',', Qt::SkipEmptyParts );
    }
    if ( layersList.isEmpty() )
    {
      throw QgsServiceException( u"InvalidParameterValue"_s, u"Layers is empty"_s, 400 );
    }
    QDomDocument myDocument = QDomDocument();

    const QDomNode header = myDocument.createProcessingInstruction( u"xml"_s, u"version=\"1.0\" encoding=\"UTF-8\""_s );
    myDocument.appendChild( header );

    // Create the root element
    QDomElement root = myDocument.createElementNS( u"http://www.opengis.net/sld"_s, u"DescribeLayerResponse"_s );
    root.setAttribute( u"xsi:schemaLocation"_s, u"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/DescribeLayer.xsd"_s );
    root.setAttribute( u"xmlns:ows"_s, u"http://www.opengis.net/ows"_s );
    root.setAttribute( u"xmlns:se"_s, u"http://www.opengis.net/se"_s );
    root.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    root.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    myDocument.appendChild( root );

    // store the Version element
    QDomElement versionNode = myDocument.createElement( u"Version"_s );
    versionNode.appendChild( myDocument.createTextNode( u"1.1.0"_s ) );
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
        throw QgsSecurityException( u"You are not allowed to access to this layer"_s );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( accessControl && !accessControl->layerReadPermission( layer ) )
      {
        throw QgsSecurityException( u"You are not allowed to access to this layer"_s );
      }
#endif

      // Create the NamedLayer element
      QDomElement layerNode = myDocument.createElement( u"LayerDescription"_s );
      root.appendChild( layerNode );

      // store the owsType element
      QDomElement typeNode = myDocument.createElement( u"owsType"_s );
      // store the se:OnlineResource element
      QDomElement oResNode = myDocument.createElement( u"se:OnlineResource"_s );
      oResNode.setAttribute( u"xlink:type"_s, u"simple"_s );
      // store the TypeName element
      QDomElement nameNode = myDocument.createElement( u"TypeName"_s );
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
        {
          typeNode.appendChild( myDocument.createTextNode( u"wfs"_s ) );

          if ( wfsLayerIds.indexOf( layer->id() ) != -1 )
          {
            oResNode.setAttribute( u"xlink:href"_s, wfsHrefString );
          }

          // store the se:FeatureTypeName element
          QDomElement typeNameNode = myDocument.createElement( u"se:FeatureTypeName"_s );
          typeNameNode.appendChild( myDocument.createTextNode( name ) );
          nameNode.appendChild( typeNameNode );
          break;
        }
        case Qgis::LayerType::Raster:
        {
          typeNode.appendChild( myDocument.createTextNode( u"wcs"_s ) );

          if ( wcsLayerIds.indexOf( layer->id() ) != -1 )
          {
            oResNode.setAttribute( u"xlink:href"_s, wcsHrefString );
          }

          // store the se:CoverageTypeName element
          QDomElement typeNameNode = myDocument.createElement( u"se:CoverageTypeName"_s );
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
