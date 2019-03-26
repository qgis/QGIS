/***************************************************************************
                              qgswmsrenderer.cpp
                              -------------------
  begin                : May 14, 2006
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
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
#include "qgsjsonutils.h"
#include "qgswmsrenderer.h"
#include "qgsfilterrestorer.h"
#include "qgsexception.h"
#include "qgsfields.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsmapserviceexception.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"
#include "qgsrenderer.h"
#include "qgsfeature.h"
#include "qgsaccesscontrol.h"
#include "qgsfeaturerequest.h"
#include "qgsmaprendererjobproxy.h"
#include "qgswmsserviceexception.h"
#include "qgsserverprojectutils.h"
#include "qgsmaplayerstylemanager.h"
#include "qgswkbtypes.h"
#include "qgsannotationmanager.h"
#include "qgsannotation.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerfeaturecounter.h"
#include "qgspallabeling.h"
#include "qgslayerrestorer.h"
#include "qgsdxfexport.h"
#include "qgssymbollayerutils.h"
#include "qgsserverexception.h"
#include "qgsexpressioncontextutils.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTemporaryFile>
#include <QDir>

//for printing
#include "qgslayoutatlas.h"
#include "qgslayoutmanager.h"
#include "qgslayoutexporter.h"
#include "qgslayoutsize.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutmeasurement.h"
#include "qgsprintlayout.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapgrid.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgsfeaturefilterprovidergroup.h"
#include "qgsogcutils.h"
#include "qgsunittypes.h"
#include <QUrl>

namespace QgsWms
{

  namespace
  {

    QgsLayerTreeModelLegendNode *_findLegendNodeForRule( QgsLayerTreeModel *legendModel, const QString &rule )
    {
      for ( QgsLayerTreeLayer *nodeLayer : legendModel->rootGroup()->findLayers() )
      {
        for ( QgsLayerTreeModelLegendNode *legendNode : legendModel->layerLegendNodes( nodeLayer ) )
        {
          if ( legendNode->data( Qt::DisplayRole ).toString() == rule )
            return legendNode;
        }
      }
      return nullptr;
    }

  } // namespace


  QgsRenderer::QgsRenderer( QgsServerInterface *serverIface,
                            const QgsProject *project,
                            const QgsWmsParameters &parameters )
    : mWmsParameters( parameters )
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    , mAccessControl( serverIface->accessControls() )
#endif
    , mSettings( *serverIface->serverSettings() )
    , mProject( project )
  {
    mWmsParameters.dump();

    initRestrictedLayers();
    initNicknameLayers();
  }

  QgsRenderer::~QgsRenderer()
  {
    removeTemporaryLayers();
  }


  QImage *QgsRenderer::getLegendGraphics()
  {
    // check parameters
    if ( mWmsParameters.allLayersNickname().isEmpty() )
      throw QgsBadRequestException( QStringLiteral( "LayerNotSpecified" ),
                                    QStringLiteral( "LAYER is mandatory for GetLegendGraphic operation" ) );

    if ( mWmsParameters.format() == QgsWmsParameters::Format::NONE )
      throw QgsBadRequestException( QStringLiteral( "FormatNotSpecified" ),
                                    QStringLiteral( "FORMAT is mandatory for GetLegendGraphic operation" ) );

    double scaleDenominator = -1;
    if ( ! mWmsParameters.scale().isEmpty() )
      scaleDenominator = mWmsParameters.scaleAsDouble();

    QgsLegendSettings legendSettings = mWmsParameters.legendSettings();

    // get layers
    std::unique_ptr<QgsLayerRestorer> restorer;
    restorer.reset( new QgsLayerRestorer( mNicknameLayers.values() ) );

    QList<QgsMapLayer *> layers;
    QList<QgsWmsParametersLayer> params = mWmsParameters.layersParameters();

    QString sld = mWmsParameters.sldBody();
    if ( !sld.isEmpty() )
      layers = sldStylizedLayers( sld );
    else
      layers = stylizedLayers( params );

    removeUnwantedLayers( layers, scaleDenominator );
    std::reverse( layers.begin(), layers.end() );

    // check permissions
    for ( QgsMapLayer *ml : layers )
      checkLayerReadPermissions( ml );

    // build layer tree model for legend
    QgsLayerTree rootGroup;
    std::unique_ptr<QgsLayerTreeModel> legendModel;
    legendModel.reset( buildLegendTreeModel( layers, scaleDenominator, rootGroup ) );

    // rendering step
    qreal dpmm = dotsPerMm();
    std::unique_ptr<QImage> image;
    std::unique_ptr<QPainter> painter;

    // getting scale from bbox
    if ( !mWmsParameters.bbox().isEmpty() )
    {
      QgsMapSettings mapSettings;
      image.reset( createImage( mWmsParameters.widthAsInt(), mWmsParameters.heightAsInt(), false ) );
      configureMapSettings( image.get(), mapSettings );
      legendSettings.setMapScale( mapSettings.scale() );
      legendSettings.setMapUnitsPerPixel( mapSettings.mapUnitsPerPixel() );
    }

    if ( !mWmsParameters.rule().isEmpty() )
    {
      QString rule = mWmsParameters.rule();
      int width = mWmsParameters.widthAsInt();
      int height = mWmsParameters.heightAsInt();

      image.reset( createImage( width, height, false ) );
      painter.reset( new QPainter( image.get() ) );
      painter->setRenderHint( QPainter::Antialiasing, true );
      painter->scale( dpmm, dpmm );

      QgsLayerTreeModelLegendNode *legendNode = _findLegendNodeForRule( legendModel.get(), rule );
      if ( legendNode )
      {
        QgsLayerTreeModelLegendNode::ItemContext ctx;
        ctx.painter = painter.get();
        ctx.labelXOffset = 0;
        ctx.point = QPointF();
        double itemHeight = height / dpmm;
        legendNode->drawSymbol( legendSettings, &ctx, itemHeight );
        painter->end();
      }
    }
    else
    {
      QgsLegendRenderer legendRendererNew( legendModel.get(), legendSettings );

      QSizeF minSize = legendRendererNew.minimumSize();
      QSize s( minSize.width() * dpmm, minSize.height() * dpmm );

      image.reset( createImage( s.width(), s.height(), false ) );
      painter.reset( new QPainter( image.get() ) );
      painter->setRenderHint( QPainter::Antialiasing, true );
      painter->scale( dpmm, dpmm );

      legendRendererNew.drawLegend( painter.get() );
      painter->end();
    }

    rootGroup.clear();
    return image.release();
  }

  void QgsRenderer::runHitTest( const QgsMapSettings &mapSettings, HitTest &hitTest ) const
  {
    QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );

    for ( const QString &id : mapSettings.layerIds() )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mProject->mapLayer( id ) );
      if ( !vl || !vl->renderer() )
        continue;

      if ( vl->hasScaleBasedVisibility() && vl->isInScaleRange( mapSettings.scale() ) )
      {
        hitTest[vl] = SymbolSet(); // no symbols -> will not be shown
        continue;
      }

      QgsCoordinateTransform tr = mapSettings.layerTransform( vl );
      context.setCoordinateTransform( tr );
      context.setExtent( tr.transformBoundingBox( mapSettings.extent(), QgsCoordinateTransform::ReverseTransform ) );

      SymbolSet &usedSymbols = hitTest[vl];
      runHitTestLayer( vl, usedSymbols, context );
    }
  }

  void QgsRenderer::runHitTestLayer( QgsVectorLayer *vl, SymbolSet &usedSymbols, QgsRenderContext &context ) const
  {
    std::unique_ptr< QgsFeatureRenderer > r( vl->renderer()->clone() );
    bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature;
    r->startRender( context, vl->fields() );
    QgsFeature f;
    QgsFeatureRequest request( context.extent() );
    request.setFlags( QgsFeatureRequest::ExactIntersect );
    QgsFeatureIterator fi = vl->getFeatures( request );
    while ( fi.nextFeature( f ) )
    {
      context.expressionContext().setFeature( f );
      if ( moreSymbolsPerFeature )
      {
        for ( QgsSymbol *s : r->originalSymbolsForFeature( f, context ) )
          usedSymbols.insert( QgsSymbolLayerUtils::symbolProperties( s ) );
      }
      else
        usedSymbols.insert( QgsSymbolLayerUtils::symbolProperties( r->originalSymbolForFeature( f, context ) ) );
    }
    r->stopRender( context );
  }


  QByteArray QgsRenderer::getPrint()
  {
    //GetPrint request needs a template parameter
    QString templateName = mWmsParameters.composerTemplate();
    if ( templateName.isEmpty() )
    {
      throw QgsBadRequestException( QStringLiteral( "ParameterMissing" ),
                                    QStringLiteral( "The TEMPLATE parameter is required for the GetPrint request" ) );
    }

    // get layers parameters
    QList<QgsMapLayer *> layers;
    QList<QgsWmsParametersLayer> params = mWmsParameters.layersParameters();

    // create the output image (this is not really used but configureMapSettings
    // needs it)
    std::unique_ptr<QImage> image( new QImage() );

    // configure map settings (background, DPI, ...)
    QgsMapSettings mapSettings;
    configureMapSettings( image.get(), mapSettings );

    // init layer restorer before doing anything
    std::unique_ptr<QgsLayerRestorer> restorer;
    restorer.reset( new QgsLayerRestorer( mNicknameLayers.values() ) );

    // init stylized layers according to LAYERS/STYLES or SLD
    QString sld = mWmsParameters.sldBody();
    if ( !sld.isEmpty() )
    {
      layers = sldStylizedLayers( sld );
    }
    else
    {
      layers = stylizedLayers( params );
    }

    // remove unwanted layers (restricted layers, ...)
    removeUnwantedLayers( layers );

    // configure each layer with opacity, selection filter, ...
    bool updateMapExtent = mWmsParameters.bbox().isEmpty();
    for ( QgsMapLayer *layer : layers )
    {
      checkLayerReadPermissions( layer );

      for ( const QgsWmsParametersLayer &param : params )
      {
        if ( param.mNickname == layerNickname( *layer ) )
        {
          setLayerOpacity( layer, param.mOpacity );

          setLayerFilter( layer, param.mFilter );

          setLayerSelection( layer, param.mSelection );

          if ( updateMapExtent )
            updateExtent( layer, mapSettings );

          break;
        }
      }

      setLayerAccessControlFilter( layer );
    }

    // add highlight layers above others
    layers = layers << highlightLayers( mWmsParameters.highlightLayersParameters() );

    // add layers to map settings (revert order for the rendering)
    std::reverse( layers.begin(), layers.end() );
    mapSettings.setLayers( layers );

    const QgsLayoutManager *lManager = mProject->layoutManager();
    QgsPrintLayout *sourceLayout( dynamic_cast<QgsPrintLayout *>( lManager->layoutByName( templateName ) ) );
    if ( !sourceLayout )
    {
      throw QgsBadRequestException( QStringLiteral( "InvalidTemplate" ),
                                    QStringLiteral( "Template '%1' is not known" ).arg( templateName ) );
    }

    // Check that layout has at least one page
    if ( sourceLayout->pageCollection()->pageCount() < 1 )
    {
      throw QgsBadRequestException( QStringLiteral( "InvalidTemplate" ),
                                    QStringLiteral( "Template '%1' has no pages" ).arg( templateName ) );
    }

    std::unique_ptr<QgsPrintLayout> layout( sourceLayout->clone() );

    //atlas print?
    QgsLayoutAtlas *atlas = 0;
    QStringList atlasPk = mWmsParameters.atlasPk();
    if ( !atlasPk.isEmpty() ) //atlas print requested?
    {
      atlas = layout->atlas();
      if ( !atlas || !atlas->enabled() )
      {
        //error
        throw QgsBadRequestException( QStringLiteral( "NoAtlas" ),
                                      QStringLiteral( "The template has no atlas enabled" ) );
      }

      QgsVectorLayer *cLayer = atlas->coverageLayer();
      if ( !cLayer )
      {
        throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                      QStringLiteral( "The atlas has no coverage layer" ) );
      }

      int maxAtlasFeatures = QgsServerProjectUtils::wmsMaxAtlasFeatures( *mProject );
      if ( atlasPk.size() == 1 && atlasPk.at( 0 ) == QStringLiteral( "*" ) )
      {
        atlas->setFilterFeatures( false );
        atlas->updateFeatures();
        if ( atlas->count() > maxAtlasFeatures )
        {
          throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                        QString( "The project configuration allows printing maximum %1 atlas features at a time" ).arg( maxAtlasFeatures ) );
        }
      }
      else
      {
        QgsAttributeList pkIndexes = cLayer->primaryKeyAttributes();
        if ( pkIndexes.size() < 1 )
        {
          throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                        QStringLiteral( "An error occurred during the Atlas print" ) );
        }
        QStringList pkAttributeNames;
        for ( int i = 0; i < pkIndexes.size(); ++i )
        {
          pkAttributeNames.append( cLayer->fields()[pkIndexes.at( i )].name() );
        }

        int nAtlasFeatures = atlasPk.size() / pkIndexes.size();
        if ( nAtlasFeatures * pkIndexes.size() != atlasPk.size() ) //Test is atlasPk.size() is a multiple of pkIndexes.size(). Bail out if not
        {
          throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                        QStringLiteral( "Wrong number of ATLAS_PK parameters" ) );
        }

        //number of atlas features might be restricted
        if ( nAtlasFeatures > maxAtlasFeatures )
        {
          throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                        QString( "%1 atlas features have been requestet, but the project configuration only allows printing %2 atlas features at a time" )
                                        .arg( nAtlasFeatures ).arg( maxAtlasFeatures ) );
        }

        QString filterString;
        int currentAtlasPk = 0;

        for ( int i = 0; i < nAtlasFeatures; ++i )
        {
          if ( i > 0 )
          {
            filterString.append( " OR " );
          }

          filterString.append( "( " );

          for ( int j = 0; j < pkIndexes.size(); ++j )
          {
            if ( j > 0 )
            {
              filterString.append( " AND " );
            }
            filterString.append( QString( "\"%1\" = %2" ).arg( pkAttributeNames.at( j ) ).arg( atlasPk.at( currentAtlasPk ) ) );
            ++currentAtlasPk;
          }

          filterString.append( " )" );
        }

        atlas->setFilterFeatures( true );
        QString errorString;
        atlas->setFilterExpression( filterString, errorString );
        if ( !errorString.isEmpty() )
        {
          throw QgsBadRequestException( QStringLiteral( "AtlasPrintError" ),
                                        QStringLiteral( "An error occurred during the Atlas print" ) );
        }
      }
    }

    configurePrintLayout( layout.get(), mapSettings, atlas );

    // Get the temporary output file
    const QgsWmsParameters::Format format = mWmsParameters.format();
    const QString extension = QgsWmsParameters::formatAsString( format ).toLower();

    QTemporaryFile tempOutputFile( QDir::tempPath() +  '/' + QStringLiteral( "XXXXXX.%1" ).arg( extension ) );
    if ( !tempOutputFile.open() )
    {
      throw QgsServerException( QStringLiteral( "Could not open temporary file for the GetPrint request." ) );

    }

    QString exportError;
    if ( format == QgsWmsParameters::SVG )
    {
      // Settings for the layout exporter
      QgsLayoutExporter::SvgExportSettings exportSettings;
      if ( !mWmsParameters.dpi().isEmpty() )
      {
        bool ok;
        double dpi( mWmsParameters.dpi().toDouble( &ok ) );
        if ( ok )
          exportSettings.dpi = dpi;
      }
      // Draw selections
      exportSettings.flags |= QgsLayoutRenderContext::FlagDrawSelection;
      if ( atlas )
      {
        //export first page of atlas
        atlas->beginRender();
        if ( atlas->next() )
        {
          QgsLayoutExporter atlasSvgExport( atlas->layout() );
          atlasSvgExport.exportToSvg( tempOutputFile.fileName(), exportSettings );
        }
      }
      else
      {
        QgsLayoutExporter exporter( layout.get() );
        exporter.exportToSvg( tempOutputFile.fileName(), exportSettings );
      }
    }
    else if ( format == QgsWmsParameters::PNG || format == QgsWmsParameters::JPG )
    {
      // Settings for the layout exporter
      QgsLayoutExporter::ImageExportSettings exportSettings;

      // Get the dpi from input or use the default
      double dpi( layout->renderContext().dpi( ) );
      if ( !mWmsParameters.dpi().isEmpty() )
      {
        bool ok;
        double _dpi = mWmsParameters.dpi().toDouble( &ok );
        if ( ! ok )
          dpi = _dpi;
      }
      exportSettings.dpi = dpi;
      // Draw selections
      exportSettings.flags |= QgsLayoutRenderContext::FlagDrawSelection;
      // Destination image size in px
      QgsLayoutSize layoutSize( layout->pageCollection()->page( 0 )->sizeWithUnits() );
      QgsLayoutMeasurement width( layout->convertFromLayoutUnits( layoutSize.width(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );
      QgsLayoutMeasurement height( layout->convertFromLayoutUnits( layoutSize.height(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );
      exportSettings.imageSize = QSize( static_cast<int>( width.length() * dpi / 25.4 ), static_cast<int>( height.length() * dpi / 25.4 ) );
      // Export first page only (unless it's a pdf, see below)
      exportSettings.pages.append( 0 );
      if ( atlas )
      {
        //only can give back one page in server rendering
        atlas->beginRender();
        if ( atlas->next() )
        {
          QgsLayoutExporter atlasPngExport( atlas->layout() );
          atlasPngExport.exportToImage( tempOutputFile.fileName(), exportSettings );
        }
      }
      else
      {
        QgsLayoutExporter exporter( layout.get() );
        exporter.exportToImage( tempOutputFile.fileName(), exportSettings );
      }
    }
    else if ( format == QgsWmsParameters::PDF )
    {
      // Settings for the layout exporter
      QgsLayoutExporter::PdfExportSettings exportSettings;
      // TODO: handle size from input ?
      if ( !mWmsParameters.dpi().isEmpty() )
      {
        bool ok;
        double dpi( mWmsParameters.dpi().toDouble( &ok ) );
        if ( ok )
          exportSettings.dpi = dpi;
      }
      // Draw selections
      exportSettings.flags |= QgsLayoutRenderContext::FlagDrawSelection;
      // Print as raster
      exportSettings.rasterizeWholeImage = layout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();

      // Export all pages
      QgsLayoutExporter exporter( layout.get() );
      if ( atlas )
      {
        exporter.exportToPdf( atlas, tempOutputFile.fileName(), exportSettings, exportError );
      }
      else
      {
        exporter.exportToPdf( tempOutputFile.fileName(), exportSettings );
      }
    }
    else //unknown format
    {
      throw QgsBadRequestException( QStringLiteral( "InvalidFormat" ),
                                    QStringLiteral( "Output format '%1' is not supported in the GetPrint request" ).arg( mWmsParameters.formatAsString() ) );
    }

    if ( atlas )
    {
      handlePrintErrors( atlas->layout() );
    }
    else
    {
      handlePrintErrors( layout.get() );
    }

    return tempOutputFile.readAll();
  }

  bool QgsRenderer::configurePrintLayout( QgsPrintLayout *c, const QgsMapSettings &mapSettings, bool atlasPrint )
  {
    c->renderContext().setSelectionColor( mapSettings.selectionColor() );
    // Maps are configured first
    QList<QgsLayoutItemMap *> maps;
    c->layoutItems<QgsLayoutItemMap>( maps );
    // Layout maps now use a string UUID as "id", let's assume that the first map
    // has id 0 and so on ...
    int mapId = 0;

    for ( const auto &map : qgis::as_const( maps ) )
    {
      QgsWmsParametersComposerMap cMapParams = mWmsParameters.composerMapParameters( mapId );
      mapId++;

      if ( !atlasPrint || !map->atlasDriven() ) //No need to extent, scal, rotation set with atlas feature
      {
        //map extent is mandatory
        if ( !cMapParams.mHasExtent )
        {
          //remove map from composition if not referenced by the request
          c->removeLayoutItem( map );
          continue;
        }
        // Change CRS of map set to "project CRS" to match requested CRS
        // (if map has a valid preset crs then we keep this crs and don't use the
        // requested crs for this map item)
        if ( mapSettings.destinationCrs().isValid() && !map->presetCrs().isValid() )
          map->setCrs( mapSettings.destinationCrs() );

        QgsRectangle r( cMapParams.mExtent );
        if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) &&
             mapSettings.destinationCrs().hasAxisInverted() )
        {
          r.invert();
        }
        map->setExtent( r );

        // scale
        if ( cMapParams.mScale > 0 )
        {
          map->setScale( cMapParams.mScale );
        }

        // rotation
        if ( cMapParams.mRotation )
        {
          map->setMapRotation( cMapParams.mRotation );
        }
      }

      if ( !map->keepLayerSet() )
      {
        if ( cMapParams.mLayers.isEmpty() && cMapParams.mExternalLayers.isEmpty() )
        {
          map->setLayers( mapSettings.layers() );
        }
        else
        {
          QList<QgsMapLayer *> layerSet = stylizedLayers( cMapParams.mLayers );
          layerSet << externalLayers( cMapParams.mExternalLayers );
          layerSet << highlightLayers( cMapParams.mHighlightLayers );
          std::reverse( layerSet.begin(), layerSet.end() );
          map->setLayers( layerSet );
        }
        map->setKeepLayerSet( true );
      }

      //grid space x / y
      if ( cMapParams.mGridX > 0 && cMapParams.mGridY > 0 )
      {
        map->grid()->setIntervalX( cMapParams.mGridX );
        map->grid()->setIntervalY( cMapParams.mGridY );
      }
    }

    // Labels
    QList<QgsLayoutItemLabel *> labels;
    c->layoutItems<QgsLayoutItemLabel>( labels );
    for ( const auto &label : qgis::as_const( labels ) )
    {
      bool ok = false;
      const QString labelId = label->id();
      const QString labelParam = mWmsParameters.layoutParameter( labelId, ok );

      if ( !ok )
        continue;

      if ( labelParam.isEmpty() )
      {
        //remove exported labels referenced in the request
        //but with empty string
        c->removeItem( label );
        delete label;
        continue;
      }

      label->setText( labelParam );
    }

    // HTMLs
    QList<QgsLayoutItemHtml *> htmls;
    c->layoutObjects<QgsLayoutItemHtml>( htmls );
    for ( const auto &html : qgis::as_const( htmls ) )
    {
      if ( html->frameCount() == 0 )
        continue;

      QgsLayoutFrame *htmlFrame = html->frame( 0 );
      bool ok = false;
      const QString htmlId = htmlFrame->id();
      const QString url = mWmsParameters.layoutParameter( htmlId, ok );

      if ( !ok )
      {
        html->update();
        continue;
      }

      //remove exported Htmls referenced in the request
      //but with empty string
      if ( url.isEmpty() )
      {
        c->removeMultiFrame( html );
        delete html;
        continue;
      }

      QUrl newUrl( url );
      html->setUrl( newUrl );
      html->update();
    }


    // legends
    QList<QgsLayoutItemLegend *> legends;
    c->layoutItems<QgsLayoutItemLegend>( legends );
    for ( const auto &legend : qgis::as_const( legends ) )
    {
      if ( legend->autoUpdateModel() )
      {
        // the legend has an auto-update model
        // we will update it with map's layers
        const QgsLayoutItemMap *map = legend->linkedMap();
        if ( !map )
        {
          continue;
        }

        legend->setAutoUpdateModel( false );

        // get model and layer tree root of the legend
        QgsLegendModel *model = legend->model();
        QStringList layerSet;
        const QList<QgsMapLayer *> layerList( map->layers() );
        for ( const auto &layer : layerList )
          layerSet << layer->id();

        //setLayerIdsToLegendModel( model, layerSet, map->scale() );

        // get model and layer tree root of the legend
        QgsLayerTree *root = model->rootGroup();

        // get layerIds find in the layer tree root
        const QStringList layerIds = root->findLayerIds();

        // find the layer in the layer tree
        // remove it if the layer id is not in map layerIds
        for ( const auto &layerId : layerIds )
        {
          QgsLayerTreeLayer *nodeLayer = root->findLayer( layerId );
          if ( !nodeLayer )
          {
            continue;
          }
          if ( !layerSet.contains( layerId ) )
          {
            qobject_cast<QgsLayerTreeGroup *>( nodeLayer->parent() )->removeChildNode( nodeLayer );
          }
          else
          {
            QgsMapLayer *layer = nodeLayer->layer();
            if ( !layer->isInScaleRange( map->scale() ) )
            {
              qobject_cast<QgsLayerTreeGroup *>( nodeLayer->parent() )->removeChildNode( nodeLayer );
            }
          }
        }
        root->removeChildrenGroupWithoutLayers();
      }
    }
    return true;
  }

  QImage *QgsRenderer::getMap( HitTest *hitTest )
  {
    QgsMapSettings mapSettings;
    return getMap( mapSettings, hitTest );
  }

  QImage *QgsRenderer::getMap( QgsMapSettings &mapSettings, HitTest *hitTest )
  {
    // check size
    if ( !checkMaximumWidthHeight() )
    {
      throw QgsBadRequestException( QStringLiteral( "Size error" ),
                                    QStringLiteral( "The requested map size is too large" ) );
    }

    // get layers parameters
    QList<QgsMapLayer *> layers;
    QList<QgsWmsParametersLayer> params = mWmsParameters.layersParameters();

    // init layer restorer before doing anything
    std::unique_ptr<QgsLayerRestorer> restorer;
    restorer.reset( new QgsLayerRestorer( mNicknameLayers.values() ) );

    // init stylized layers according to LAYERS/STYLES or SLD
    QString sld = mWmsParameters.sldBody();
    if ( !sld.isEmpty() )
    {
      layers = sldStylizedLayers( sld );
    }
    else
    {
      layers = stylizedLayers( params );
    }

    // remove unwanted layers (restricted layers, ...)
    removeUnwantedLayers( layers );

    // configure each layer with opacity, selection filter, ...
    bool updateMapExtent = mWmsParameters.bbox().isEmpty();
    for ( QgsMapLayer *layer : layers )
    {
      checkLayerReadPermissions( layer );

      for ( const QgsWmsParametersLayer &param : params )
      {
        if ( param.mNickname == layerNickname( *layer ) )
        {
          setLayerOpacity( layer, param.mOpacity );

          setLayerFilter( layer, param.mFilter );

          setLayerSelection( layer, param.mSelection );

          if ( updateMapExtent )
            updateExtent( layer, mapSettings );

          break;
        }
      }

      setLayerAccessControlFilter( layer );
    }

    // add external layers
    layers = layers << externalLayers( mWmsParameters.externalLayersParameters() );

    // add highlight layers above others
    layers = layers << highlightLayers( mWmsParameters.highlightLayersParameters() );

    // create the output image and the painter
    std::unique_ptr<QPainter> painter;
    std::unique_ptr<QImage> image( createImage() );

    // configure map settings (background, DPI, ...)
    configureMapSettings( image.get(), mapSettings );

    // add layers to map settings (revert order for the rendering)
    std::reverse( layers.begin(), layers.end() );
    mapSettings.setLayers( layers );

    // rendering step for layers
    painter.reset( layersRendering( mapSettings, *image, hitTest ) );

    // rendering step for annotations
    annotationsRendering( painter.get() );

    // painting is terminated
    painter->end();

    // scale output image if necessary (required by WMS spec)
    QImage *scaledImage = scaleImage( image.get() );
    if ( scaledImage )
      image.reset( scaledImage );

    // return
    return image.release();
  }

  QgsDxfExport QgsRenderer::getDxf()
  {
    QgsDxfExport dxf;

    // set extent
    QgsRectangle extent = mWmsParameters.bboxAsRectangle();
    dxf.setExtent( extent );

    // get layers parameters
    QList<QgsMapLayer *> layers;
    QList<QgsWmsParametersLayer> params = mWmsParameters.layersParameters();

    // init layer restorer before doing anything
    std::unique_ptr<QgsLayerRestorer> restorer;
    restorer.reset( new QgsLayerRestorer( mNicknameLayers.values() ) );

    // init stylized layers according to LAYERS/STYLES or SLD
    QString sld = mWmsParameters.sldBody();
    if ( !sld.isEmpty() )
    {
      layers = sldStylizedLayers( sld );
    }
    else
    {
      layers = stylizedLayers( params );
    }

    // only wfs layers are allowed to be published
    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *mProject );

    // get dxf layers
    const QStringList attributes = mWmsParameters.dxfLayerAttributes();
    QList< QgsDxfExport::DxfLayer > dxfLayers;
    int layerIdx = -1;
    for ( QgsMapLayer *layer : layers )
    {
      layerIdx++;
      if ( layer->type() != QgsMapLayerType::VectorLayer )
        continue;
      if ( !wfsLayerIds.contains( layer->id() ) )
        continue;

      checkLayerReadPermissions( layer );

      for ( const QgsWmsParametersLayer &param : params )
      {
        if ( param.mNickname == layerNickname( *layer ) )
        {
          setLayerOpacity( layer, param.mOpacity );

          setLayerFilter( layer, param.mFilter );

          break;
        }
      }

      setLayerAccessControlFilter( layer );

      // cast for dxf layers
      QgsVectorLayer *vlayer = static_cast<QgsVectorLayer *>( layer );

      // get the layer attribute used in dxf
      int layerAttribute = -1;
      if ( attributes.size() > layerIdx )
      {
        layerAttribute = vlayer->fields().indexFromName( attributes[ layerIdx ] );
      }

      dxfLayers.append( QgsDxfExport::DxfLayer( vlayer, layerAttribute ) );
    }

    // add layers to dxf
    dxf.addLayers( dxfLayers );
    dxf.setLayerTitleAsName( mWmsParameters.dxfUseLayerTitleAsName() );
    dxf.setSymbologyExport( mWmsParameters.dxfMode() );
    if ( mWmsParameters.dxfFormatOptions().contains( QgsWmsParameters::DxfFormatOption::SCALE ) )
    {
      dxf.setSymbologyScale( mWmsParameters.dxfScale() );
    }

    return dxf;
  }

  static void infoPointToMapCoordinates( int i, int j, QgsPointXY *infoPoint, const QgsMapSettings &mapSettings )
  {
    //check if i, j are in the pixel range of the image
    if ( i < 0 || i > mapSettings.outputSize().width() || j < 0 || j > mapSettings.outputSize().height() )
    {
      throw QgsBadRequestException( "InvalidPoint", "I/J parameters not within the pixel range" );
    }

    double xRes = mapSettings.extent().width() / mapSettings.outputSize().width();
    double yRes = mapSettings.extent().height() / mapSettings.outputSize().height();
    infoPoint->setX( mapSettings.extent().xMinimum() + i * xRes + xRes / 2.0 );
    infoPoint->setY( mapSettings.extent().yMaximum() - j * yRes - yRes / 2.0 );
  }

  QByteArray QgsRenderer::getFeatureInfo( const QString &version )
  {
    // Verifying Mandatory parameters
    // The QUERY_LAYERS parameter is Mandatory
    QStringList queryLayers = mWmsParameters.queryLayersNickname();
    if ( queryLayers.isEmpty() )
    {
      QString msg = QObject::tr( "QUERY_LAYERS parameter is required for GetFeatureInfo" );
      throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ), msg );
    }

    // The I/J parameters are Mandatory if they are not replaced by X/Y or FILTER or FILTER_GEOM
    const bool ijDefined = !mWmsParameters.i().isEmpty() && !mWmsParameters.j().isEmpty();
    const bool xyDefined = !mWmsParameters.x().isEmpty() && !mWmsParameters.y().isEmpty();
    const bool filtersDefined = !mWmsParameters.filters().isEmpty();
    const bool filterGeomDefined = !mWmsParameters.filterGeom().isEmpty();

    if ( !ijDefined && !xyDefined && !filtersDefined && !filterGeomDefined )
    {
      throw QgsBadRequestException( QStringLiteral( "ParameterMissing" ),
                                    QStringLiteral( "I/J parameters are required for GetFeatureInfo" ) );
    }

    QgsWmsParameters::Format infoFormat = mWmsParameters.infoFormat();
    if ( infoFormat == QgsWmsParameters::Format::NONE )
    {
      throw QgsBadRequestException( QStringLiteral( "InvalidFormat" ),
                                    QStringLiteral( "Invalid INFO_FORMAT parameter" ) );
    }

    // get layers parameters
    QList<QgsMapLayer *> layers;
    QList<QgsWmsParametersLayer> params = mWmsParameters.layersParameters();

    // init layer restorer before doing anything
    std::unique_ptr<QgsLayerRestorer> restorer;
    restorer.reset( new QgsLayerRestorer( mNicknameLayers.values() ) );

    // init stylized layers according to LAYERS/STYLES or SLD
    QString sld = mWmsParameters.sldBody();
    if ( !sld.isEmpty() )
      layers = sldStylizedLayers( sld );
    else
      layers = stylizedLayers( params );

    // add QUERY_LAYERS to list of available layers for more flexibility
    for ( const QString &queryLayer : queryLayers )
    {
      if ( mNicknameLayers.contains( queryLayer )
           && !layers.contains( mNicknameLayers[queryLayer] ) )
      {
        layers.append( mNicknameLayers[queryLayer] );
      }
    }

    // create the mapSettings and the output image
    int imageWidth = mWmsParameters.widthAsInt();
    int imageHeight = mWmsParameters.heightAsInt();

    // Provide default image width/height values if format is not image
    if ( !( imageWidth && imageHeight ) &&  ! mWmsParameters.infoFormatIsImage() )
    {
      imageWidth = 10;
      imageHeight = 10;
    }

    QgsMapSettings mapSettings;
    std::unique_ptr<QImage> outputImage( createImage( imageWidth, imageHeight ) );

    // The CRS parameter is considered as mandatory in configureMapSettings
    // but in the case of filter parameter, CRS parameter has not to be mandatory
    bool mandatoryCrsParam = true;
    if ( filtersDefined && !ijDefined && !xyDefined && mWmsParameters.crs().isEmpty() )
    {
      mandatoryCrsParam = false;
    }

    // configure map settings (background, DPI, ...)
    configureMapSettings( outputImage.get(), mapSettings, mandatoryCrsParam );

    QgsMessageLog::logMessage( "mapSettings.destinationCrs(): " +  mapSettings.destinationCrs().authid() );
    QgsMessageLog::logMessage( "mapSettings.extent(): " +  mapSettings.extent().toString() );
    QgsMessageLog::logMessage( QStringLiteral( "mapSettings width = %1 height = %2" ).arg( mapSettings.outputSize().width() ).arg( mapSettings.outputSize().height() ) );
    QgsMessageLog::logMessage( QStringLiteral( "mapSettings.mapUnitsPerPixel() = %1" ).arg( mapSettings.mapUnitsPerPixel() ) );

    QgsScaleCalculator scaleCalc( ( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2, mapSettings.destinationCrs().mapUnits() );
    QgsRectangle mapExtent = mapSettings.extent();
    double scaleDenominator = scaleCalc.calculate( mapExtent, outputImage->width() );

    // remove unwanted layers (restricted layers, ...)
    removeUnwantedLayers( layers, scaleDenominator );
    // remove non identifiable layers
    //removeNonIdentifiableLayers( layers );

    for ( QgsMapLayer *layer : layers )
    {
      checkLayerReadPermissions( layer );

      for ( const QgsWmsParametersLayer &param : params )
      {
        if ( param.mNickname == layerNickname( *layer ) )
        {
          setLayerFilter( layer, param.mFilter );

          break;
        }
      }

      setLayerAccessControlFilter( layer );
    }

    // add layers to map settings (revert order for the rendering)
    std::reverse( layers.begin(), layers.end() );
    mapSettings.setLayers( layers );

    QDomDocument result = featureInfoDocument( layers, mapSettings, outputImage.get(), version );

    QByteArray ba;

    if ( infoFormat == QgsWmsParameters::Format::TEXT )
      ba = convertFeatureInfoToText( result );
    else if ( infoFormat == QgsWmsParameters::Format::HTML )
      ba = convertFeatureInfoToHtml( result );
    else if ( infoFormat == QgsWmsParameters::Format::JSON )
      ba = convertFeatureInfoToJson( layers, result );
    else
      ba = result.toByteArray();

    return ba;
  }

  QImage *QgsRenderer::createImage( int width, int height, bool useBbox ) const
  {
    if ( width < 0 )
      width = mWmsParameters.widthAsInt();

    if ( height < 0 )
      height = mWmsParameters.heightAsInt();

    //Adapt width / height if the aspect ratio does not correspond with the BBOX.
    //Required by WMS spec. 1.3.
    if ( useBbox && mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) )
    {
      QgsRectangle mapExtent = mWmsParameters.bboxAsRectangle();
      if ( !mWmsParameters.bbox().isEmpty() && mapExtent.isEmpty() )
      {
        throw QgsBadRequestException( QStringLiteral( "InvalidParameterValue" ),
                                      QStringLiteral( "Invalid BBOX parameter" ) );
      }

      QString crs = mWmsParameters.crs();
      if ( crs.compare( "CRS:84", Qt::CaseInsensitive ) == 0 )
      {
        crs = QString( "EPSG:4326" );
        mapExtent.invert();
      }
      QgsCoordinateReferenceSystem outputCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( outputCRS.hasAxisInverted() )
      {
        mapExtent.invert();
      }
      if ( !mapExtent.isEmpty() && height > 0 && width > 0 )
      {
        double mapWidthHeightRatio = mapExtent.width() / mapExtent.height();
        double imageWidthHeightRatio = static_cast<double>( width ) / static_cast<double>( height );
        if ( !qgsDoubleNear( mapWidthHeightRatio, imageWidthHeightRatio, 0.0001 ) )
        {
          // inspired by MapServer, mapdraw.c L115
          double cellsize = ( mapExtent.width() / static_cast<double>( width ) ) * 0.5 + ( mapExtent.height() / static_cast<double>( height ) ) * 0.5;
          width = mapExtent.width() / cellsize;
          height = mapExtent.height() / cellsize;
        }
      }
    }

    if ( width <= 0 || height <= 0 )
    {
      throw QgsException( QStringLiteral( "createImage: Invalid width / height parameters" ) );
    }

    std::unique_ptr<QImage> image;

    // use alpha channel only if necessary because it slows down performance
    QgsWmsParameters::Format format = mWmsParameters.format();
    bool transparent = mWmsParameters.transparentAsBool();

    if ( transparent && format != QgsWmsParameters::JPG )
    {
      image = qgis::make_unique<QImage>( width, height, QImage::Format_ARGB32_Premultiplied );
      image->fill( 0 );
    }
    else
    {
      image = qgis::make_unique<QImage>( width, height, QImage::Format_RGB32 );
      image->fill( mWmsParameters.backgroundColorAsColor() );
    }

    // Check that image was correctly created
    if ( image->isNull() )
    {
      throw QgsException( QStringLiteral( "createImage: image could not be created, check for out of memory conditions" ) );
    }

    //apply DPI parameter if present. This is an extension of Qgis Mapserver compared to WMS 1.3.
    //Because of backwards compatibility, this parameter is optional
    double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
    int dpm = 1 / OGC_PX_M;
    if ( !mWmsParameters.dpi().isEmpty() )
      dpm = mWmsParameters.dpiAsDouble() / 0.0254;

    image->setDotsPerMeterX( dpm );
    image->setDotsPerMeterY( dpm );
    return image.release();
  }

  void QgsRenderer::configureMapSettings( const QPaintDevice *paintDevice, QgsMapSettings &mapSettings, bool mandatoryCrsParam ) const
  {
    if ( !paintDevice )
    {
      throw QgsException( QStringLiteral( "configureMapSettings: no paint device" ) );
    }

    mapSettings.setOutputSize( QSize( paintDevice->width(), paintDevice->height() ) );
    mapSettings.setOutputDpi( paintDevice->logicalDpiX() );

    //map extent
    QgsRectangle mapExtent = mWmsParameters.bboxAsRectangle();
    if ( !mWmsParameters.bbox().isEmpty() && mapExtent.isEmpty() )
    {
      throw QgsBadRequestException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Invalid BBOX parameter" ) );
    }

    QString crs = mWmsParameters.crs();
    if ( crs.compare( "CRS:84", Qt::CaseInsensitive ) == 0 )
    {
      crs = QString( "EPSG:4326" );
      mapExtent.invert();
    }
    else if ( crs.isEmpty() && !mandatoryCrsParam )
    {
      crs = QString( "EPSG:4326" );
    }

    QgsCoordinateReferenceSystem outputCRS;

    //wms spec says that CRS parameter is mandatory.
    outputCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
    if ( !outputCRS.isValid() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, could not create output CRS from EPSG" ) );
      throw QgsBadRequestException( QStringLiteral( "InvalidCRS" ), QStringLiteral( "Could not create output CRS" ) );
    }

    //then set destinationCrs
    mapSettings.setDestinationCrs( outputCRS );

    // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
    if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) && outputCRS.hasAxisInverted() )
    {
      mapExtent.invert();
    }

    mapSettings.setExtent( mapExtent );

    /* Define the background color
     * Transparent or colored
     */
    QgsWmsParameters::Format format = mWmsParameters.format();
    bool transparent = mWmsParameters.transparentAsBool();
    QColor backgroundColor = mWmsParameters.backgroundColorAsColor();

    //set background color
    if ( transparent && format != QgsWmsParameters::JPG )
    {
      mapSettings.setBackgroundColor( QColor( 0, 0, 0, 0 ) );
    }
    else if ( backgroundColor.isValid() )
    {
      mapSettings.setBackgroundColor( backgroundColor );
    }

    // add context from project (global variables, ...)
    QgsExpressionContext context = mProject->createExpressionContext();
    context << QgsExpressionContextUtils::mapSettingsScope( mapSettings );
    mapSettings.setExpressionContext( context );

    // add labeling engine settings
    mapSettings.setLabelingEngineSettings( mProject->labelingEngineSettings() );

    // enable rendering optimization
    mapSettings.setFlag( QgsMapSettings::UseRenderingOptimization );

    // set selection color
    int myRed = mProject->readNumEntry( "Gui", "/SelectionColorRedPart", 255 );
    int myGreen = mProject->readNumEntry( "Gui", "/SelectionColorGreenPart", 255 );
    int myBlue = mProject->readNumEntry( "Gui", "/SelectionColorBluePart", 0 );
    int myAlpha = mProject->readNumEntry( "Gui", "/SelectionColorAlphaPart", 255 );
    mapSettings.setSelectionColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  }

  QDomDocument QgsRenderer::featureInfoDocument( QList<QgsMapLayer *> &layers, const QgsMapSettings &mapSettings,
      const QImage *outputImage, const QString &version ) const
  {
    QStringList queryLayers = mWmsParameters.queryLayersNickname();

    bool ijDefined = ( !mWmsParameters.i().isEmpty() && !mWmsParameters.j().isEmpty() );

    bool xyDefined = ( !mWmsParameters.x().isEmpty() && !mWmsParameters.y().isEmpty() );

    bool filtersDefined = !mWmsParameters.filters().isEmpty();

    bool filterGeomDefined = !mWmsParameters.filterGeom().isEmpty();

    int featureCount = mWmsParameters.featureCountAsInt();
    if ( featureCount < 1 )
    {
      featureCount = 1;
    }

    int i = mWmsParameters.iAsInt();
    int j = mWmsParameters.jAsInt();
    if ( xyDefined && !ijDefined )
    {
      i = mWmsParameters.xAsInt();
      j = mWmsParameters.yAsInt();
    }
    int width = mWmsParameters.widthAsInt();
    int height = mWmsParameters.heightAsInt();
    if ( ( i != -1 && j != -1 && width != 0 && height != 0 ) && ( width != outputImage->width() || height != outputImage->height() ) )
    {
      i *= ( outputImage->width() / static_cast<double>( width ) );
      j *= ( outputImage->height() / static_cast<double>( height ) );
    }

    // init search variables
    std::unique_ptr<QgsRectangle> featuresRect;
    std::unique_ptr<QgsGeometry> filterGeom;
    std::unique_ptr<QgsPointXY> infoPoint;

    if ( i != -1 && j != -1 )
    {
      infoPoint.reset( new QgsPointXY() );
      infoPointToMapCoordinates( i, j, infoPoint.get(), mapSettings );
    }
    else if ( filtersDefined )
    {
      featuresRect.reset( new QgsRectangle() );
    }
    else if ( filterGeomDefined )
    {
      filterGeom.reset( new QgsGeometry( QgsGeometry::fromWkt( mWmsParameters.filterGeom() ) ) );
    }

    QDomDocument result;

    QDomElement getFeatureInfoElement;
    QgsWmsParameters::Format infoFormat = mWmsParameters.infoFormat();
    if ( infoFormat == QgsWmsParameters::Format::GML )
    {
      getFeatureInfoElement = result.createElement( QStringLiteral( "wfs:FeatureCollection" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:wfs" ), QStringLiteral( "http://www.opengis.net/wfs" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QStringLiteral( "http://qgis.org/gml" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://qgis.org/gml" ) );
    }
    else
    {
      QString featureInfoElemName = QgsServerProjectUtils::wmsFeatureInfoDocumentElement( *mProject );
      if ( featureInfoElemName.isEmpty() )
      {
        featureInfoElemName = QStringLiteral( "GetFeatureInfoResponse" );
      }
      QString featureInfoElemNs = QgsServerProjectUtils::wmsFeatureInfoDocumentElementNs( *mProject );
      if ( featureInfoElemNs.isEmpty() )
      {
        getFeatureInfoElement = result.createElement( featureInfoElemName );
      }
      else
      {
        getFeatureInfoElement = result.createElementNS( featureInfoElemNs, featureInfoElemName );
      }
      //feature info schema
      QString featureInfoSchema = QgsServerProjectUtils::wmsFeatureInfoSchema( *mProject );
      if ( !featureInfoSchema.isEmpty() )
      {
        getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        getFeatureInfoElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), featureInfoSchema );
      }
    }
    result.appendChild( getFeatureInfoElement );

    //Render context is needed to determine feature visibility for vector layers
    QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mapSettings );

    bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *mProject );

    //layers can have assigned a different name for GetCapabilities
    QHash<QString, QString> layerAliasMap = QgsServerProjectUtils::wmsFeatureInfoLayerAliasMap( *mProject );

    for ( const QString &queryLayer : queryLayers )
    {
      bool validLayer = false;
      bool queryableLayer = true;
      for ( QgsMapLayer *layer : layers )
      {
        if ( queryLayer == layerNickname( *layer ) )
        {
          validLayer = true;
          queryableLayer = layer->flags().testFlag( QgsMapLayer::Identifiable );
          if ( !queryableLayer )
          {
            break;
          }

          QDomElement layerElement;
          if ( infoFormat == QgsWmsParameters::Format::GML )
          {
            layerElement = getFeatureInfoElement;
          }
          else
          {
            layerElement = result.createElement( QStringLiteral( "Layer" ) );
            QString layerName = queryLayer;

            //check if the layer is given a different name for GetFeatureInfo output
            QHash<QString, QString>::const_iterator layerAliasIt = layerAliasMap.find( layerName );
            if ( layerAliasIt != layerAliasMap.constEnd() )
            {
              layerName = layerAliasIt.value();
            }

            layerElement.setAttribute( QStringLiteral( "name" ), layerName );
            getFeatureInfoElement.appendChild( layerElement );
            if ( sia2045 ) //the name might not be unique after alias replacement
            {
              layerElement.setAttribute( QStringLiteral( "id" ), layer->id() );
            }
          }

          if ( layer->type() == QgsMapLayerType::VectorLayer )
          {
            QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
            if ( vectorLayer )
            {
              ( void )featureInfoFromVectorLayer( vectorLayer, infoPoint.get(), featureCount, result, layerElement, mapSettings, renderContext, version, featuresRect.get(), filterGeom.get() );
              break;
            }
          }
          else
          {
            QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
            if ( !rasterLayer )
            {
              break;
            }
            if ( !infoPoint )
            {
              break;
            }
            QgsPointXY layerInfoPoint = mapSettings.mapToLayerCoordinates( layer, *( infoPoint.get() ) );
            if ( !rasterLayer->extent().contains( layerInfoPoint ) )
            {
              break;
            }
            if ( infoFormat == QgsWmsParameters::Format::GML )
            {
              layerElement = result.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );
              getFeatureInfoElement.appendChild( layerElement );
            }

            ( void )featureInfoFromRasterLayer( rasterLayer, mapSettings, &layerInfoPoint, result, layerElement, version );
          }
          break;
        }
      }
      if ( !validLayer && !mNicknameLayers.contains( queryLayer ) && !mLayerGroups.contains( queryLayer ) )
      {
        QString msg = QObject::tr( "Layer '%1' not found" ).arg( queryLayer );
        throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ), msg );
      }
      else if ( ( validLayer && !queryableLayer ) || ( !validLayer && mLayerGroups.contains( queryLayer ) ) )
      {
        QString msg = QObject::tr( "Layer '%1' is not queryable" ).arg( queryLayer );
        throw QgsBadRequestException( QStringLiteral( "LayerNotQueryable" ), msg );
      }
    }

    if ( featuresRect )
    {
      if ( infoFormat == QgsWmsParameters::Format::GML )
      {
        QDomElement bBoxElem = result.createElement( QStringLiteral( "gml:boundedBy" ) );
        QDomElement boxElem;
        int gmlVersion = mWmsParameters.infoFormatVersion();
        if ( gmlVersion < 3 )
        {
          boxElem = QgsOgcUtils::rectangleToGMLBox( featuresRect.get(), result, 8 );
        }
        else
        {
          boxElem = QgsOgcUtils::rectangleToGMLEnvelope( featuresRect.get(), result, 8 );
        }

        QgsCoordinateReferenceSystem crs = mapSettings.destinationCrs();
        if ( crs.isValid() )
        {
          boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        bBoxElem.appendChild( boxElem );
        getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
      }
      else
      {
        QDomElement bBoxElem = result.createElement( QStringLiteral( "BoundingBox" ) );
        bBoxElem.setAttribute( QStringLiteral( "CRS" ), mapSettings.destinationCrs().authid() );
        bBoxElem.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( featuresRect->xMinimum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( featuresRect->xMaximum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( featuresRect->yMinimum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( featuresRect->yMaximum(), 8 ) );
        getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
      }
    }

    if ( sia2045 && infoFormat == QgsWmsParameters::Format::XML )
    {
      convertFeatureInfoToSia2045( result );
    }

    return result;
  }

  bool QgsRenderer::featureInfoFromVectorLayer( QgsVectorLayer *layer,
      const QgsPointXY *infoPoint,
      int nFeatures,
      QDomDocument &infoDocument,
      QDomElement &layerElement,
      const QgsMapSettings &mapSettings,
      QgsRenderContext &renderContext,
      const QString &version,
      QgsRectangle *featureBBox,
      QgsGeometry *filterGeom ) const
  {
    if ( !layer )
    {
      return false;
    }

    QgsFeatureRequest fReq;

    // Transform filter geometry to layer CRS
    std::unique_ptr<QgsGeometry> layerFilterGeom;
    if ( filterGeom )
    {
      layerFilterGeom.reset( new QgsGeometry( *filterGeom ) );
      layerFilterGeom->transform( QgsCoordinateTransform( mapSettings.destinationCrs(), layer->crs(), fReq.transformContext() ) );
    }

    //we need a selection rect (0.01 of map width)
    QgsRectangle mapRect = mapSettings.extent();
    QgsRectangle layerRect = mapSettings.mapToLayerCoordinates( layer, mapRect );


    QgsRectangle searchRect;

    //info point could be 0 in case there is only an attribute filter
    if ( infoPoint )
    {
      searchRect = featureInfoSearchRect( layer, mapSettings, renderContext, *infoPoint );
    }
    else if ( layerFilterGeom )
    {
      searchRect = layerFilterGeom->boundingBox();
    }
    else if ( !mWmsParameters.bbox().isEmpty() )
    {
      searchRect = layerRect;
    }

    //do a select with searchRect and go through all the features

    QgsFeature feature;
    QgsAttributes featureAttributes;
    int featureCounter = 0;
    layer->updateFields();
    const QgsFields fields = layer->fields();
    bool addWktGeometry = ( QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) && mWmsParameters.withGeometry() );
    bool segmentizeWktGeometry = QgsServerProjectUtils::wmsFeatureInfoSegmentizeWktGeometry( *mProject );
    const QSet<QString> &excludedAttributes = layer->excludeAttributesWms();

    bool hasGeometry = addWktGeometry || featureBBox || layerFilterGeom;
    fReq.setFlags( ( ( hasGeometry ) ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) | QgsFeatureRequest::ExactIntersect );

    if ( ! searchRect.isEmpty() )
    {
      fReq.setFilterRect( searchRect );
    }
    else
    {
      fReq.setFlags( fReq.flags() & ~ QgsFeatureRequest::ExactIntersect );
    }


    if ( layerFilterGeom )
    {
      fReq.setFilterExpression( QString( "intersects( $geometry, geom_from_wkt('%1') )" ).arg( layerFilterGeom->asWkt() ) );
    }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mAccessControl->filterFeatures( layer, fReq );

    QStringList attributes;
    for ( const QgsField &field : fields )
    {
      attributes.append( field.name() );
    }
    attributes = mAccessControl->layerAttributes( layer, attributes );
    fReq.setSubsetOfAttributes( attributes, layer->fields() );
#endif

    QgsFeatureIterator fit = layer->getFeatures( fReq );
    std::unique_ptr< QgsFeatureRenderer > r2( layer->renderer() ? layer->renderer()->clone() : nullptr );
    if ( r2 )
    {
      r2->startRender( renderContext, layer->fields() );
    }

    bool featureBBoxInitialized = false;
    while ( fit.nextFeature( feature ) )
    {
      if ( layer->wkbType() == QgsWkbTypes::NoGeometry && ! searchRect.isEmpty() )
      {
        break;
      }

      ++featureCounter;
      if ( featureCounter > nFeatures )
      {
        break;
      }

      renderContext.expressionContext().setFeature( feature );

      if ( layer->wkbType() != QgsWkbTypes::NoGeometry && ! searchRect.isEmpty() )
      {
        if ( !r2 )
        {
          continue;
        }

        //check if feature is rendered at all
        bool render = r2->willRenderFeature( feature, renderContext );
        if ( !render )
        {
          continue;
        }
      }

      QgsRectangle box;
      if ( layer->wkbType() != QgsWkbTypes::NoGeometry && hasGeometry )
      {
        box = mapSettings.layerExtentToOutputExtent( layer, feature.geometry().boundingBox() );
        if ( featureBBox ) //extend feature info bounding box if requested
        {
          if ( !featureBBoxInitialized && featureBBox->isEmpty() )
          {
            *featureBBox = box;
            featureBBoxInitialized = true;
          }
          else
          {
            featureBBox->combineExtentWith( box );
          }
        }
      }

      QgsCoordinateReferenceSystem outputCrs = layer->crs();
      if ( layer->crs() != mapSettings.destinationCrs() )
      {
        outputCrs = mapSettings.destinationCrs();
      }

      if ( mWmsParameters.infoFormat() == QgsWmsParameters::Format::GML )
      {
        bool withGeom = layer->wkbType() != QgsWkbTypes::NoGeometry && addWktGeometry;
        int gmlVersion = mWmsParameters.infoFormatVersion();
        QString typeName = layerNickname( *layer );
        QDomElement elem = createFeatureGML(
                             &feature, layer, infoDocument, outputCrs, mapSettings, typeName, withGeom, gmlVersion
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                             , &attributes
#endif
                           );
        QDomElement featureMemberElem = infoDocument.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );
        featureMemberElem.appendChild( elem );
        layerElement.appendChild( featureMemberElem );
        continue;
      }
      else
      {
        QDomElement featureElement = infoDocument.createElement( QStringLiteral( "Feature" ) );
        featureElement.setAttribute( QStringLiteral( "id" ), FID_TO_STRING( feature.id() ) );
        layerElement.appendChild( featureElement );

        //read all attribute values from the feature
        featureAttributes = feature.attributes();
        for ( int i = 0; i < featureAttributes.count(); ++i )
        {
          //skip attribute if it is explicitly excluded from WMS publication
          if ( excludedAttributes.contains( fields.at( i ).name() ) )
          {
            continue;
          }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          //skip attribute if it is excluded by access control
          if ( !attributes.contains( fields.at( i ).name() ) )
          {
            continue;
          }
#endif

          //replace attribute name if there is an attribute alias?
          QString attributeName = layer->attributeDisplayName( i );

          QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          attributeElement.setAttribute( QStringLiteral( "name" ), attributeName );
          const QgsEditorWidgetSetup setup = layer->editorWidgetSetup( i );
          attributeElement.setAttribute( QStringLiteral( "value" ),
                                         QgsExpression::replaceExpressionText(
                                           replaceValueMapAndRelation(
                                             layer, i,
                                             featureAttributes[i] ),
                                           &renderContext.expressionContext() )
                                       );
          featureElement.appendChild( attributeElement );
        }

        //add maptip attribute based on html/expression (in case there is no maptip attribute)
        QString mapTip = layer->mapTipTemplate();
        if ( !mapTip.isEmpty() && mWmsParameters.withMapTip() )
        {
          QDomElement maptipElem = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          maptipElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "maptip" ) );
          maptipElem.setAttribute( QStringLiteral( "value" ),  QgsExpression::replaceExpressionText( mapTip, &renderContext.expressionContext() ) );
          featureElement.appendChild( maptipElem );
        }

        //append feature bounding box to feature info xml
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry && hasGeometry )
        {
          QDomElement bBoxElem = infoDocument.createElement( QStringLiteral( "BoundingBox" ) );
          bBoxElem.setAttribute( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS", outputCrs.authid() );
          bBoxElem.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( box.xMinimum(), wmsPrecision() ) );
          bBoxElem.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( box.xMaximum(), wmsPrecision() ) );
          bBoxElem.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( box.yMinimum(), wmsPrecision() ) );
          bBoxElem.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( box.yMaximum(), wmsPrecision() ) );
          featureElement.appendChild( bBoxElem );
        }

        //also append the wkt geometry as an attribute
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry && addWktGeometry && hasGeometry )
        {
          QgsGeometry geom = feature.geometry();
          if ( !geom.isNull() )
          {
            if ( layer->crs() != outputCrs )
            {
              QgsCoordinateTransform transform = mapSettings.layerTransform( layer );
              if ( transform.isValid() )
                geom.transform( transform );
            }

            if ( segmentizeWktGeometry )
            {
              const QgsAbstractGeometry *abstractGeom = geom.constGet();
              if ( abstractGeom )
              {
                if ( QgsWkbTypes::isCurvedType( abstractGeom->wkbType() ) )
                {
                  QgsAbstractGeometry *segmentizedGeom = abstractGeom->segmentize();
                  geom.set( segmentizedGeom );
                }
              }
            }
            QDomElement geometryElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
            geometryElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "geometry" ) );
            geometryElement.setAttribute( QStringLiteral( "value" ), geom.asWkt( wmsPrecision() ) );
            geometryElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "derived" ) );
            featureElement.appendChild( geometryElement );
          }
        }
      }
    }
    if ( r2 )
    {
      r2->stopRender( renderContext );
    }

    return true;
  }

  bool QgsRenderer::featureInfoFromRasterLayer( QgsRasterLayer *layer,
      const QgsMapSettings &mapSettings,
      const QgsPointXY *infoPoint,
      QDomDocument &infoDocument,
      QDomElement &layerElement,
      const QString &version ) const
  {
    Q_UNUSED( version );

    if ( !infoPoint || !layer || !layer->dataProvider() )
    {
      return false;
    }

    QgsMessageLog::logMessage( QStringLiteral( "infoPoint: %1 %2" ).arg( infoPoint->x() ).arg( infoPoint->y() ) );

    if ( !( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyValue ) )
    {
      return false;
    }

    QgsRasterIdentifyResult identifyResult;
    // use context extent, width height (comes with request) to use WCS cache
    // We can only use context if raster is not reprojected, otherwise it is difficult
    // to guess correct source resolution
    if ( layer->dataProvider()->crs() != mapSettings.destinationCrs() )
    {
      identifyResult = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue );
    }
    else
    {
      identifyResult = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue, mapSettings.extent(), mapSettings.outputSize().width(), mapSettings.outputSize().height() );
    }

    if ( !identifyResult.isValid() )
      return false;

    QMap<int, QVariant> attributes = identifyResult.results();

    if ( mWmsParameters.infoFormat() == QgsWmsParameters::Format::GML )
    {
      QgsFeature feature;
      QgsFields fields;
      feature.initAttributes( attributes.count() );
      int index = 0;
      for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
      {
        fields.append( QgsField( layer->bandName( it.key() ), QVariant::Double ) );
        feature.setAttribute( index++, QString::number( it.value().toDouble() ) );
      }
      feature.setFields( fields );

      QgsCoordinateReferenceSystem layerCrs = layer->crs();
      int gmlVersion = mWmsParameters.infoFormatVersion();
      QString typeName = layerNickname( *layer );
      QDomElement elem = createFeatureGML(
                           &feature, nullptr, infoDocument, layerCrs, mapSettings, typeName, false, gmlVersion, nullptr );
      layerElement.appendChild( elem );
    }
    else
    {
      for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
      {
        QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
        attributeElement.setAttribute( QStringLiteral( "name" ), layer->bandName( it.key() ) );
        attributeElement.setAttribute( QStringLiteral( "value" ), QString::number( it.value().toDouble() ) );
        layerElement.appendChild( attributeElement );
      }
    }
    return true;
  }

  bool QgsRenderer::testFilterStringSafety( const QString &filter ) const
  {
    //; too dangerous for sql injections
    if ( filter.contains( QLatin1String( ";" ) ) )
    {
      return false;
    }

    QStringList tokens = filter.split( ' ', QString::SkipEmptyParts );
    groupStringList( tokens, QStringLiteral( "'" ) );
    groupStringList( tokens, QStringLiteral( "\"" ) );

    for ( auto tokenIt = tokens.constBegin() ; tokenIt != tokens.constEnd(); ++tokenIt )
    {
      //whitelist of allowed characters and keywords
      if ( tokenIt->compare( QLatin1String( "," ) ) == 0
           || tokenIt->compare( QLatin1String( "(" ) ) == 0
           || tokenIt->compare( QLatin1String( ")" ) ) == 0
           || tokenIt->compare( QLatin1String( "=" ) ) == 0
           || tokenIt->compare( QLatin1String( "!=" ) ) == 0
           || tokenIt->compare( QLatin1String( "<" ) ) == 0
           || tokenIt->compare( QLatin1String( "<=" ) ) == 0
           || tokenIt->compare( QLatin1String( ">" ) ) == 0
           || tokenIt->compare( QLatin1String( ">=" ) ) == 0
           || tokenIt->compare( QLatin1String( "%" ) ) == 0
           || tokenIt->compare( QLatin1String( "AND" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "OR" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "IN" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "LIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "ILIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "DMETAPHONE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "SOUNDEX" ), Qt::CaseInsensitive ) == 0 )
      {
        continue;
      }

      //numbers are OK
      bool isNumeric;
      tokenIt->toDouble( &isNumeric );
      if ( isNumeric )
      {
        continue;
      }

      //numeric strings need to be quoted once either with single or with double quotes

      //empty strings are OK
      if ( *tokenIt == QLatin1String( "''" ) )
      {
        continue;
      }

      //single quote
      if ( tokenIt->size() > 2
           && ( *tokenIt )[0] == QChar( '\'' )
           && ( *tokenIt )[tokenIt->size() - 1] == QChar( '\'' )
           && ( *tokenIt )[1] != QChar( '\'' )
           && ( *tokenIt )[tokenIt->size() - 2] != QChar( '\'' ) )
      {
        continue;
      }

      //double quote
      if ( tokenIt->size() > 2
           && ( *tokenIt )[0] == QChar( '"' )
           && ( *tokenIt )[tokenIt->size() - 1] == QChar( '"' )
           && ( *tokenIt )[1] != QChar( '"' )
           && ( *tokenIt )[tokenIt->size() - 2] != QChar( '"' ) )
      {
        continue;
      }

      return false;
    }

    return true;
  }

  void QgsRenderer::groupStringList( QStringList &list, const QString &groupString )
  {
    //group contents within single quotes together
    bool groupActive = false;
    int startGroup = -1;
    QString concatString;

    for ( int i = 0; i < list.size(); ++i )
    {
      QString &str = list[i];
      if ( str.startsWith( groupString ) )
      {
        startGroup = i;
        groupActive = true;
        concatString.clear();
      }

      if ( groupActive )
      {
        if ( i != startGroup )
        {
          concatString.append( " " );
        }
        concatString.append( str );
      }

      if ( str.endsWith( groupString ) )
      {
        int endGroup = i;
        groupActive = false;

        if ( startGroup != -1 )
        {
          list[startGroup] = concatString;
          for ( int j = startGroup + 1; j <= endGroup; ++j )
          {
            list.removeAt( startGroup + 1 );
            --i;
          }
        }

        concatString.clear();
        startGroup = -1;
      }
    }
  }

  bool QgsRenderer::checkMaximumWidthHeight() const
  {
    //test if maxWidth / maxHeight set and WIDTH / HEIGHT parameter is in the range
    int wmsMaxWidth = QgsServerProjectUtils::wmsMaxWidth( *mProject );
    int width = mWmsParameters.widthAsInt();
    if ( wmsMaxWidth != -1 && width > wmsMaxWidth )
    {
      return false;
    }

    int wmsMaxHeight = QgsServerProjectUtils::wmsMaxHeight( *mProject );
    int height = mWmsParameters.heightAsInt();
    if ( wmsMaxHeight != -1 && height > wmsMaxHeight )
    {
      return false;
    }

    // Sanity check from internal QImage checks (see qimage.cpp)
    // this is to report a meaningful error message in case of
    // image creation failure and to differentiate it from out
    // of memory conditions.

    // depth for now it cannot be anything other than 32, but I don't like
    // to hardcode it: I hope we will support other depths in the future.
    uint depth = 32;
    switch ( mWmsParameters.format() )
    {
      case QgsWmsParameters::Format::JPG:
      case QgsWmsParameters::Format::PNG:
      default:
        depth = 32;
    }

    const int bytes_per_line = ( ( width * depth + 31 ) >> 5 ) << 2; // bytes per scanline (must be multiple of 4)

    if ( std::numeric_limits<int>::max() / depth < static_cast<uint>( width )
         || bytes_per_line <= 0
         || height <= 0
         || std::numeric_limits<int>::max() / static_cast<uint>( bytes_per_line ) < static_cast<uint>( height )
         || std::numeric_limits<int>::max() / sizeof( uchar * ) < static_cast<uint>( height ) )
      return false;

    return true;
  }

  void QgsRenderer::convertFeatureInfoToSia2045( QDomDocument &doc ) const
  {
    QDomDocument SIAInfoDoc;
    QDomElement infoDocElement = doc.documentElement();
    QDomElement SIAInfoDocElement = SIAInfoDoc.importNode( infoDocElement, false ).toElement();
    SIAInfoDoc.appendChild( SIAInfoDocElement );

    QString currentAttributeName;
    QString currentAttributeValue;
    QDomElement currentAttributeElem;
    QString currentLayerName;
    QDomElement currentLayerElem;
    QDomNodeList layerNodeList = infoDocElement.elementsByTagName( QStringLiteral( "Layer" ) );
    for ( int i = 0; i < layerNodeList.size(); ++i )
    {
      currentLayerElem = layerNodeList.at( i ).toElement();
      currentLayerName = currentLayerElem.attribute( QStringLiteral( "name" ) );

      QDomElement currentFeatureElem;

      QDomNodeList featureList = currentLayerElem.elementsByTagName( QStringLiteral( "Feature" ) );
      if ( featureList.isEmpty() )
      {
        //raster?
        QDomNodeList attributeList = currentLayerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        QDomElement rasterLayerElem;
        if ( !attributeList.isEmpty() )
        {
          rasterLayerElem = SIAInfoDoc.createElement( currentLayerName );
        }
        for ( int j = 0; j < attributeList.size(); ++j )
        {
          currentAttributeElem = attributeList.at( j ).toElement();
          currentAttributeName = currentAttributeElem.attribute( QStringLiteral( "name" ) );
          currentAttributeValue = currentAttributeElem.attribute( QStringLiteral( "value" ) );
          QDomElement outAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
          QDomText outAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
          outAttributeElem.appendChild( outAttributeText );
          rasterLayerElem.appendChild( outAttributeElem );
        }
        if ( !attributeList.isEmpty() )
        {
          SIAInfoDocElement.appendChild( rasterLayerElem );
        }
      }
      else //vector
      {
        //property attributes
        QSet<QString> layerPropertyAttributes;
        QString currentLayerId = currentLayerElem.attribute( QStringLiteral( "id" ) );
        if ( !currentLayerId.isEmpty() )
        {
          QgsMapLayer *currentLayer = mProject->mapLayer( currentLayerId );
          if ( currentLayer )
          {
            QString WMSPropertyAttributesString = currentLayer->customProperty( QStringLiteral( "WMSPropertyAttributes" ) ).toString();
            if ( !WMSPropertyAttributesString.isEmpty() )
            {
              QStringList propertyList = WMSPropertyAttributesString.split( QStringLiteral( "//" ) );
              for ( auto propertyIt = propertyList.constBegin() ; propertyIt != propertyList.constEnd(); ++propertyIt )
              {
                layerPropertyAttributes.insert( *propertyIt );
              }
            }
          }
        }

        QDomElement propertyRefChild; //child to insert the next property after (or
        for ( int j = 0; j < featureList.size(); ++j )
        {
          QDomElement SIAFeatureElem = SIAInfoDoc.createElement( currentLayerName );
          currentFeatureElem = featureList.at( j ).toElement();
          QDomNodeList attributeList = currentFeatureElem.elementsByTagName( QStringLiteral( "Attribute" ) );

          for ( int k = 0; k < attributeList.size(); ++k )
          {
            currentAttributeElem = attributeList.at( k ).toElement();
            currentAttributeName = currentAttributeElem.attribute( QStringLiteral( "name" ) );
            currentAttributeValue = currentAttributeElem.attribute( QStringLiteral( "value" ) );
            if ( layerPropertyAttributes.contains( currentAttributeName ) )
            {
              QDomElement propertyElem = SIAInfoDoc.createElement( QStringLiteral( "property" ) );
              QDomElement identifierElem = SIAInfoDoc.createElement( QStringLiteral( "identifier" ) );
              QDomText identifierText = SIAInfoDoc.createTextNode( currentAttributeName );
              identifierElem.appendChild( identifierText );
              QDomElement valueElem = SIAInfoDoc.createElement( QStringLiteral( "value" ) );
              QDomText valueText = SIAInfoDoc.createTextNode( currentAttributeValue );
              valueElem.appendChild( valueText );
              propertyElem.appendChild( identifierElem );
              propertyElem.appendChild( valueElem );
              if ( propertyRefChild.isNull() )
              {
                SIAFeatureElem.insertBefore( propertyElem, QDomNode() );
                propertyRefChild = propertyElem;
              }
              else
              {
                SIAFeatureElem.insertAfter( propertyElem, propertyRefChild );
              }
            }
            else
            {
              QDomElement SIAAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
              QDomText SIAAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
              SIAAttributeElem.appendChild( SIAAttributeText );
              SIAFeatureElem.appendChild( SIAAttributeElem );
            }
          }
          SIAInfoDocElement.appendChild( SIAFeatureElem );
        }
      }
    }
    doc = SIAInfoDoc;
  }

  QByteArray QgsRenderer::convertFeatureInfoToHtml( const QDomDocument &doc ) const
  {
    QString featureInfoString;

    //the HTML head
    featureInfoString.append( "<HEAD>\n" );
    featureInfoString.append( "<TITLE> GetFeatureInfo results </TITLE>\n" );
    featureInfoString.append( "<META http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/>\n" );
    featureInfoString.append( "</HEAD>\n" );

    //start the html body
    featureInfoString.append( "<BODY>\n" );

    QDomNodeList layerList = doc.elementsByTagName( QStringLiteral( "Layer" ) );

    //layer loop
    for ( int i = 0; i < layerList.size(); ++i )
    {
      QDomElement layerElem = layerList.at( i ).toElement();

      featureInfoString.append( "<TABLE border=1 width=100%>\n" );
      featureInfoString.append( "<TR><TH width=25%>Layer</TH><TD>" + layerElem.attribute( QStringLiteral( "name" ) ) + "</TD></TR>\n" );
      featureInfoString.append( "</BR>" );

      //feature loop (for vector layers)
      QDomNodeList featureNodeList = layerElem.elementsByTagName( QStringLiteral( "Feature" ) );
      QDomElement currentFeatureElement;

      if ( !featureNodeList.isEmpty() ) //vector layer
      {
        for ( int j = 0; j < featureNodeList.size(); ++j )
        {
          QDomElement featureElement = featureNodeList.at( j ).toElement();
          featureInfoString.append( "<TABLE border=1 width=100%>\n" );
          featureInfoString.append( "<TR><TH>Feature</TH><TD>" + featureElement.attribute( QStringLiteral( "id" ) ) +
                                    "</TD></TR>\n" );

          //attribute loop
          QDomNodeList attributeNodeList = featureElement.elementsByTagName( QStringLiteral( "Attribute" ) );
          for ( int k = 0; k < attributeNodeList.size(); ++k )
          {
            QDomElement attributeElement = attributeNodeList.at( k ).toElement();
            featureInfoString.append( "<TR><TH>" + attributeElement.attribute( QStringLiteral( "name" ) ) +
                                      "</TH><TD>" + attributeElement.attribute( QStringLiteral( "value" ) ) + "</TD></TR>\n" );
          }

          featureInfoString.append( "</TABLE>\n</BR>\n" );
        }
      }
      else //raster layer
      {
        QDomNodeList attributeNodeList = layerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        for ( int j = 0; j < attributeNodeList.size(); ++j )
        {
          QDomElement attributeElement = attributeNodeList.at( j ).toElement();
          featureInfoString.append( "<TR><TH>" + attributeElement.attribute( QStringLiteral( "name" ) ) +
                                    "</TH><TD>" + attributeElement.attribute( QStringLiteral( "value" ) ) + "</TD></TR>\n" );
        }
      }

      featureInfoString.append( "</TABLE>\n<BR></BR>\n" );
    }

    //start the html body
    featureInfoString.append( "</BODY>\n" );

    return featureInfoString.toUtf8();
  }

  QByteArray QgsRenderer::convertFeatureInfoToText( const QDomDocument &doc ) const
  {
    QString featureInfoString;

    //the Text head
    featureInfoString.append( "GetFeatureInfo results\n" );
    featureInfoString.append( "\n" );

    QDomNodeList layerList = doc.elementsByTagName( QStringLiteral( "Layer" ) );

    //layer loop
    for ( int i = 0; i < layerList.size(); ++i )
    {
      QDomElement layerElem = layerList.at( i ).toElement();

      featureInfoString.append( "Layer '" + layerElem.attribute( QStringLiteral( "name" ) ) + "'\n" );

      //feature loop (for vector layers)
      QDomNodeList featureNodeList = layerElem.elementsByTagName( QStringLiteral( "Feature" ) );
      QDomElement currentFeatureElement;

      if ( !featureNodeList.isEmpty() ) //vector layer
      {
        for ( int j = 0; j < featureNodeList.size(); ++j )
        {
          QDomElement featureElement = featureNodeList.at( j ).toElement();
          featureInfoString.append( "Feature " + featureElement.attribute( QStringLiteral( "id" ) ) + "\n" );

          //attribute loop
          QDomNodeList attributeNodeList = featureElement.elementsByTagName( QStringLiteral( "Attribute" ) );
          for ( int k = 0; k < attributeNodeList.size(); ++k )
          {
            QDomElement attributeElement = attributeNodeList.at( k ).toElement();
            featureInfoString.append( attributeElement.attribute( QStringLiteral( "name" ) ) + " = '" +
                                      attributeElement.attribute( QStringLiteral( "value" ) ) + "'\n" );
          }
        }
      }
      else //raster layer
      {
        QDomNodeList attributeNodeList = layerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        for ( int j = 0; j < attributeNodeList.size(); ++j )
        {
          QDomElement attributeElement = attributeNodeList.at( j ).toElement();
          featureInfoString.append( attributeElement.attribute( QStringLiteral( "name" ) ) + " = '" +
                                    attributeElement.attribute( QStringLiteral( "value" ) ) + "'\n" );
        }
      }

      featureInfoString.append( "\n" );
    }

    return featureInfoString.toUtf8();
  }

  QByteArray QgsRenderer::convertFeatureInfoToJson( const QList<QgsMapLayer *> &layers, const QDomDocument &doc ) const
  {
    QString json;
    json.append( QStringLiteral( "{\"type\": \"FeatureCollection\",\n" ) );
    json.append( QStringLiteral( "    \"features\":[\n" ) );

    const bool withGeometry = ( QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) && mWmsParameters.withGeometry() );

    const QDomNodeList layerList = doc.elementsByTagName( QStringLiteral( "Layer" ) );
    for ( int i = 0; i < layerList.size(); ++i )
    {
      const QDomElement layerElem = layerList.at( i ).toElement();
      const QString layerName = layerElem.attribute( QStringLiteral( "name" ) );

      QgsMapLayer *layer = nullptr;
      for ( QgsMapLayer *l : layers )
      {
        if ( layerNickname( *l ).compare( layerName ) == 0 )
        {
          layer = l;
        }
      }

      if ( !layer )
        continue;

      if ( layer->type() == QgsMapLayerType::VectorLayer )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );

        // search features to export
        QgsFeatureList features;
        QgsAttributeList attributes;
        const QDomNodeList featuresNode = layerElem.elementsByTagName( QStringLiteral( "Feature" ) );
        if ( featuresNode.isEmpty() )
          continue;

        for ( int j = 0; j < featuresNode.size(); ++j )
        {
          const QDomElement featureNode = featuresNode.at( j ).toElement();
          const QgsFeatureId fid = featureNode.attribute( QStringLiteral( "id" ) ).toLongLong();
          QgsFeature feature = QgsFeature( vl->getFeature( fid ) );

          QString wkt;
          if ( withGeometry )
          {
            const QDomNodeList attrs = featureNode.elementsByTagName( "Attribute" );
            for ( int k = 0; k < attrs.count(); k++ )
            {
              const QDomElement elm = attrs.at( k ).toElement();
              if ( elm.attribute( QStringLiteral( "name" ) ).compare( "geometry" ) == 0 )
              {
                wkt = elm.attribute( "value" );
                break;
              }
            }

            if ( ! wkt.isEmpty() )
            {
              // CRS in WMS parameters may be different from the layer
              feature.setGeometry( QgsGeometry::fromWkt( wkt ) );
            }
          }
          features << feature;

          // search attributes to export (one time only)
          if ( !attributes.isEmpty() )
            continue;

          const QDomNodeList attributesNode = featureNode.elementsByTagName( QStringLiteral( "Attribute" ) );
          for ( int k = 0; k < attributesNode.size(); ++k )
          {
            const QDomElement attributeElement = attributesNode.at( k ).toElement();
            const QString fieldName = attributeElement.attribute( QStringLiteral( "name" ) );

            attributes << feature.fieldNameIndex( fieldName );
          }
        }

        // export
        QgsJsonExporter exporter( vl );
        exporter.setAttributeDisplayName( true );
        exporter.setAttributes( attributes );
        exporter.setIncludeGeometry( withGeometry );

        for ( const auto feature : features )
        {
          if ( json.right( 1 ).compare( QStringLiteral( "}" ) ) == 0 )
          {
            json.append( QStringLiteral( "," ) );
          }

          const QString id = QStringLiteral( "%1.%2" ).arg( layer->name(), QgsJsonUtils::encodeValue( feature.id() ) );
          json.append( exporter.exportFeature( feature, QVariantMap(), id ) );
        }
      }
      else // raster layer
      {
        json.append( QStringLiteral( "{" ) );
        json.append( QStringLiteral( "\"type\":\"Feature\",\n" ) );
        json.append( QStringLiteral( "\"id\":\"%1\",\n" ).arg( layer->name() ) );
        json.append( QStringLiteral( "\"properties\":{\n" ) );

        const QDomNodeList attributesNode = layerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        for ( int j = 0; j < attributesNode.size(); ++j )
        {
          const QDomElement attrElmt = attributesNode.at( j ).toElement();
          const QString name = attrElmt.attribute( QStringLiteral( "name" ) );
          const QString value = attrElmt.attribute( QStringLiteral( "value" ) );

          if ( j > 0 )
            json.append( QStringLiteral( ",\n" ) );

          json.append( QStringLiteral( "    \"%1\": \"%2\"" ).arg( name, value ) );
        }

        json.append( QStringLiteral( "\n}\n}" ) );
      }
    }

    json.append( QStringLiteral( "]}" ) );

    return json.toUtf8();
  }

  QDomElement QgsRenderer::createFeatureGML(
    QgsFeature *feat,
    QgsVectorLayer *layer,
    QDomDocument &doc,
    QgsCoordinateReferenceSystem &crs,
    const QgsMapSettings &mapSettings,
    const QString &typeName,
    bool withGeom,
    int version,
    QStringList *attributes ) const
  {
    //qgs:%TYPENAME%
    QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
    typeNameElement.setAttribute( QStringLiteral( "fid" ), typeName + "." + QString::number( feat->id() ) );

    QgsCoordinateTransform transform;
    if ( layer && layer->crs() != crs )
    {
      transform = mapSettings.layerTransform( layer );
    }

    QgsGeometry geom = feat->geometry();

    QgsExpressionContext expressionContext;
    expressionContext << QgsExpressionContextUtils::globalScope()
                      << QgsExpressionContextUtils::projectScope( mProject );
    if ( layer )
      expressionContext << QgsExpressionContextUtils::layerScope( layer );
    expressionContext.setFeature( *feat );

    // always add bounding box info if feature contains geometry
    if ( !geom.isNull() && geom.type() != QgsWkbTypes::UnknownGeometry && geom.type() != QgsWkbTypes::NullGeometry )
    {
      QgsRectangle box = feat->geometry().boundingBox();
      if ( transform.isValid() )
      {
        try
        {
          QgsRectangle transformedBox = transform.transformBoundingBox( box );
          box = transformedBox;
        }
        catch ( QgsCsException &e )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Transform error caught: %1" ).arg( e.what() ) );
        }
      }

      QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
      QDomElement boxElem;
      if ( version < 3 )
      {
        boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, wmsPrecision() );
      }
      else
      {
        boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, wmsPrecision() );
      }

      if ( crs.isValid() )
      {
        boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
      }
      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );
    }

    if ( withGeom && !geom.isNull() )
    {
      //add geometry column (as gml)

      if ( transform.isValid() )
      {
        geom.transform( transform );
      }

      QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
      QDomElement gmlElem;
      if ( version < 3 )
      {
        gmlElem = QgsOgcUtils::geometryToGML( geom, doc, wmsPrecision() );
      }
      else
      {
        gmlElem = QgsOgcUtils::geometryToGML( geom, doc, QStringLiteral( "GML3" ), wmsPrecision() );
      }

      if ( !gmlElem.isNull() )
      {
        if ( crs.isValid() )
        {
          gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        geomElem.appendChild( gmlElem );
        typeNameElement.appendChild( geomElem );
      }
    }

    //read all allowed attribute values from the feature
    QgsAttributes featureAttributes = feat->attributes();
    QgsFields fields = feat->fields();
    for ( int i = 0; i < fields.count(); ++i )
    {
      QString attributeName = fields.at( i ).name();
      //skip attribute if it is explicitly excluded from WMS publication
      if ( layer && layer->excludeAttributesWms().contains( attributeName ) )
      {
        continue;
      }
      //skip attribute if it is excluded by access control
      if ( attributes && !attributes->contains( attributeName ) )
      {
        continue;
      }

      QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( ' ', '_' ) );
      QString fieldTextString = featureAttributes.at( i ).toString();
      if ( layer )
      {
        fieldTextString = QgsExpression::replaceExpressionText( replaceValueMapAndRelation( layer, i, fieldTextString ), &expressionContext );
      }
      QDomText fieldText = doc.createTextNode( fieldTextString );
      fieldElem.appendChild( fieldText );
      typeNameElement.appendChild( fieldElem );
    }

    //add maptip attribute based on html/expression (in case there is no maptip attribute)
    if ( layer )
    {
      QString mapTip = layer->mapTipTemplate();

      if ( !mapTip.isEmpty() && mWmsParameters.withMapTip() )
      {
        QString fieldTextString = QgsExpression::replaceExpressionText( mapTip, &expressionContext );
        QDomElement fieldElem = doc.createElement( QStringLiteral( "qgs:maptip" ) );
        QDomText maptipText = doc.createTextNode( fieldTextString );
        fieldElem.appendChild( maptipText );
        typeNameElement.appendChild( fieldElem );
      }
    }

    return typeNameElement;
  }

  QString QgsRenderer::replaceValueMapAndRelation( QgsVectorLayer *vl, int idx, const QVariant &attributeVal )
  {
    const QgsEditorWidgetSetup setup = vl->editorWidgetSetup( idx );
    QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
    QString value( fieldFormatter->representValue( vl, idx, setup.config(), QVariant(), attributeVal ) );

    if ( setup.config().value( QStringLiteral( "AllowMulti" ) ).toBool() && value.startsWith( QLatin1String( "{" ) ) && value.endsWith( QLatin1String( "}" ) ) )
    {
      value = value.mid( 1, value.size() - 2 );
    }
    return value;
  }

  int QgsRenderer::imageQuality() const
  {
    // First taken from QGIS project
    int imageQuality = QgsServerProjectUtils::wmsImageQuality( *mProject );

    // Then checks if a parameter is given, if so use it instead
    if ( !mWmsParameters.imageQuality().isEmpty() )
    {
      imageQuality = mWmsParameters.imageQualityAsInt();
    }

    return imageQuality;
  }

  int QgsRenderer::wmsPrecision() const
  {
    // First taken from QGIS project and the default value is 6
    int WMSPrecision = QgsServerProjectUtils::wmsFeatureInfoPrecision( *mProject );

    // Then checks if a parameter is given, if so use it instead
    int WMSPrecisionParameter = mWmsParameters.wmsPrecisionAsInt();

    if ( WMSPrecisionParameter > -1 )
      return WMSPrecisionParameter;
    else
      return WMSPrecision;
  }

  QgsRectangle QgsRenderer::featureInfoSearchRect( QgsVectorLayer *ml, const QgsMapSettings &mapSettings, const QgsRenderContext &rct, const QgsPointXY &infoPoint ) const
  {
    if ( !ml )
    {
      return QgsRectangle();
    }

    double mapUnitTolerance = 0.0;
    if ( ml->geometryType() == QgsWkbTypes::PolygonGeometry )
    {
      if ( ! mWmsParameters.polygonTolerance().isEmpty()
           && mWmsParameters.polygonToleranceAsInt() > 0 )
      {
        mapUnitTolerance = mWmsParameters.polygonToleranceAsInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 400.0;
      }
    }
    else if ( ml->geometryType() == QgsWkbTypes::LineGeometry )
    {
      if ( ! mWmsParameters.lineTolerance().isEmpty()
           && mWmsParameters.lineToleranceAsInt() > 0 )
      {
        mapUnitTolerance = mWmsParameters.lineToleranceAsInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 200.0;
      }
    }
    else //points
    {
      if ( ! mWmsParameters.pointTolerance().isEmpty()
           && mWmsParameters.pointToleranceAsInt() > 0 )
      {
        mapUnitTolerance = mWmsParameters.pointToleranceAsInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 100.0;
      }
    }

    QgsRectangle mapRectangle( infoPoint.x() - mapUnitTolerance, infoPoint.y() - mapUnitTolerance,
                               infoPoint.x() + mapUnitTolerance, infoPoint.y() + mapUnitTolerance );
    return ( mapSettings.mapToLayerCoordinates( ml, mapRectangle ) );
  }


  void QgsRenderer::initRestrictedLayers()
  {
    mRestrictedLayers.clear();

    // get name of restricted layers/groups in project
    QStringList restricted = QgsServerProjectUtils::wmsRestrictedLayers( *mProject );

    // extract restricted layers from excluded groups
    QStringList restrictedLayersNames;
    QgsLayerTreeGroup *root = mProject->layerTreeRoot();

    for ( const QString &l : restricted )
    {
      QgsLayerTreeGroup *group = root->findGroup( l );
      if ( group )
      {
        QList<QgsLayerTreeLayer *> groupLayers = group->findLayers();
        for ( QgsLayerTreeLayer *treeLayer : groupLayers )
        {
          restrictedLayersNames.append( treeLayer->name() );
        }
      }
      else
      {
        restrictedLayersNames.append( l );
      }
    }

    // build output with names, ids or short name according to the configuration
    QList<QgsLayerTreeLayer *> layers = root->findLayers();
    for ( QgsLayerTreeLayer *layer : layers )
    {
      if ( restrictedLayersNames.contains( layer->name() ) )
      {
        mRestrictedLayers.append( layerNickname( *layer->layer() ) );
      }
    }
  }

  void QgsRenderer::initNicknameLayers()
  {
    for ( QgsMapLayer *ml : mProject->mapLayers() )
    {
      mNicknameLayers[ layerNickname( *ml ) ] = ml;
    }

    // init groups
    const QString rootName { QgsServerProjectUtils::wmsRootName( *mProject ) };
    const QgsLayerTreeGroup *root = mProject->layerTreeRoot();
    initLayerGroupsRecursive( root, rootName.isEmpty() ? mProject->title() : rootName );
  }

  void QgsRenderer::initLayerGroupsRecursive( const QgsLayerTreeGroup *group, const QString &groupName )
  {
    if ( !groupName.isEmpty() )
    {
      mLayerGroups[groupName] = QList<QgsMapLayer *>();
      for ( QgsLayerTreeLayer *layer : group->findLayers() )
      {
        mLayerGroups[groupName].append( layer->layer() );
      }
    }

    for ( const QgsLayerTreeNode *child : group->children() )
    {
      if ( child->nodeType() == QgsLayerTreeNode::NodeGroup )
      {
        QString name = child->customProperty( QStringLiteral( "wmsShortName" ) ).toString();

        if ( name.isEmpty() )
          name = child->name();

        initLayerGroupsRecursive( static_cast<const QgsLayerTreeGroup *>( child ), name );

      }
    }
  }

  QString QgsRenderer::layerNickname( const QgsMapLayer &layer ) const
  {
    QString name = layer.shortName();
    if ( QgsServerProjectUtils::wmsUseLayerIds( *mProject ) )
    {
      name = layer.id();
    }
    else if ( name.isEmpty() )
    {
      name = layer.name();
    }

    return name;
  }

  bool QgsRenderer::layerScaleVisibility( const QgsMapLayer &layer, double scaleDenominator ) const
  {
    bool visible = false;
    bool scaleBasedVisibility = layer.hasScaleBasedVisibility();
    bool useScaleConstraint = ( scaleDenominator > 0 && scaleBasedVisibility );

    if ( !useScaleConstraint || layer.isInScaleRange( scaleDenominator ) )
    {
      visible = true;
    }

    return visible;
  }

  QList<QgsMapLayer *> QgsRenderer::highlightLayers( QList<QgsWmsParametersHighlightLayer> params )
  {
    QList<QgsMapLayer *> highlightLayers;

    // try to create highlight layer for each geometry
    QString crs = mWmsParameters.crs();
    for ( const QgsWmsParametersHighlightLayer &param : params )
    {
      // create sld document from symbology
      QDomDocument sldDoc;
      if ( !sldDoc.setContent( param.mSld, true ) )
      {
        continue;
      }

      // create renderer from sld document
      QString errorMsg;
      std::unique_ptr<QgsFeatureRenderer> renderer;
      QDomElement el = sldDoc.documentElement();
      renderer.reset( QgsFeatureRenderer::loadSld( el, param.mGeom.type(), errorMsg ) );
      if ( !renderer )
      {
        QgsMessageLog::logMessage( errorMsg, "Server", Qgis::Info );
        continue;
      }

      // build url for vector layer
      const QString typeName = QgsWkbTypes::displayString( param.mGeom.wkbType() );
      QString url = typeName + "?crs=" + crs;
      if ( ! param.mLabel.isEmpty() )
      {
        url += "&field=label:string";
      }

      // create vector layer
      std::unique_ptr<QgsVectorLayer> layer;
      layer.reset( new QgsVectorLayer( url, param.mName, "memory" ) );
      if ( !layer->isValid() )
      {
        continue;
      }

      // create feature with label if necessary
      QgsFeature fet( layer->fields() );
      if ( ! param.mLabel.isEmpty() )
      {
        fet.setAttribute( 0, param.mLabel );

        // init labeling engine
        QgsPalLayerSettings palSettings;
        palSettings.fieldName = "label"; // defined in url
        palSettings.priority = 10; // always drawn
        palSettings.displayAll = true;

        QgsPalLayerSettings::Placement placement = QgsPalLayerSettings::AroundPoint;
        switch ( param.mGeom.type() )
        {
          case QgsWkbTypes::PointGeometry:
          {
            placement = QgsPalLayerSettings::AroundPoint;
            palSettings.dist = 2; // in mm
            palSettings.placementFlags = 0;
            break;
          }
          case QgsWkbTypes::PolygonGeometry:
          {
            QgsGeometry point = param.mGeom.pointOnSurface();
            QgsPointXY pt = point.asPoint();
            placement = QgsPalLayerSettings::AroundPoint;

            QgsPalLayerSettings::Property pX = QgsPalLayerSettings::PositionX;
            QVariant x( pt.x() );
            palSettings.dataDefinedProperties().setProperty( pX, x );

            QgsPalLayerSettings::Property pY = QgsPalLayerSettings::PositionY;
            QVariant y( pt.y() );
            palSettings.dataDefinedProperties().setProperty( pY, y );

            QgsPalLayerSettings::Property pHali = QgsPalLayerSettings::Hali;
            QVariant hali( "Center" );
            palSettings.dataDefinedProperties().setProperty( pHali, hali );

            QgsPalLayerSettings::Property pVali = QgsPalLayerSettings::Vali;
            QVariant vali( "Half" );
            palSettings.dataDefinedProperties().setProperty( pVali, vali );
            break;
          }
          default:
          {
            placement = QgsPalLayerSettings::Line;
            palSettings.dist = 2;
            palSettings.placementFlags = 10;
            break;
          }
        }
        palSettings.placement = placement;
        QgsTextFormat textFormat;
        QgsTextBufferSettings bufferSettings;

        if ( param.mColor.isValid() )
        {
          textFormat.setColor( param.mColor );
        }

        if ( param.mSize > 0 )
        {
          textFormat.setSize( param.mSize );
        }

        // no weight property in PAL settings or QgsTextFormat
        /* if ( param.fontWeight > 0 )
        {
        } */

        if ( ! param.mFont.isEmpty() )
        {
          textFormat.setFont( param.mFont );
        }

        if ( param.mBufferColor.isValid() )
        {
          bufferSettings.setColor( param.mBufferColor );
        }

        if ( param.mBufferSize > 0 )
        {
          bufferSettings.setEnabled( true );
          bufferSettings.setSize( param.mBufferSize );
        }

        textFormat.setBuffer( bufferSettings );
        palSettings.setFormat( textFormat );

        QgsVectorLayerSimpleLabeling *simpleLabeling = new QgsVectorLayerSimpleLabeling( palSettings );
        layer->setLabeling( simpleLabeling );
        layer->setLabelsEnabled( true );
      }
      fet.setGeometry( param.mGeom );

      // add feature to layer and set the SLD renderer
      layer->dataProvider()->addFeatures( QgsFeatureList() << fet );
      layer->setRenderer( renderer.release() );

      // keep the vector as an highlight layer
      if ( layer->isValid() )
      {
        highlightLayers.append( layer.release() );
      }
    }

    mTemporaryLayers.append( highlightLayers );
    return highlightLayers;
  }

  QList<QgsMapLayer *> QgsRenderer::externalLayers( const QList<QgsWmsParametersExternalLayer> &params )
  {
    QList<QgsMapLayer *> layers;

    for ( const QgsWmsParametersExternalLayer &param : params )
    {
      std::unique_ptr<QgsMapLayer> layer = qgis::make_unique< QgsRasterLayer >( param.mUri, param.mName, QStringLiteral( "wms" ) );

      if ( layer->isValid() )
      {
        // to delete later
        mTemporaryLayers.append( layer.release() );
        layers << mTemporaryLayers.last();
      }
    }

    return layers;
  }

  QList<QgsMapLayer *> QgsRenderer::sldStylizedLayers( const QString &sld ) const
  {
    QList<QgsMapLayer *> layers;

    if ( !sld.isEmpty() )
    {
      QDomDocument doc;
      ( void )doc.setContent( sld, true );
      QDomElement docEl = doc.documentElement();

      QDomElement root = doc.firstChildElement( "StyledLayerDescriptor" );
      QDomElement namedElem = root.firstChildElement( "NamedLayer" );

      if ( !docEl.isNull() )
      {
        QDomNodeList named = docEl.elementsByTagName( "NamedLayer" );
        for ( int i = 0; i < named.size(); ++i )
        {
          QDomNodeList names = named.item( i ).toElement().elementsByTagName( "Name" );
          if ( !names.isEmpty() )
          {
            QString lname = names.item( 0 ).toElement().text();
            QString err;
            if ( mNicknameLayers.contains( lname ) && !mRestrictedLayers.contains( lname ) )
            {
              mNicknameLayers[lname]->readSld( namedElem, err );
              mNicknameLayers[lname]->setCustomProperty( "readSLD", true );
              layers.append( mNicknameLayers[lname] );
            }
            else if ( mLayerGroups.contains( lname ) )
            {
              for ( QgsMapLayer *layer : mLayerGroups[lname] )
              {
                if ( !mRestrictedLayers.contains( layerNickname( *layer ) ) )
                {
                  layer->readSld( namedElem, err );
                  layer->setCustomProperty( "readSLD", true );
                  layers.insert( 0, layer );
                }
              }
            }
            else
            {
              throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ),
                                            QStringLiteral( "Layer \"%1\" does not exist" ).arg( lname ) );
            }
          }
        }
      }
    }

    return layers;
  }

  QList<QgsMapLayer *> QgsRenderer::stylizedLayers( const QList<QgsWmsParametersLayer> &params )
  {
    QList<QgsMapLayer *> layers;

    for ( const QgsWmsParametersLayer &param : params )
    {
      QString nickname = param.mNickname;
      QString style = param.mStyle;
      if ( mNicknameLayers.contains( nickname ) && !mRestrictedLayers.contains( nickname ) )
      {
        if ( !style.isEmpty() )
        {
          bool rc = mNicknameLayers[nickname]->styleManager()->setCurrentStyle( style );
          if ( ! rc )
          {
            throw QgsMapServiceException( QStringLiteral( "StyleNotDefined" ), QStringLiteral( "Style \"%1\" does not exist for layer \"%2\"" ).arg( style, nickname ) );
          }
        }

        layers.append( mNicknameLayers[nickname] );
      }
      else if ( mLayerGroups.contains( nickname ) )
      {
        // Reverse order of layers from a group
        QList<QgsMapLayer *> layersFromGroup;
        for ( QgsMapLayer *layer : mLayerGroups[nickname] )
        {
          if ( !mRestrictedLayers.contains( layerNickname( *layer ) ) )
          {
            if ( !style.isEmpty() )
            {
              bool rc = layer->styleManager()->setCurrentStyle( style );
              if ( ! rc )
              {
                throw QgsMapServiceException( QStringLiteral( "StyleNotDefined" ), QStringLiteral( "Style \"%1\" does not exist for layer \"%2\"" ).arg( style, layerNickname( *layer ) ) );
              }
            }
            layersFromGroup.push_front( layer );
          }
        }
        layers.append( layersFromGroup );
      }
      else
      {
        throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ),
                                      QStringLiteral( "Layer \"%1\" does not exist" ).arg( nickname ) );
      }
    }

    return layers;
  }

  void QgsRenderer::removeTemporaryLayers()
  {
    qDeleteAll( mTemporaryLayers );
    mTemporaryLayers.clear();
  }

  QPainter *QgsRenderer::layersRendering( const QgsMapSettings &mapSettings, QImage &image, HitTest *hitTest ) const
  {
    QPainter *painter = nullptr;
    if ( hitTest )
    {
      runHitTest( mapSettings, *hitTest );
      painter = new QPainter();
    }
    else
    {
      QgsFeatureFilterProviderGroup filters;
      filters.addProvider( &mFeatureFilter );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      mAccessControl->resolveFilterFeatures( mapSettings.layers() );
      filters.addProvider( mAccessControl );
#endif
      QgsMapRendererJobProxy renderJob( mSettings.parallelRendering(), mSettings.maxThreads(), &filters );
      renderJob.render( mapSettings, &image );
      painter = renderJob.takePainter();

      if ( !renderJob.errors().isEmpty() )
      {
        QString layerWMSName;
        QString firstErrorLayerId = renderJob.errors().at( 0 ).layerID;
        QgsMapLayer *errorLayer = mProject->mapLayer( firstErrorLayerId );
        if ( errorLayer )
        {
          layerWMSName = layerNickname( *errorLayer );
        }

        //Log first error
        QString errorMsg = QStringLiteral( "Map rendering error in layer '%1'" ).arg( firstErrorLayerId );
        QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), Qgis::Critical );
        throw QgsServerException( QStringLiteral( "Map rendering error in layer '%1'" ).arg( layerWMSName ) );
      }
    }

    return painter;
  }

  void QgsRenderer::setLayerOpacity( QgsMapLayer *layer, int opacity ) const
  {
    if ( opacity >= 0 && opacity <= 255 )
    {
      switch ( layer->type() )
      {
        case QgsMapLayerType::VectorLayer:
        {
          QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
          vl->setOpacity( opacity / 255. );
          break;
        }

        case QgsMapLayerType::RasterLayer:
        {
          QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer );
          QgsRasterRenderer *rasterRenderer = rl->renderer();
          rasterRenderer->setOpacity( opacity / 255. );
          break;
        }

        case QgsMapLayerType::MeshLayer:
        case QgsMapLayerType::PluginLayer:
          break;
      }
    }
  }

  void QgsRenderer::setLayerFilter( QgsMapLayer *layer, const QList<QgsWmsParametersFilter> &filters )
  {
    if ( layer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *filteredLayer = qobject_cast<QgsVectorLayer *>( layer );
      for ( const QgsWmsParametersFilter &filter : filters )
      {
        if ( filter.mType == QgsWmsParametersFilter::OGC_FE )
        {
          // OGC filter
          QDomDocument filterXml;
          QString errorMsg;
          if ( !filterXml.setContent( filter.mFilter, true, &errorMsg ) )
          {
            throw QgsBadRequestException( QStringLiteral( "Filter string rejected" ),
                                          QStringLiteral( "error message: %1. The XML string was: %2" ).arg( errorMsg, filter.mFilter ) );
          }
          QDomElement filterElem = filterXml.firstChildElement();
          std::unique_ptr<QgsExpression> expression( QgsOgcUtils::expressionFromOgcFilter( filterElem, filter.mVersion, filteredLayer ) );

          if ( expression )
          {
            mFeatureFilter.setFilter( filteredLayer, *expression );
          }
        }
        else if ( filter.mType == QgsWmsParametersFilter::SQL )
        {
          // QGIS (SQL) filter
          if ( !testFilterStringSafety( filter.mFilter ) )
          {
            throw QgsBadRequestException( QStringLiteral( "Filter string rejected" ),
                                          QStringLiteral( "The filter string %1"
                                              " has been rejected because of security reasons."
                                              " Note: Text strings have to be enclosed in single or double quotes."
                                              " A space between each word / special character is mandatory."
                                              " Allowed Keywords and special characters are "
                                              " AND,OR,IN,<,>=,>,>=,!=,',',(,),DMETAPHONE,SOUNDEX."
                                              " Not allowed are semicolons in the filter expression." ).arg(
                                            filter.mFilter ) );
          }

          QString newSubsetString = filter.mFilter;
          if ( !filteredLayer->subsetString().isEmpty() )
          {
            newSubsetString.prepend( ") AND (" );
            newSubsetString.append( ")" );
            newSubsetString.prepend( filteredLayer->subsetString() );
            newSubsetString.prepend( "(" );
          }
          filteredLayer->setSubsetString( newSubsetString );
        }
      }
    }
  }

  void QgsRenderer::setLayerSelection( QgsMapLayer *layer, const QStringList &fids ) const
  {
    if ( layer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsFeatureIds selectedIds;

      for ( const QString &id : fids )
      {
        selectedIds.insert( STRING_TO_FID( id ) );
      }

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
      vl->selectByIds( selectedIds );
    }
  }

  void QgsRenderer::setLayerAccessControlFilter( QgsMapLayer *layer ) const
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( mAccessControl, layer );
#else
    Q_UNUSED( layer );
#endif
  }

  void QgsRenderer::updateExtent( const QgsMapLayer *layer, QgsMapSettings &mapSettings ) const
  {
    QgsRectangle layerExtent = mapSettings.layerToMapCoordinates( layer, layer->extent() );
    QgsRectangle mapExtent = mapSettings.extent();
    if ( !layerExtent.isEmpty() )
    {
      mapExtent.combineExtentWith( layerExtent );
      mapSettings.setExtent( mapExtent );
    }
  }

  void QgsRenderer::annotationsRendering( QPainter *painter ) const
  {
    const QgsAnnotationManager *annotationManager = mProject->annotationManager();
    const QList< QgsAnnotation * > annotations = annotationManager->annotations();

    QgsRenderContext renderContext = QgsRenderContext::fromQPainter( painter );
    for ( QgsAnnotation *annotation : annotations )
    {
      if ( !annotation || !annotation->isVisible() )
        continue;

      annotation->render( renderContext );
    }
  }

  QImage *QgsRenderer::scaleImage( const QImage *image ) const
  {
    //test if width / height ratio of image is the same as the ratio of
    // WIDTH / HEIGHT parameters. If not, the image has to be scaled (required
    // by WMS spec)
    QImage *scaledImage = nullptr;
    int width = mWmsParameters.widthAsInt();
    int height = mWmsParameters.heightAsInt();
    if ( width != image->width() || height != image->height() )
    {
      scaledImage = new QImage( image->scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
    }

    return scaledImage;
  }

  void QgsRenderer::checkLayerReadPermissions( QgsMapLayer *layer ) const
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    if ( !mAccessControl->layerReadPermission( layer ) )
    {
      throw QgsSecurityException( QStringLiteral( "You are not allowed to access to the layer: %1" ).arg( layer->name() ) );
    }
#else
    Q_UNUSED( layer );
#endif
  }

  void QgsRenderer::removeUnwantedLayers( QList<QgsMapLayer *> &layers, double scaleDenominator ) const
  {
    QList<QgsMapLayer *> wantedLayers;

    for ( QgsMapLayer *layer : layers )
    {
      if ( !layerScaleVisibility( *layer, scaleDenominator ) )
        continue;

      if ( mRestrictedLayers.contains( layerNickname( *layer ) ) )
        continue;

      wantedLayers.append( layer );
    }

    layers = wantedLayers;
  }

  void QgsRenderer::removeNonIdentifiableLayers( QList<QgsMapLayer *> &layers ) const
  {
    QList<QgsMapLayer *>::iterator it = layers.begin();
    while ( it != layers.end() )
    {
      if ( !( *it )->flags().testFlag( QgsMapLayer::Identifiable ) )
        it = layers.erase( it );
      else
        ++it;
    }
  }

  QgsLayerTreeModel *QgsRenderer::buildLegendTreeModel( const QList<QgsMapLayer *> &layers, double scaleDenominator, QgsLayerTree &rootGroup )
  {
    // get params
    bool showFeatureCount = mWmsParameters.showFeatureCountAsBool();
    bool drawLegendLayerLabel = mWmsParameters.layerTitleAsBool();
    bool drawLegendItemLabel = mWmsParameters.ruleLabelAsBool();

    bool ruleDefined = false;
    if ( !mWmsParameters.rule().isEmpty() )
      ruleDefined = true;

    bool contentBasedLegend = false;
    QgsRectangle contentBasedLegendExtent;
    if ( ! mWmsParameters.bbox().isEmpty() )
    {
      contentBasedLegend = true;
      contentBasedLegendExtent = mWmsParameters.bboxAsRectangle();
      if ( contentBasedLegendExtent.isEmpty() )
        throw QgsBadRequestException( QStringLiteral( "InvalidParameterValue" ),
                                      QStringLiteral( "Invalid BBOX parameter" ) );

      if ( !mWmsParameters.rule().isEmpty() )
        throw QgsBadRequestException( QStringLiteral( "InvalidParameterValue" ),
                                      QStringLiteral( "BBOX parameter cannot be combined with RULE" ) );
    }

    // build layer tree
    rootGroup.clear();
    QList<QgsVectorLayerFeatureCounter *> counters;
    for ( QgsMapLayer *ml : layers )
    {
      QgsLayerTreeLayer *lt = rootGroup.addLayer( ml );
      lt->setCustomProperty( QStringLiteral( "showFeatureCount" ), showFeatureCount );

      if ( !ml->title().isEmpty() )
        lt->setName( ml->title() );

      if ( ml->type() != QgsMapLayerType::VectorLayer || !showFeatureCount )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
      QgsVectorLayerFeatureCounter *counter = vl->countSymbolFeatures();
      if ( !counter )
        continue;
      counters.append( counter );
    }

    // build legend model
    QgsLayerTreeModel *legendModel = new QgsLayerTreeModel( &rootGroup );
    if ( scaleDenominator > 0 )
      legendModel->setLegendFilterByScale( scaleDenominator );

    QgsMapSettings contentBasedMapSettings;
    if ( contentBasedLegend )
    {
      HitTest hitTest;
      getMap( contentBasedMapSettings, &hitTest );

      for ( QgsLayerTreeNode *node : rootGroup.children() )
      {
        Q_ASSERT( QgsLayerTree::isLayer( node ) );
        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() );
        if ( !vl || !vl->renderer() )
          continue;

        const SymbolSet &usedSymbols = hitTest[vl];
        QList<int> order;
        int i = 0;
        for ( const QgsLegendSymbolItem &legendItem : vl->renderer()->legendSymbolItems() )
        {
          QString sProp = QgsSymbolLayerUtils::symbolProperties( legendItem.legacyRuleKey() );
          if ( usedSymbols.contains( sProp ) )
            order.append( i );
          ++i;
        }

        // either remove the whole layer or just filter out some items
        if ( order.isEmpty() )
          rootGroup.removeChildNode( nodeLayer );
        else
        {
          QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
          legendModel->refreshLayerLegend( nodeLayer );
        }
      }
    }

    // if legend is not based on rendering rules
    if ( ! ruleDefined )
    {
      QList<QgsLayerTreeNode *> rootChildren = rootGroup.children();
      for ( QgsLayerTreeNode *node : rootChildren )
      {
        if ( QgsLayerTree::isLayer( node ) )
        {
          QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

          // layer titles - hidden or not
          QgsLegendRenderer::setNodeLegendStyle( nodeLayer, drawLegendLayerLabel ? QgsLegendStyle::Subgroup : QgsLegendStyle::Hidden );

          // rule item titles
          if ( !drawLegendItemLabel )
          {
            for ( QgsLayerTreeModelLegendNode *legendNode : legendModel->layerLegendNodes( nodeLayer ) )
            {
              legendNode->setUserLabel( QStringLiteral( " " ) ); // empty string = no override, so let's use one space
            }
          }
          else if ( !drawLegendLayerLabel )
          {
            for ( QgsLayerTreeModelLegendNode *legendNode : legendModel->layerLegendNodes( nodeLayer ) )
            {
              if ( legendNode->isEmbeddedInParent() )
                legendNode->setEmbeddedInParent( false );
            }
          }
        }
      }
    }

    for ( QgsVectorLayerFeatureCounter *c : counters )
    {
      c->waitForFinished();
    }

    return legendModel;
  }

  qreal QgsRenderer::dotsPerMm() const
  {
    std::unique_ptr<QImage> tmpImage( createImage( 1, 1, false ) );
    return tmpImage->dotsPerMeterX() / 1000.0;
  }

  void QgsRenderer::handlePrintErrors( const QgsLayout *layout ) const
  {
    if ( !layout )
    {
      return;
    }
    QList< QgsLayoutItemMap * > mapList;
    layout->layoutItems( mapList );

    QList< QgsLayoutItemMap * >::const_iterator mapIt = mapList.constBegin();
    for ( ; mapIt != mapList.constEnd(); ++mapIt )
    {
      if ( !( *mapIt )->renderingErrors().isEmpty() )
      {
        //Log first error
        QgsMapRendererJob::Error e = ( *mapIt )->renderingErrors().at( 0 );
        QString errorMsg = QString( "Rendering error : '%1' in layer %2" ).arg( e.message ).arg( e.layerID );
        QgsMessageLog::logMessage( errorMsg, "Server", Qgis::Critical );

        throw QgsServerException( QStringLiteral( "Print error" ) );
      }
    }
  }

} // namespace QgsWms
