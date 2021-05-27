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
#include "qgswmsutils.h"
#include "qgswmsrequest.h"
#include "qgswmsgetcontext.h"
#include "qgsserverprojectutils.h"

#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertree.h"
#include "qgsmaplayerstylemanager.h"

#include "qgsexception.h"

namespace QgsWms
{
  namespace
  {
    void appendOwsLayerStyles( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer );

    void appendOwsLayersFromTreeGroup( QDomDocument &doc,
                                       QDomElement &parentLayer,
                                       QgsServerInterface *serverIface,
                                       const QgsProject *project,
                                       const QgsWmsRequest &request,
                                       const QgsLayerTreeGroup *layerTreeGroup,
                                       QgsRectangle &combinedBBox,
                                       const QString &strGroup );

    void appendOwsGeneralAndResourceList( QDomDocument &doc, QDomElement &parentElement,
                                          QgsServerInterface *serverIface, const QgsProject *project,
                                          const QgsWmsRequest &request );
  }

  void writeGetContext( QgsServerInterface *serverIface, const QgsProject *project,
                        const QgsWmsRequest &request,
                        QgsServerResponse &response )
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
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( contextDocument->toByteArray() );
  }


  QDomDocument getContext( QgsServerInterface *serverIface,
                           const QgsProject *project,
                           const QgsWmsRequest &request )
  {
    QDomDocument doc;
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( QStringLiteral( "xml" ),
        QStringLiteral( "version=\"1.0\" encoding=\"utf-8\"" ) );

    doc.appendChild( xmlDeclaration );

    QDomElement owsContextElem = doc.createElement( QStringLiteral( "OWSContext" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows-context" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:context" ), QStringLiteral( "http://www.opengis.net/context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:kml" ), QStringLiteral( "http://www.opengis.net/kml/2.2" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ns9" ), QStringLiteral( "http://www.w3.org/2005/Atom" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xal" ), QStringLiteral( "urn:oasis:names:tc:ciq:xsdschema:xAL:2.0" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ins" ), QStringLiteral( "http://www.inspire.org" ) );
    owsContextElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "0.3.1" ) );
    doc.appendChild( owsContextElem );

    appendOwsGeneralAndResourceList( doc, owsContextElem, serverIface, project, request );

    return doc;
  }
  namespace
  {
    void appendOwsGeneralAndResourceList( QDomDocument &doc, QDomElement &parentElement,
                                          QgsServerInterface *serverIface, const QgsProject *project,
                                          const QgsWmsRequest &request )
    {
      parentElement.setAttribute( QStringLiteral( "id" ), "ows-context-" + project->baseName() );

      // OWSContext General element
      QDomElement generalElem = doc.createElement( QStringLiteral( "General" ) );

      // OWSContext Window element
      QDomElement windowElem = doc.createElement( QStringLiteral( "Window" ) );
      windowElem.setAttribute( QStringLiteral( "height" ), QStringLiteral( "600" ) );
      windowElem.setAttribute( QStringLiteral( "width" ), QStringLiteral( "800" ) );
      generalElem.appendChild( windowElem );

      //OWS title
      //why not use project title ?
      QString title = QgsServerProjectUtils::owsServiceTitle( *project );
      if ( !title.isEmpty() )
      {
        QDomElement titleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
        QDomText titleText = doc.createTextNode( title );
        titleElem.appendChild( titleText );
        generalElem.appendChild( titleElem );
      }

      //OWS abstract
      QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
        QDomText abstractText = doc.createCDATASection( abstract );
        abstractElem.appendChild( abstractText );
        generalElem.appendChild( abstractElem );
      }

      //OWS Keywords
      QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      if ( !keywords.isEmpty() )
      {
        bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

        QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );

        for ( int i = 0; i < keywords.size(); ++i )
        {
          QString keyword = keywords.at( i );
          if ( !keyword.isEmpty() )
          {
            QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
            QDomText keywordText = doc.createTextNode( keyword );
            keywordElem.appendChild( keywordText );
            if ( sia2045 )
            {
              keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
            }
            keywordsElem.appendChild( keywordElem );
          }
        }
        generalElem.appendChild( keywordsElem );
      }

      // OWSContext General element is complete
      parentElement.appendChild( generalElem );

      // OWSContext ResourceList element
      QDomElement resourceListElem = doc.createElement( QStringLiteral( "ResourceList" ) );
      const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();
      QgsRectangle combinedBBox;
      appendOwsLayersFromTreeGroup( doc, resourceListElem, serverIface, project, request, projectLayerTreeRoot, combinedBBox, QString() );
      parentElement.appendChild( resourceListElem );

      // OWSContext BoundingBox
      QgsCoordinateReferenceSystem projectCrs = project->crs();
      QgsRectangle mapRect = QgsServerProjectUtils::wmsExtent( *project );
      if ( mapRect.isEmpty() )
      {
        mapRect = combinedBBox;
      }
      QDomElement bboxElem = doc.createElement( QStringLiteral( "ows:BoundingBox" ) );
      bboxElem.setAttribute( QStringLiteral( "crs" ), projectCrs.authid() );
      if ( projectCrs.hasAxisInverted() )
      {
        mapRect.invert();
      }
      QDomElement lowerCornerElem = doc.createElement( QStringLiteral( "ows:LowerCorner" ) );
      QDomText lowerCornerText = doc.createTextNode( QString::number( mapRect.xMinimum() ) + " " +  QString::number( mapRect.yMinimum() ) );
      lowerCornerElem.appendChild( lowerCornerText );
      bboxElem.appendChild( lowerCornerElem );
      QDomElement upperCornerElem = doc.createElement( QStringLiteral( "ows:UpperCorner" ) );
      QDomText upperCornerText = doc.createTextNode( QString::number( mapRect.xMaximum() ) + " " +  QString::number( mapRect.yMaximum() ) );
      upperCornerElem.appendChild( upperCornerText );
      bboxElem.appendChild( upperCornerElem );
      generalElem.appendChild( bboxElem );
    }

    void appendOwsLayersFromTreeGroup( QDomDocument &doc,
                                       QDomElement &parentLayer,
                                       QgsServerInterface *serverIface,
                                       const QgsProject *project,
                                       const QgsWmsRequest &request,
                                       const QgsLayerTreeGroup *layerTreeGroup,
                                       QgsRectangle &combinedBBox,
                                       const QString &strGroup )
    {
      QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      QList< QgsLayerTreeNode * > layerTreeGroupChildren = layerTreeGroup->children();
      for ( int i = 0; i < layerTreeGroupChildren.size(); ++i )
      {
        QgsLayerTreeNode *treeNode = layerTreeGroupChildren.at( i );

        if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
        {
          QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );

          QString name = treeGroupChild->name();
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
          QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );

          // queryable layer
          if ( !l->flags().testFlag( QgsMapLayer::Identifiable ) )
          {
            layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "false" ) );
          }
          else
          {
            layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "true" ) );
          }

          // visibility
          if ( treeLayer->itemVisibilityChecked() )
          {
            layerElem.setAttribute( QStringLiteral( "hidden" ), QStringLiteral( "false" ) );
          }
          else
          {
            layerElem.setAttribute( QStringLiteral( "hidden" ), QStringLiteral( "true" ) );
          }

          // layer group
          if ( !strGroup.isEmpty() )
          {
            layerElem.setAttribute( QStringLiteral( "group" ), strGroup );
          }

          // Because Layer transparency is used for the rendering
          // OWSContext Layer opacity is set to 1
          layerElem.setAttribute( QStringLiteral( "opacity" ), 1 );

          QString wmsName = l->name();
          if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
          {
            wmsName = l->id();
          }
          else if ( !l->shortName().isEmpty() )
          {
            wmsName = l->shortName();
          }
          // layer wms name
          layerElem.setAttribute( QStringLiteral( "name" ), wmsName );
          // define an id based on layer wms name
          layerElem.setAttribute( QStringLiteral( "id" ), wmsName.replace( QRegExp( "[\\W]" ), QStringLiteral( "_" ) ) );

          // layer title
          QDomElement titleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
          QString title = l->title();
          if ( title.isEmpty() )
          {
            title = l->name();
          }
          QDomText titleText = doc.createTextNode( title );
          titleElem.appendChild( titleText );
          layerElem.appendChild( titleElem );

          // WMS GetMap output format
          QDomElement formatElem = doc.createElement( QStringLiteral( "ows:OutputFormat" ) );
          QDomText formatText = doc.createTextNode( QStringLiteral( "image/png" ) );
          formatElem.appendChild( formatText );
          layerElem.appendChild( formatElem );

          // Get WMS service URL for Server Element
          QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

          //href needs to be a prefix
          QString hrefString = href.toString();
          hrefString.append( href.hasQuery() ? "&" : "?" );

          // COntext Server Element with WMS service URL
          QDomElement serverElem = doc.createElement( QStringLiteral( "Server" ) );
          serverElem.setAttribute( QStringLiteral( "service" ), QStringLiteral( "urn:ogc:serviceType:WMS" ) );
          serverElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.3.0" ) );
          serverElem.setAttribute( QStringLiteral( "default" ), QStringLiteral( "true" ) );
          QDomElement orServerElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
          orServerElem.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
          serverElem.appendChild( orServerElem );
          layerElem.appendChild( serverElem );

          QString abstract = l->abstract();
          if ( !abstract.isEmpty() )
          {
            QDomElement abstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
            QDomText abstractText = doc.createTextNode( abstract );
            abstractElem.appendChild( abstractText );
            layerElem.appendChild( abstractElem );
          }

          //min/max scale denominatorScaleBasedVisibility
          if ( l->hasScaleBasedVisibility() )
          {
            QString minScaleString = QString::number( l->maximumScale() );
            QString maxScaleString = QString::number( l->minimumScale() );
            QDomElement minScaleElem = doc.createElement( QStringLiteral( "sld:MinScaleDenominator" ) );
            QDomText minScaleText = doc.createTextNode( minScaleString );
            minScaleElem.appendChild( minScaleText );
            layerElem.appendChild( minScaleElem );
            QDomElement maxScaleElem = doc.createElement( QStringLiteral( "sld:MaxScaleDenominator" ) );
            QDomText maxScaleText = doc.createTextNode( maxScaleString );
            maxScaleElem.appendChild( maxScaleText );
            layerElem.appendChild( maxScaleElem );
          }

          // Style list
          appendOwsLayerStyles( doc, layerElem, l );

          //keyword list
          if ( !l->keywordList().isEmpty() )
          {
            QStringList keywordStringList = l->keywordList().split( ',' );
            bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

            QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
            for ( int i = 0; i < keywordStringList.size(); ++i )
            {
              QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
              QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
              keywordElem.appendChild( keywordText );
              if ( sia2045 )
              {
                keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
              }
              keywordsElem.appendChild( keywordElem );
            }
            layerElem.appendChild( keywordsElem );
          }

          // layer data URL
          QString dataUrl = l->dataUrl();
          if ( !dataUrl.isEmpty() )
          {
            QDomElement dataUrlElem = doc.createElement( QStringLiteral( "DataURL" ) );
            QString dataUrlFormat = l->dataUrlFormat();
            dataUrlElem.setAttribute( QStringLiteral( "format" ), dataUrlFormat );
            QDomElement dataORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
            dataORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
            dataORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
            dataORElem.setAttribute( QStringLiteral( "xlink:href" ), dataUrl );
            dataUrlElem.appendChild( dataORElem );
            layerElem.appendChild( dataUrlElem );
          }

          // layer metadata URL
          QString metadataUrl = l->metadataUrl();
          if ( !metadataUrl.isEmpty() )
          {
            QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
            QString metadataUrlFormat = l->metadataUrlFormat();
            metaUrlElem.setAttribute( QStringLiteral( "format" ), metadataUrlFormat );
            QDomElement metaUrlORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xlink:href" ), metadataUrl );
            metaUrlElem.appendChild( metaUrlORElem );
            layerElem.appendChild( metaUrlElem );
          }

          // update combineBBox
          try
          {
            QgsCoordinateTransform t( l->crs(), project->crs(), project );
            QgsRectangle BBox = t.transformBoundingBox( l->extent() );
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
        }// end of treeNode type
      }// end of for
    }

    void appendOwsLayerStyles( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer )
    {
      for ( const QString &styleName : currentLayer->styleManager()->styles() )
      {
        QDomElement styleListElem = doc.createElement( QStringLiteral( "StyleList" ) );
        //only one default style in project file mode
        QDomElement styleElem = doc.createElement( QStringLiteral( "Style" ) );
        styleElem.setAttribute( QStringLiteral( "current" ), QStringLiteral( "true" ) );
        QDomElement styleNameElem = doc.createElement( QStringLiteral( "Name" ) );
        QDomText styleNameText = doc.createTextNode( styleName );
        styleNameElem.appendChild( styleNameText );
        QDomElement styleTitleElem = doc.createElement( QStringLiteral( "Title" ) );
        QDomText styleTitleText = doc.createTextNode( styleName );
        styleTitleElem.appendChild( styleTitleText );
        styleElem.appendChild( styleNameElem );
        styleElem.appendChild( styleTitleElem );
        styleListElem.appendChild( styleElem );
        layerElem.appendChild( styleListElem );
      }
    }
  }

} // namespace QgsWms
