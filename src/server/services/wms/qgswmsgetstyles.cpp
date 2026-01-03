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

#include "qgswmsgetstyles.h"

#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsserverresponse.h"
#include "qgssldexportcontext.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgswmsrendercontext.h"
#include "qgswmsrequest.h"

namespace QgsWms
{

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request );
  }

  void writeGetStyles( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    const QDomDocument doc = getStyles( serverIface, project, request );
    response.setHeader( u"Content-Type"_s, u"text/xml; charset=utf-8"_s );
    response.write( doc.toByteArray() );
  }

  QDomDocument getStyles( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
  {
    return getStyledLayerDescriptorDocument( serverIface, project, request );
  }

  namespace
  {
    QDomDocument getStyledLayerDescriptorDocument( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
    {
      // init WMS parameters and context
      const QgsWmsParameters parameters = request.wmsParameters();

      QgsWmsRenderContext context( project, serverIface );
      context.setFlag( QgsWmsRenderContext::SetAccessControl );
      context.setParameters( parameters );

      // init document
      QDomDocument myDocument = QDomDocument();

      const QDomNode header = myDocument.createProcessingInstruction( u"xml"_s, u"version=\"1.0\" encoding=\"UTF-8\""_s );
      myDocument.appendChild( header );

      // Create the root element
      QDomElement root = myDocument.createElementNS( u"http://www.opengis.net/sld"_s, u"StyledLayerDescriptor"_s );
      root.setAttribute( u"version"_s, u"1.1.0"_s );
      root.setAttribute( u"xsi:schemaLocation"_s, u"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd"_s );
      root.setAttribute( u"xmlns:ogc"_s, u"http://www.opengis.net/ogc"_s );
      root.setAttribute( u"xmlns:se"_s, u"http://www.opengis.net/se"_s );
      root.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
      root.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
      myDocument.appendChild( root );

      for ( auto layer : context.layersToRender() )
      {
        // Create the NamedLayer element
        QDomElement namedLayerNode = myDocument.createElement( u"NamedLayer"_s );
        root.appendChild( namedLayerNode );

        // store the Name element
        QDomElement nameNode = myDocument.createElement( u"se:Name"_s );
        nameNode.appendChild( myDocument.createTextNode( context.layerNickname( *layer ) ) );
        namedLayerNode.appendChild( nameNode );

        if ( layer->type() != Qgis::LayerType::Vector )
          continue;

        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        if ( !vlayer->isSpatial() )
          continue;

        const QString currentStyle = vlayer->styleManager()->currentStyle();

        QVariantMap props;
        if ( vlayer->hasScaleBasedVisibility() )
        {
          props[u"scaleMinDenom"_s] = QString::number( vlayer->maximumScale() );
          props[u"scaleMaxDenom"_s] = QString::number( vlayer->minimumScale() );
        }

        for ( const QString &styleName : vlayer->styleManager()->styles() )
        {
          vlayer->styleManager()->setCurrentStyle( styleName );

          QDomElement userStyleElem = myDocument.createElement( u"UserStyle"_s );

          QDomElement styleNameElem = myDocument.createElement( u"se:Name"_s );
          styleNameElem.appendChild( myDocument.createTextNode( styleName ) );

          userStyleElem.appendChild( styleNameElem );

          QDomElement featureTypeStyleElem = myDocument.createElement( u"se:FeatureTypeStyle"_s );
          userStyleElem.appendChild( featureTypeStyleElem );

          QgsSldExportContext exportContext;
          exportContext.setExtraProperties( props );
          vlayer->renderer()->toSld( myDocument, featureTypeStyleElem, exportContext );
          if ( vlayer->labelsEnabled() )
          {
            vlayer->labeling()->toSld( featureTypeStyleElem, exportContext );
          }

          namedLayerNode.appendChild( userStyleElem );
        }
        vlayer->styleManager()->setCurrentStyle( currentStyle );
      }

      return myDocument;
    }
  } // namespace
} // namespace QgsWms
