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
#include "qgswmsrequest.h"
#include "qgswmsserviceexception.h"
#include "qgswmsgetstyles.h"
#include "qgswmsrendercontext.h"
#include "qgsserverprojectutils.h"

#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayerlabeling.h"


namespace QgsWms
{

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request );
  }

  void writeGetStyles( QgsServerInterface *serverIface, const QgsProject *project,
                       const QgsWmsRequest &request, QgsServerResponse &response )
  {
    const QDomDocument doc = getStyles( serverIface, project, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyles( QgsServerInterface *serverIface, const QgsProject *project,
                          const QgsWmsRequest &request )
  {
    return getStyledLayerDescriptorDocument( serverIface, project, request );
  }

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project,   const QgsWmsRequest &request )
    {
      // init WMS parameters and context
      const QgsWmsParameters parameters = request.wmsParameters();

      QgsWmsRenderContext context( project, serverIface );
      context.setFlag( QgsWmsRenderContext::SetAccessControl );
      context.setParameters( parameters );

      // init document
      QDomDocument myDocument = QDomDocument();

      const QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
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

      for ( auto layer : context.layersToRender() )
      {
        // Create the NamedLayer element
        QDomElement namedLayerNode = myDocument.createElement( QStringLiteral( "NamedLayer" ) );
        root.appendChild( namedLayerNode );

        // store the Name element
        QDomElement nameNode = myDocument.createElement( QStringLiteral( "se:Name" ) );
        nameNode.appendChild( myDocument.createTextNode( context.layerNickname( *layer ) ) );
        namedLayerNode.appendChild( nameNode );

        if ( layer->type() != QgsMapLayerType::VectorLayer )
          continue;

        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        if ( ! vlayer->isSpatial() )
          continue;

        const QString currentStyle = vlayer->styleManager()->currentStyle();

        QVariantMap props;
        if ( vlayer->hasScaleBasedVisibility() )
        {
          props[ QStringLiteral( "scaleMinDenom" ) ] = QString::number( vlayer->maximumScale() );
          props[ QStringLiteral( "scaleMaxDenom" ) ] = QString::number( vlayer->minimumScale() );
        }

        for ( const QString &styleName : vlayer->styleManager()->styles() )
        {
          vlayer->styleManager()->setCurrentStyle( styleName );

          QDomElement userStyleElem = myDocument.createElement( QStringLiteral( "UserStyle" ) );

          QDomElement styleNameElem = myDocument.createElement( QStringLiteral( "se:Name" ) );
          styleNameElem.appendChild( myDocument.createTextNode( styleName ) );

          userStyleElem.appendChild( styleNameElem );

          QDomElement featureTypeStyleElem = myDocument.createElement( QStringLiteral( "se:FeatureTypeStyle" ) );
          userStyleElem.appendChild( featureTypeStyleElem );

          vlayer->renderer()->toSld( myDocument, featureTypeStyleElem, props );
          if ( vlayer->labelsEnabled() )
          {
            vlayer->labeling()->toSld( featureTypeStyleElem, props );
          }

          namedLayerNode.appendChild( userStyleElem );
        }
        vlayer->styleManager()->setCurrentStyle( currentStyle );
      }

      return myDocument;
    }
  }
} // namespace QgsWms
