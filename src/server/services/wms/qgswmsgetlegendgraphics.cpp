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
#include "qgswmsrequest.h"
#include "qgswmsserviceexception.h"
#include "qgswmsgetlegendgraphics.h"
#include "qgswmsrenderer.h"
#include "qgsserverprojectutils.h"

#include <QImage>
#include <QJsonObject>
#include <QJsonDocument>

namespace QgsWms
{
  void writeGetLegendGraphics( QgsServerInterface *serverIface, const QgsProject *project,
                               const QgsWmsRequest &request,
                               QgsServerResponse &response )
  {
    // get parameters from query
    QgsWmsParameters parameters = request.wmsParameters();

    // check parameters validity
    // FIXME fail with png + mode
    checkParameters( parameters );

    // init render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::UseScaleDenominator );
    context.setFlag( QgsWmsRenderContext::UseSrcWidthHeight );
    context.setParameters( parameters );

    // get the requested output format
    QgsWmsParameters::Format format = parameters.format();

    // parameters.format() returns NONE if the requested format is image/png with a
    // mode (e.g. image/png;mode=16bit), so in that case we use parseImageFormat to
    // give the requested format another chance

    QString imageSaveFormat;
    QString imageContentType;
    if ( format == QgsWmsParameters::Format::PNG )
    {
      imageContentType = "image/png";
      imageSaveFormat = "PNG";
    }
    else if ( format == QgsWmsParameters::Format::JPG )
    {
      imageContentType = "image/jpeg";
      imageSaveFormat = "JPEG";
    }
    else if ( format == QgsWmsParameters::Format::NONE )
    {
      switch ( parseImageFormat( parameters.formatAsString() ) )
      {
        case ImageOutputFormat::PNG:
        case ImageOutputFormat::PNG8:
        case ImageOutputFormat::PNG16:
        case ImageOutputFormat::PNG1:
          format = QgsWmsParameters::Format::PNG;
          imageContentType = "image/png";
          imageSaveFormat = "PNG";
          break;
        case ImageOutputFormat::Unknown:
          break;

        // not possible
        case QgsWms::ImageOutputFormat::JPEG:
        case QgsWms::ImageOutputFormat::WEBP:
          break;
      }
    }

    if ( format == QgsWmsParameters::Format::NONE )
    {
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidFormat,
                                    QStringLiteral( "Output format '%1' is not supported in the GetLegendGraphic request" ).arg( parameters.formatAsString() ) );
    }

    // Get cached image
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && !imageSaveFormat.isEmpty() )
    {
      QImage image;
      const QByteArray content = cacheManager->getCachedImage( project, request, accessControl );
      if ( !content.isEmpty() && image.loadFromData( content ) )
      {
        response.setHeader( QStringLiteral( "Content-Type" ), imageContentType );
        image.save( response.io(), qPrintable( imageSaveFormat ) );
        return;
      }
    }
#endif
    QgsRenderer renderer( context );

    // retrieve legend settings and model
    bool addLegendGroups = QgsServerProjectUtils::wmsAddLegendGroupsLegendGraphic( *project ) || parameters.addLayerGroups();
    std::unique_ptr<QgsLayerTree> tree( addLegendGroups ? layerTreeWithGroups( context, QgsProject::instance()->layerTreeRoot() ) : layerTree( context ) );
    const std::unique_ptr<QgsLayerTreeModel> model( legendModel( context, *tree.get() ) );

    // rendering
    if ( format == QgsWmsParameters::Format::JSON )
    {
      QJsonObject result;
      if ( !parameters.rule().isEmpty() )
      {
        throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                      QStringLiteral( "RULE cannot be used with JSON format" ) );
      }
      else
      {
        result = renderer.getLegendGraphicsAsJson( *model.get() );
      }
      tree->clear();
      response.setHeader( QStringLiteral( "Content-Type" ), parameters.formatAsString() );
      const QJsonDocument doc( result );
      response.write( doc.toJson( QJsonDocument::Compact ) );
    }
    else
    {
      std::unique_ptr<QImage> result;
      if ( !parameters.rule().isEmpty() )
      {
        QgsLayerTreeModelLegendNode *node = legendNode( parameters.rule(), *model.get() );
        if ( ! node )
        {
          throw QgsException( QStringLiteral( "Could not get a legend node for the requested RULE" ) );
        }
        result.reset( renderer.getLegendGraphics( *node ) );
      }
      else
      {
        result.reset( renderer.getLegendGraphics( *model.get() ) );
      }
      tree->clear();
      if ( result )
      {
        writeImage( response, *result, parameters.formatAsString(), context.imageQuality() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( cacheManager )
        {
          const QByteArray content = response.data();
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
  }

  void checkParameters( QgsWmsParameters &parameters )
  {
    if ( parameters.allLayersNickname().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MissingParameterValue,
                                    parameters[QgsWmsParameter::LAYERS] );
    }

    if ( parameters.format() == QgsWmsParameters::Format::NONE )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MissingParameterValue,
                                    parameters[QgsWmsParameter::FORMAT] );
    }

    if ( ! parameters.bbox().isEmpty() && !parameters.rule().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    QStringLiteral( "BBOX parameter cannot be combined with RULE." ) );
    }

    if ( ! parameters.bbox().isEmpty() && parameters.bboxAsRectangle().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    parameters[QgsWmsParameter::BBOX] );
    }
    // If we have a contextual legend (BBOX is set)
    // make sure (SRC)WIDTH and (SRC)HEIGHT are set, default to 800px width
    // height is calculated from that value, respecting the aspect
    if ( ! parameters.bbox().isEmpty() )
    {
      // Calculate ratio from bbox
      QgsRectangle bbox { parameters.bboxAsRectangle() };
      const QString crs = parameters.crs();
      if ( crs.compare( QStringLiteral( "CRS:84" ), Qt::CaseInsensitive ) == 0 )
      {
        bbox.invert();
      }
      const QgsCoordinateReferenceSystem outputCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( parameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) &&
           outputCrs.hasAxisInverted() )
      {
        bbox.invert();
      }
      const double ratio { bbox.width() / bbox.height() };
      const int defaultHeight { static_cast<int>( 800 / ratio ) };
      if ( parameters.width().isEmpty() && parameters.srcWidth().isEmpty() )
      {
        parameters.set( QgsWmsParameter::SRCWIDTH, 800 );
      }
      if ( parameters.height().isEmpty() && parameters.srcHeight().isEmpty() )
      {
        parameters.set( QgsWmsParameter::SRCHEIGHT, defaultHeight );
      }
    }
  }

  QgsLayerTreeModel *legendModel( const QgsWmsRenderContext &context, QgsLayerTree &tree )
  {

    const QgsWmsParameters parameters = context.parameters();
    std::unique_ptr<QgsLayerTreeModel> model( new QgsLayerTreeModel( &tree ) );
    std::unique_ptr<QgsMapSettings> mapSettings;

    if ( context.scaleDenominator() > 0 )
    {
      model->setLegendFilterByScale( context.scaleDenominator() );
    }

    // content based legend
    if ( ! parameters.bbox().isEmpty() )
    {
      mapSettings = std::make_unique<QgsMapSettings>();
      mapSettings->setOutputSize( context.mapSize() );
      // Inverted axis?
      QgsRectangle bbox { parameters.bboxAsRectangle() };
      const QString crs = parameters.crs();
      if ( crs.compare( QStringLiteral( "CRS:84" ), Qt::CaseInsensitive ) == 0 )
      {
        bbox.invert();
      }
      const QgsCoordinateReferenceSystem outputCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( parameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) &&
           outputCrs.hasAxisInverted() )
      {
        bbox.invert();
      }
      mapSettings->setDestinationCrs( outputCrs );
      mapSettings->setExtent( bbox );
      QgsRenderer renderer( context );
      QList<QgsMapLayer *> layers = context.layersToRender();
      renderer.configureLayers( layers, mapSettings.get() );
      mapSettings->setLayers( context.layersToRender() );
      model->setLegendFilterByMap( mapSettings.get() );
    }

    // if legend is not based on rendering rules
    if ( parameters.rule().isEmpty() )
    {
      const QList<QgsLayerTreeNode *> children = tree.children();
      const QString ruleLabel = parameters.ruleLabel();
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
        else if ( ruleLabel.compare( QStringLiteral( "AUTO" ), Qt::CaseInsensitive ) == 0 )
        {
          for ( QgsLayerTreeModelLegendNode *legendNode : model->layerLegendNodes( nodeLayer ) )
          {
            //clearing label for single symbol
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
    std::unique_ptr<QgsLayerTree> tree( new QgsLayerTree() );

    QList<QgsVectorLayerFeatureCounter *> counters;
    for ( QgsMapLayer *ml : context.layersToRender() )
    {
      QgsLayerTreeLayer *lt = tree->addLayer( ml );
      lt->setUseLayerName( false ); // do not modify underlying layer

      // name
      if ( !ml->title().isEmpty() )
        lt->setName( ml->title() );

      // show feature count
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

  QgsLayerTree *layerTreeWithGroups( const QgsWmsRenderContext &context, QgsLayerTree *projectRoot )
  {
    if ( !projectRoot )
    {
      return 0;
    }

    std::unique_ptr<QgsLayerTree> tree( new QgsLayerTree() );

    QgsWmsParameters wmsParams = context.parameters();
    QStringList layerNicknames = wmsParams.allLayersNickname();
    for ( int i = 0; i < layerNicknames.size(); ++i )
    {
      QString nickname = layerNicknames.at( i );

      //single layer
      QgsMapLayer *layer = context.layer( nickname );
      if ( layer )
      {
        tree->addLayer( layer );
      }
      else //nickname refers to a group
      {
        QgsLayerTreeGroup *group = projectRoot->findGroup( nickname );
        if ( group )
        {
          tree->insertChildNode( i, group->clone() );
        }
      }
    }

    return tree.release();
  }

  QgsLayerTreeModelLegendNode *legendNode( const QString &rule,  QgsLayerTreeModel &model )
  {
    for ( QgsLayerTreeLayer *layer : model.rootGroup()->findLayers() )
    {
      for ( QgsLayerTreeModelLegendNode *node : model.layerLegendNodes( layer ) )
      {
        if ( node->data( Qt::DisplayRole ).toString().compare( rule ) == 0 )
          return node;
      }
    }
    return nullptr;
  }
} // namespace QgsWms
