/***************************************************************************
                              qgswmsgetstyles.h
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
#include "qgswmsserviceexception.h"
#include "qgswmsgetstyles.h"
#include "qgsserverprojectutils.h"

#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerstylemanager.h"


namespace QgsWms
{

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project,
        QStringList &layerList );
  }

  void writeGetStyles( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                       const QgsServerRequest &request, QgsServerResponse &response )
  {
    QDomDocument doc = getStyles( serverIface, project, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyles( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                          const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters parameters = request.parameters();

    QString layersName = parameters.value( "LAYERS" );

    if ( layersName.isEmpty() )
    {
      throw QgsBadRequestException( QStringLiteral( "LayerNotSpecified" ),
                                    QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    QStringList layerList = layersName.split( ',', QString::SkipEmptyParts );
    if ( layerList.isEmpty() )
    {
      throw QgsBadRequestException( QStringLiteral( "LayerNotSpecified" ),
                                    QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    return getStyledLayerDescriptorDocument( serverIface, project, layerList );
  }

  //GetStyle for compatibility with earlier QGIS versions
  void writeGetStyle( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                      const QgsServerRequest &request, QgsServerResponse &response )
  {
    QDomDocument doc = getStyle( serverIface, project, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyle( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                         const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters parameters = request.parameters();

    QDomDocument doc;

    QString styleName = parameters.value( QStringLiteral( "STYLE" ) );
    QString layerName = parameters.value( QStringLiteral( "LAYER" ) );

    if ( styleName.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "StyleNotSpecified" ),
                                 QStringLiteral( "Style is mandatory for GetStyle operation" ), 400 );
    }

    if ( layerName.isEmpty() )
    {
      throw QgsServiceException( QStringLiteral( "LayerNotSpecified" ),
                                 QStringLiteral( "Layer is mandatory for GetStyle operation" ), 400 );
    }

    QStringList layerList;
    layerList.append( layerName );
    return getStyledLayerDescriptorDocument( serverIface, project, layerList );
  }

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project,
        QStringList &layerList )
    {
      QDomDocument myDocument = QDomDocument();

      QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
      myDocument.appendChild( header );

      // Create the root element
      QDomElement root = myDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "StyledLayerDescriptor" ) );
      root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1.0" ) );
      root.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" ) );
      root.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
      root.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
      root.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      root.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      myDocument.appendChild( root );

      // access control
      QgsAccessControl *accessControl = serverIface->accessControls();
      // Use layer ids
      bool useLayerIds = QgsServerProjectUtils::wmsUseLayerIds( *project );
      // WMS restricted layers
      QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      for ( QgsMapLayer *layer : project->mapLayers() )
      {
        QString name = layer->name();
        if ( useLayerIds )
          name = layer->id();
        else if ( !layer->shortName().isEmpty() )
          name = layer->shortName();

        if ( !layerList.contains( name ) )
        {
          continue;
        }

        //unpublished layer
        if ( restrictedLayers.contains( layer->name() ) )
        {
          throw QgsSecurityException( QStringLiteral( "You are not allowed to access to this layer" ) );
        }

        if ( accessControl && !accessControl->layerReadPermission( layer ) )
        {
          throw QgsSecurityException( QStringLiteral( "You are not allowed to access to this layer" ) );
        }

        // Create the NamedLayer element
        QDomElement namedLayerNode = myDocument.createElement( QStringLiteral( "NamedLayer" ) );
        root.appendChild( namedLayerNode );

        // store the Name element
        QDomElement nameNode = myDocument.createElement( QStringLiteral( "se:Name" ) );
        nameNode.appendChild( myDocument.createTextNode( name ) );
        namedLayerNode.appendChild( nameNode );

        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
          if ( vlayer->isSpatial() )
          {
            QString currentStyle = vlayer->styleManager()->currentStyle();
            for ( const QString &styleName : vlayer->styleManager()->styles() )
            {
              vlayer->styleManager()->setCurrentStyle( styleName );
              QDomElement styleElem = vlayer->renderer()->writeSld( myDocument, styleName );
              namedLayerNode.appendChild( styleElem );
            }
            vlayer->styleManager()->setCurrentStyle( currentStyle );
          }
        }
      }

      return myDocument;
    }
  }


} // namespace QgsWms




