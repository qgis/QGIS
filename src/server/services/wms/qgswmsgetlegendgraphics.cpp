/***************************************************************************
                              qgswmsgetlegendgraphics.cpp
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
#include "qgslayertree.h"
#include "qgslegendrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeaturecounter.h"

#include "qgswmsutils.h"
#include "qgswmsserviceexception.h"
#include "qgswmsgetlegendgraphics.h"
#include "qgswmsrenderer.h"

#include <QImage>

namespace QgsWms
{
  void writeGetLegendGraphics( QgsServerInterface *serverIface, const QgsProject *project,
                               const QString &, const QgsServerRequest &request,
                               QgsServerResponse &response )
  {
    // get parameters from query
    QgsWmsParameters parameters( QUrlQuery( request.url() ) );

    // check parameters validity
    checkParameters( parameters );

    // init render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::UseScaleDenominator );
    context.setParameters( parameters );

    const QString format = request.parameters().value( QStringLiteral( "FORMAT" ), QStringLiteral( "PNG" ) );
    ImageOutputFormat outputFormat = parseImageFormat( format );

    QString saveFormat;
    QString contentType;
    switch ( outputFormat )
    {
      case PNG:
      case PNG8:
      case PNG16:
      case PNG1:
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case JPEG:
        contentType = "image/jpeg";
        saveFormat = "JPEG";
        break;
      default:
        throw QgsServiceException( "InvalidFormat",
                                   QStringLiteral( "Output format '%1' is not supported in the GetLegendGraphic request" ).arg( format ) );
        break;
    }

    // Get cached image
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager )
    {
      QImage image;
      QByteArray content = cacheManager->getCachedImage( project, request, accessControl );
      if ( !content.isEmpty() && image.loadFromData( content ) )
      {
        response.setHeader( QStringLiteral( "Content-Type" ), contentType );
        image.save( response.io(), qPrintable( saveFormat ) );
        return;
      }
    }
#endif
    QgsRenderer renderer( context );

    std::unique_ptr<QImage> result( renderer.getLegendGraphics() );

    if ( result )
    {
      writeImage( response, *result,  format, context.imageQuality() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( cacheManager )
      {
        QByteArray content = response.data();
        if ( !content.isEmpty() )
          cacheManager->setCachedImage( &content, project, request, accessControl );
      }
#endif
    }
    else
    {
      throw QgsException( QStringLiteral( "Failed to compute GetLegendGraphics image" ) );
    }
  }

  void checkParameters( const QgsWmsParameters &parameters )
  {
    if ( parameters.allLayersNickname().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MISSING_PARAMETER_VALUE,
                                    parameters[QgsWmsParameter::LAYERS] );
    }

    if ( parameters.format() == QgsWmsParameters::Format::NONE )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MISSING_PARAMETER_VALUE,
                                    parameters[QgsWmsParameter::FORMAT] );
    }

    if ( ! parameters.bbox().isEmpty() && !parameters.rule().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_INVALID_PARAMETER_VALUE,
                                    QStringLiteral( "BBOX parameter cannot be combined with RULE." ) );
    }

    if ( ! parameters.bbox().isEmpty() && parameters.bboxAsRectangle().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_INVALID_PARAMETER_VALUE,
                                    parameters[QgsWmsParameter::BBOX] );
    }
  }

  QgsLayerTreeModel *legendModel( const QgsWmsRenderContext &context, QgsLayerTree &tree )
  {
    const QgsWmsParameters parameters = context.parameters();
    std::unique_ptr<QgsLayerTreeModel> model( new QgsLayerTreeModel( &tree ) );

    if ( context.scaleDenominator() > 0 )
    {
      model->setLegendFilterByScale( context.scaleDenominator() );
    }

    // content based legend
    if ( ! parameters.bbox().isEmpty() )
    {
      QgsRenderer renderer( context );
      const QgsRenderer::HitTest symbols = renderer.symbols();

      // TODO
    }

    // if legend is not based on rendering rules
    if ( parameters.rule().isEmpty() )
    {
      QList<QgsLayerTreeNode *> children = tree.children();
      for ( QgsLayerTreeNode *node : children )
      {
        if ( ! QgsLayerTree::isLayer( node ) )
          continue;

        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

        // layer titles - hidden or not
        QgsLegendRenderer::setNodeLegendStyle( nodeLayer, parameters.layerTitleAsBool() ? QgsLegendStyle::Subgroup : QgsLegendStyle::Hidden );

        // rule item titles
        if ( !parameters.ruleLabelAsBool() )
        {
          for ( QgsLayerTreeModelLegendNode *legendNode : model->layerLegendNodes( nodeLayer ) )
          {
            // empty string = no override, so let's use one space
            legendNode->setUserLabel( QStringLiteral( " " ) );
          }
        }
        else if ( !parameters.layerTitleAsBool() )
        {
          for ( QgsLayerTreeModelLegendNode *legendNode : model->layerLegendNodes( nodeLayer ) )
          {
            if ( legendNode->isEmbeddedInParent() )
              legendNode->setEmbeddedInParent( false );
          }
        }
      }
    }

    return model.release();
  }

  QgsLayerTree *layerTree( const QgsWmsRenderContext &context )
  {
    std::unique_ptr<QgsLayerTree> tree;

    QList<QgsVectorLayerFeatureCounter *> counters;
    for ( QgsMapLayer *ml : context.layersToRender() )
    {
      QgsLayerTreeLayer *lt = tree->addLayer( ml );

      // name
      if ( !ml->title().isEmpty() )
        lt->setName( ml->title() );

      //show feature count
      const bool showFeatureCount = context.parameters().showFeatureCountAsBool();
      const QString property = QStringLiteral( "showFeatureCount" );
      lt->setCustomProperty( property, showFeatureCount );

      if ( ml->type() != QgsMapLayerType::VectorLayer || !showFeatureCount )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
      QgsVectorLayerFeatureCounter *counter = vl->countSymbolFeatures();
      if ( !counter )
        continue;

      counters.append( counter );
    }

    for ( QgsVectorLayerFeatureCounter *counter : counters )
    {
      counter->waitForFinished();
    }

    return tree.release();
  }
} // namespace QgsWms

