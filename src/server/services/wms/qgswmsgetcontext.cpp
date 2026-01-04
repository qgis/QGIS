/***************************************************************************
                              qgswmsgetcontext.cpp
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
#include "qgswmsgetcontext.h"

#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertreenode.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsserverprojectutils.h"
#include "qgswmsrequest.h"
#include "qgswmsutils.h"

#include <QRegularExpression>

namespace QgsWms
{
  namespace
  {
    void appendOwsLayerStyles( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer );

    void appendOwsLayersFromTreeGroup( QDomDocument &doc, QDomElement &parentLayer, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, const QgsLayerTreeGroup *layerTreeGroup, QgsRectangle &combinedBBox, const QString &strGroup );

    void appendOwsGeneralAndResourceList( QDomDocument &doc, QDomElement &parentElement, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request );
  } // namespace

  void writeGetContext( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif

    QDomDocument doc;
    const QDomDocument *contextDocument = nullptr;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
    {
      contextDocument = &doc;
    }
    else //context xml not in cache. Create a new one
    {
      doc = getContext( serverIface, project, request );

      if ( cacheManager )
      {
        cacheManager->setCachedDocument( &doc, project, request, accessControl );
      }
      contextDocument = &doc;
    }
#else
    doc = getContext( serverIface, project, request );
    contextDocument = &doc;
#endif
    response.setHeader( u"Content-Type"_s, u"text/xml; charset=utf-8"_s );
    response.write( contextDocument->toByteArray() );
  }


  QDomDocument getContext( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
  {
    QDomDocument doc;
    const QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( u"xml"_s, u"version=\"1.0\" encoding=\"utf-8\""_s );

    doc.appendChild( xmlDeclaration );

    QDomElement owsContextElem = doc.createElement( u"OWSContext"_s );
    owsContextElem.setAttribute( u"xmlns"_s, u"http://www.opengis.net/ows-context"_s );
    owsContextElem.setAttribute( u"xmlns:ows-context"_s, u"http://www.opengis.net/ows-context"_s );
    owsContextElem.setAttribute( u"xmlns:context"_s, u"http://www.opengis.net/context"_s );
    owsContextElem.setAttribute( u"xmlns:ows"_s, u"http://www.opengis.net/ows"_s );
    owsContextElem.setAttribute( u"xmlns:sld"_s, u"http://www.opengis.net/sld"_s );
    owsContextElem.setAttribute( u"xmlns:ogc"_s, u"http://www.opengis.net/ogc"_s );
    owsContextElem.setAttribute( u"xmlns:gml"_s, u"http://www.opengis.net/gml"_s );
    owsContextElem.setAttribute( u"xmlns:kml"_s, u"http://www.opengis.net/kml/2.2"_s );
    owsContextElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    owsContextElem.setAttribute( u"xmlns:ns9"_s, u"http://www.w3.org/2005/Atom"_s );
    owsContextElem.setAttribute( u"xmlns:xal"_s, u"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0"_s );
    owsContextElem.setAttribute( u"xmlns:ins"_s, u"http://www.inspire.org"_s );
    owsContextElem.setAttribute( u"version"_s, u"0.3.1"_s );
    doc.appendChild( owsContextElem );

    appendOwsGeneralAndResourceList( doc, owsContextElem, serverIface, project, request );

    return doc;
  }
  namespace
  {
    void appendOwsGeneralAndResourceList( QDomDocument &doc, QDomElement &parentElement, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request )
    {
      parentElement.setAttribute( u"id"_s, "ows-context-" + project->baseName() );

      // OWSContext General element
      QDomElement generalElem = doc.createElement( u"General"_s );

      // OWSContext Window element
      QDomElement windowElem = doc.createElement( u"Window"_s );
      windowElem.setAttribute( u"height"_s, u"600"_s );
      windowElem.setAttribute( u"width"_s, u"800"_s );
      generalElem.appendChild( windowElem );

      //OWS title
      const QString title = QgsServerProjectUtils::owsServiceTitle( *project );
      QDomElement titleElem = doc.createElement( u"ows:Title"_s );
      const QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      generalElem.appendChild( titleElem );

      //OWS abstract
      const QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( u"ows:Abstract"_s );
        const QDomText abstractText = doc.createCDATASection( abstract );
        abstractElem.appendChild( abstractText );
        generalElem.appendChild( abstractElem );
      }

      //OWS Keywords
      const QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      if ( !keywords.isEmpty() )
      {
        const bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

        QDomElement keywordsElem = doc.createElement( u"ows:Keywords"_s );

        for ( int i = 0; i < keywords.size(); ++i )
        {
          const QString keyword = keywords.at( i );
          if ( !keyword.isEmpty() )
          {
            QDomElement keywordElem = doc.createElement( u"ows:Keyword"_s );
            const QDomText keywordText = doc.createTextNode( keyword );
            keywordElem.appendChild( keywordText );
            if ( sia2045 )
            {
              keywordElem.setAttribute( u"vocabulary"_s, u"SIA_Geo405"_s );
            }
            keywordsElem.appendChild( keywordElem );
          }
        }
        generalElem.appendChild( keywordsElem );
      }

      // OWSContext General element is complete
      parentElement.appendChild( generalElem );

      // OWSContext ResourceList element
      QDomElement resourceListElem = doc.createElement( u"ResourceList"_s );
      const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();
      QgsRectangle combinedBBox;
      appendOwsLayersFromTreeGroup( doc, resourceListElem, serverIface, project, request, projectLayerTreeRoot, combinedBBox, QString() );
      parentElement.appendChild( resourceListElem );

      // OWSContext BoundingBox
      const QgsCoordinateReferenceSystem projectCrs = project->crs();
      QgsRectangle mapRect = QgsServerProjectUtils::wmsExtent( *project );
      if ( mapRect.isEmpty() )
      {
        mapRect = combinedBBox;
      }
      QDomElement bboxElem = doc.createElement( u"ows:BoundingBox"_s );
      bboxElem.setAttribute( u"crs"_s, projectCrs.authid() );
      if ( projectCrs.hasAxisInverted() )
      {
        mapRect.invert();
      }
      QDomElement lowerCornerElem = doc.createElement( u"ows:LowerCorner"_s );
      const QDomText lowerCornerText = doc.createTextNode( QString::number( mapRect.xMinimum() ) + " " + QString::number( mapRect.yMinimum() ) );
      lowerCornerElem.appendChild( lowerCornerText );
      bboxElem.appendChild( lowerCornerElem );
      QDomElement upperCornerElem = doc.createElement( u"ows:UpperCorner"_s );
      const QDomText upperCornerText = doc.createTextNode( QString::number( mapRect.xMaximum() ) + " " + QString::number( mapRect.yMaximum() ) );
      upperCornerElem.appendChild( upperCornerText );
      bboxElem.appendChild( upperCornerElem );
      generalElem.appendChild( bboxElem );
    }

    void appendOwsLayersFromTreeGroup( QDomDocument &doc, QDomElement &parentLayer, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, const QgsLayerTreeGroup *layerTreeGroup, QgsRectangle &combinedBBox, const QString &strGroup )
    {
      const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      const QList<QgsLayerTreeNode *> layerTreeGroupChildren = layerTreeGroup->children();
      for ( int i = 0; i < layerTreeGroupChildren.size(); ++i )
      {
        QgsLayerTreeNode *treeNode = layerTreeGroupChildren.at( i );

        if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
        {
          QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );

          const QString name = treeGroupChild->name();
          if ( restrictedLayers.contains( name ) ) //unpublished group
          {
            continue;
          }

          QString group;
          if ( strGroup.isEmpty() )
          {
            group = name;
          }
          else
          {
            group = strGroup + "/" + name;
          }

          appendOwsLayersFromTreeGroup( doc, parentLayer, serverIface, project, request, treeGroupChild, combinedBBox, group );
        }
        else
        {
          QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
          QgsMapLayer *l = treeLayer->layer();
          if ( restrictedLayers.contains( l->name() ) ) //unpublished layer
          {
            continue;
          }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          QgsAccessControl *accessControl = serverIface->accessControls();
          if ( accessControl && !accessControl->layerReadPermission( l ) )
          {
            continue;
          }
#endif
          QDomElement layerElem = doc.createElement( u"Layer"_s );

          // queryable layer
          if ( !l->flags().testFlag( QgsMapLayer::Identifiable ) )
          {
            layerElem.setAttribute( u"queryable"_s, u"false"_s );
          }
          else
          {
            layerElem.setAttribute( u"queryable"_s, u"true"_s );
          }

          // visibility
          if ( treeLayer->itemVisibilityChecked() )
          {
            layerElem.setAttribute( u"hidden"_s, u"false"_s );
          }
          else
          {
            layerElem.setAttribute( u"hidden"_s, u"true"_s );
          }

          // layer group
          if ( !strGroup.isEmpty() )
          {
            layerElem.setAttribute( u"group"_s, strGroup );
          }

          // Because Layer transparency is used for the rendering
          // OWSContext Layer opacity is set to 1
          layerElem.setAttribute( u"opacity"_s, 1 );

          QString wmsName = l->name();
          if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
          {
            wmsName = l->id();
          }
          else if ( !l->serverProperties()->shortName().isEmpty() )
          {
            wmsName = l->serverProperties()->shortName();
          }
          // layer wms name
          layerElem.setAttribute( u"name"_s, wmsName );
          // define an id based on layer wms name
          const thread_local QRegularExpression sRegEx( u"[\\W]"_s, QRegularExpression::UseUnicodePropertiesOption );
          layerElem.setAttribute( u"id"_s, wmsName.replace( sRegEx, u"_"_s ) );

          // layer title
          QDomElement titleElem = doc.createElement( u"ows:Title"_s );
          QString title = l->serverProperties()->title();
          if ( title.isEmpty() )
          {
            title = l->name();
          }
          const QDomText titleText = doc.createTextNode( title );
          titleElem.appendChild( titleText );
          layerElem.appendChild( titleElem );

          // WMS GetMap output format
          QDomElement formatElem = doc.createElement( u"ows:OutputFormat"_s );
          const QDomText formatText = doc.createTextNode( u"image/png"_s );
          formatElem.appendChild( formatText );
          layerElem.appendChild( formatElem );

          // Get WMS service URL for Server Element
          const QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

          //href needs to be a prefix
          QString hrefString = href.toString();
          hrefString.append( href.hasQuery() ? "&" : "?" );

          // COntext Server Element with WMS service URL
          QDomElement serverElem = doc.createElement( u"Server"_s );
          serverElem.setAttribute( u"service"_s, u"urn:ogc:serviceType:WMS"_s );
          serverElem.setAttribute( u"version"_s, u"1.3.0"_s );
          serverElem.setAttribute( u"default"_s, u"true"_s );
          QDomElement orServerElem = doc.createElement( u"OnlineResource"_s );
          orServerElem.setAttribute( u"xlink:href"_s, hrefString );
          serverElem.appendChild( orServerElem );
          layerElem.appendChild( serverElem );

          const QString abstract = l->serverProperties()->abstract();
          if ( !abstract.isEmpty() )
          {
            QDomElement abstractElem = doc.createElement( u"ows:Abstract"_s );
            const QDomText abstractText = doc.createTextNode( abstract );
            abstractElem.appendChild( abstractText );
            layerElem.appendChild( abstractElem );
          }

          //min/max scale denominatorScaleBasedVisibility
          if ( l->hasScaleBasedVisibility() )
          {
            const QString minScaleString = QString::number( l->maximumScale() );
            const QString maxScaleString = QString::number( l->minimumScale() );
            QDomElement minScaleElem = doc.createElement( u"sld:MinScaleDenominator"_s );
            const QDomText minScaleText = doc.createTextNode( minScaleString );
            minScaleElem.appendChild( minScaleText );
            layerElem.appendChild( minScaleElem );
            QDomElement maxScaleElem = doc.createElement( u"sld:MaxScaleDenominator"_s );
            const QDomText maxScaleText = doc.createTextNode( maxScaleString );
            maxScaleElem.appendChild( maxScaleText );
            layerElem.appendChild( maxScaleElem );
          }

          // Style list
          appendOwsLayerStyles( doc, layerElem, l );

          //keyword list
          if ( !l->serverProperties()->keywordList().isEmpty() )
          {
            const QStringList keywordStringList = l->serverProperties()->keywordList().split( ',' );
            const bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

            QDomElement keywordsElem = doc.createElement( u"ows:Keywords"_s );
            for ( int i = 0; i < keywordStringList.size(); ++i )
            {
              QDomElement keywordElem = doc.createElement( u"ows:Keyword"_s );
              const QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
              keywordElem.appendChild( keywordText );
              if ( sia2045 )
              {
                keywordElem.setAttribute( u"vocabulary"_s, u"SIA_Geo405"_s );
              }
              keywordsElem.appendChild( keywordElem );
            }
            layerElem.appendChild( keywordsElem );
          }

          // layer data URL
          const QString dataUrl = l->serverProperties()->dataUrl();
          if ( !dataUrl.isEmpty() )
          {
            QDomElement dataUrlElem = doc.createElement( u"DataURL"_s );
            const QString dataUrlFormat = l->serverProperties()->dataUrlFormat();
            dataUrlElem.setAttribute( u"format"_s, dataUrlFormat );
            QDomElement dataORElem = doc.createElement( u"OnlineResource"_s );
            dataORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
            dataORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
            dataORElem.setAttribute( u"xlink:href"_s, dataUrl );
            dataUrlElem.appendChild( dataORElem );
            layerElem.appendChild( dataUrlElem );
          }

          // layer metadata URL
          const QList<QgsMapLayerServerProperties::MetadataUrl> urls = l->serverProperties()->metadataUrls();
          for ( const QgsMapLayerServerProperties::MetadataUrl &url : urls )
          {
            QDomElement metaUrlElem = doc.createElement( u"MetadataURL"_s );
            metaUrlElem.setAttribute( u"format"_s, url.format );
            QDomElement metaUrlORElem = doc.createElement( u"OnlineResource"_s );
            metaUrlORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
            metaUrlORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
            metaUrlORElem.setAttribute( u"xlink:href"_s, url.url );
            metaUrlElem.appendChild( metaUrlORElem );
            layerElem.appendChild( metaUrlElem );
          }

          // update combineBBox
          try
          {
            const QgsCoordinateTransform t( l->crs(), project->crs(), project );
            const QgsRectangle BBox = t.transformBoundingBox( l->extent() );
            if ( combinedBBox.isEmpty() )
            {
              combinedBBox = BBox;
            }
            else
            {
              combinedBBox.combineExtentWith( BBox );
            }
          }
          catch ( const QgsCsException &cse )
          {
            Q_UNUSED( cse )
          }

          if ( parentLayer.hasChildNodes() )
          {
            parentLayer.insertBefore( layerElem, parentLayer.firstChild() );
          }
          else
          {
            parentLayer.appendChild( layerElem );
          }
        } // end of treeNode type
      } // end of for
    }

    void appendOwsLayerStyles( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer )
    {
      for ( const QString &styleName : currentLayer->styleManager()->styles() )
      {
        QDomElement styleListElem = doc.createElement( u"StyleList"_s );
        //only one default style in project file mode
        QDomElement styleElem = doc.createElement( u"Style"_s );
        styleElem.setAttribute( u"current"_s, u"true"_s );
        QDomElement styleNameElem = doc.createElement( u"Name"_s );
        const QDomText styleNameText = doc.createTextNode( styleName );
        styleNameElem.appendChild( styleNameText );
        QDomElement styleTitleElem = doc.createElement( u"Title"_s );
        const QDomText styleTitleText = doc.createTextNode( styleName );
        styleTitleElem.appendChild( styleTitleText );
        styleElem.appendChild( styleNameElem );
        styleElem.appendChild( styleTitleElem );
        styleListElem.appendChild( styleElem );
        layerElem.appendChild( styleListElem );
      }
    }
  } // namespace

} // namespace QgsWms
