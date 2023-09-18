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
#include "qgslayoututils.h"
#include "qgslayertreemodel.h"
#include "qgslegendrenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmapthemecollection.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsscalecalculator.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsmessagelog.h"
#include "qgsrenderer.h"
#include "qgsfeature.h"
#include "qgsaccesscontrol.h"
#include "qgsfeaturerequest.h"
#include "qgsmaprendererjobproxy.h"
#include "qgswmsserviceexception.h"
#include "qgsserverprojectutils.h"
#include "qgsserverfeatureid.h"
#include "qgsmaplayerstylemanager.h"
#include "qgswkbtypes.h"
#include "qgsannotationmanager.h"
#include "qgsannotation.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerfeaturecounter.h"
#include "qgspallabeling.h"
#include "qgswmsrestorer.h"
#include "qgsdxfexport.h"
#include "qgssymbollayerutils.h"
#include "qgsserverexception.h"
#include "qgsserverapiutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeaturestore.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorelement.h"
#include "qgsattributeeditorfield.h"
#include "qgsdimensionfilter.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTemporaryFile>
#include <QDir>
#include <QUrl>
#include <nlohmann/json.hpp>

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

namespace QgsWms
{
  QgsRenderer::QgsRenderer( const QgsWmsRenderContext &context )
    : mContext( context )
  {
    mProject = mContext.project();

    mWmsParameters = mContext.parameters();
    mWmsParameters.dump();
  }

  QgsRenderer::~QgsRenderer()
  {
    removeTemporaryLayers();
  }

  QImage *QgsRenderer::getLegendGraphics( QgsLayerTreeModel &model )
  {
    // get layers
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers );

    // init renderer
    QgsLegendSettings settings = legendSettings();
    QgsLegendRenderer renderer( &model, settings );

    // create context
    QgsRenderContext context;
    if ( !mWmsParameters.bbox().isEmpty() )
    {
      QgsMapSettings mapSettings;
      mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
      std::unique_ptr<QImage> tmp( createImage( mContext.mapSize( false ) ) );
      configureMapSettings( tmp.get(), mapSettings );
      context = QgsRenderContext::fromMapSettings( mapSettings );
    }
    else
    {
      //use default scale settings
      context = configureDefaultRenderContext();
    }

    // create image according to context
    std::unique_ptr<QImage> image;
    const qreal dpmm = mContext.dotsPerMm();
    const QSizeF minSize = renderer.minimumSize( &context );
    const QSize size( static_cast<int>( minSize.width() * dpmm ), static_cast<int>( minSize.height() * dpmm ) );
    if ( !mContext.isValidWidthHeight( size.width(), size.height() ) )
    {
      throw QgsServerException( QStringLiteral( "Legend image is too large" ) );
    }
    image.reset( createImage( size ) );

    // configure painter and addapt to the context
    QPainter painter( image.get() );

    context.setPainter( &painter );
    if ( painter.renderHints() & QPainter::SmoothPixmapTransform )
      context.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
    if ( painter.renderHints() & QPainter::LosslessImageRendering )
      context.setFlag( Qgis::RenderContextFlag::LosslessImageRendering, true );

    context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
    QgsScopedRenderContextScaleToMm scaleContext( context );

    // rendering
    renderer.drawLegend( context );
    painter.end();

    return image.release();
  }

  QImage *QgsRenderer::getLegendGraphics( QgsLayerTreeModelLegendNode &nodeModel )
  {
    // get layers
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers );

    // create image
    const QSize size( mWmsParameters.widthAsInt(), mWmsParameters.heightAsInt() );
    //test if legend image is larger than max width/height
    if ( !mContext.isValidWidthHeight( size.width(), size.height() ) )
    {
      throw QgsServerException( QStringLiteral( "Legend image is too large" ) );
    }
    std::unique_ptr<QImage> image( createImage( size ) );

    // configure painter
    const qreal dpmm = mContext.dotsPerMm();
    std::unique_ptr<QPainter> painter;
    painter.reset( new QPainter( image.get() ) );
    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->scale( dpmm, dpmm );

    // rendering
    QgsLegendSettings settings = legendSettings();
    QgsLayerTreeModelLegendNode::ItemContext ctx;
    ctx.painter = painter.get();

    // create context
    QgsRenderContext context = configureDefaultRenderContext( painter.get() );
    ctx.context = &context;

    nodeModel.drawSymbol( settings, &ctx, size.height() / dpmm );
    painter->end();

    return image.release();
  }

  QJsonObject QgsRenderer::getLegendGraphicsAsJson( QgsLayerTreeModel &model )
  {
    // get layers
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers );

    // init renderer
    QgsLegendSettings settings = legendSettings();
    QgsLegendRenderer renderer( &model, settings );

    // rendering
    QgsRenderContext renderContext;
    return renderer.exportLegendToJson( renderContext );
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
      context.setExtent( tr.transformBoundingBox( mapSettings.extent(), Qgis::TransformDirection::Reverse ) );

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

  QgsRenderer::HitTest QgsRenderer::symbols()
  {
    // check size
    if ( ! mContext.isValidWidthHeight() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    QStringLiteral( "The requested map size is too large" ) );
    }

    // init layer restorer before doing anything
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QgsMapSettings mapSettings;
    mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers, &mapSettings );

    // create the output image and the painter
    std::unique_ptr<QPainter> painter;
    std::unique_ptr<QImage> image( createImage( mContext.mapSize() ) );

    // configure map settings (background, DPI, ...)
    configureMapSettings( image.get(), mapSettings );

    // add layers to map settings
    mapSettings.setLayers( layers );

    // run hit tests
    HitTest symbols;
    runHitTest( mapSettings, symbols );

    return symbols;
  }

  QByteArray QgsRenderer::getPrint()
  {
    // init layer restorer before doing anything
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // GetPrint request needs a template parameter
    const QString templateName = mWmsParameters.composerTemplate();
    if ( templateName.isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MissingParameterValue,
                                    QgsWmsParameter::TEMPLATE );
    }
    else if ( QgsServerProjectUtils::wmsRestrictedComposers( *mProject ).contains( templateName ) )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    mWmsParameters[QgsWmsParameter::TEMPLATE ] );
    }

    // check template
    const QgsLayoutManager *lManager = mProject->layoutManager();
    QgsPrintLayout *sourceLayout( dynamic_cast<QgsPrintLayout *>( lManager->layoutByName( templateName ) ) );
    if ( !sourceLayout )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    mWmsParameters[QgsWmsParameter::TEMPLATE ] );
    }

    // Check that layout has at least one page
    if ( sourceLayout->pageCollection()->pageCount() < 1 )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    QStringLiteral( "The template has no pages" ) );
    }

    std::unique_ptr<QgsPrintLayout> layout( sourceLayout->clone() );

    //atlas print?
    QgsLayoutAtlas *atlas = nullptr;
    QStringList atlasPk = mWmsParameters.atlasPk();
    if ( !atlasPk.isEmpty() ) //atlas print requested?
    {
      atlas = layout->atlas();
      if ( !atlas || !atlas->enabled() )
      {
        //error
        throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                      QStringLiteral( "The template has no atlas enabled" ) );
      }

      QgsVectorLayer *cLayer = atlas->coverageLayer();
      if ( !cLayer )
      {
        throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                      QStringLiteral( "The atlas has no coverage layer" ) );
      }

      int maxAtlasFeatures = QgsServerProjectUtils::wmsMaxAtlasFeatures( *mProject );
      if ( atlasPk.size() == 1 && atlasPk.at( 0 ) == QLatin1String( "*" ) )
      {
        atlas->setFilterFeatures( false );
        atlas->updateFeatures();
        if ( atlas->count() > maxAtlasFeatures )
        {
          throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                        QString( "The project configuration allows printing maximum %1 atlas features at a time" ).arg( maxAtlasFeatures ) );
        }
      }
      else
      {
        const QgsAttributeList pkIndexes = cLayer->primaryKeyAttributes();
        if ( pkIndexes.size() == 0 )
        {
          QgsDebugMsgLevel( QStringLiteral( "Atlas print: layer %1 has no primary key attributes" ).arg( cLayer->name() ), 2 );
        }

        // Handles the pk-less case
        const int pkIndexesSize {std::max< int >( pkIndexes.size(), 1 )};

        QStringList pkAttributeNames;
        for ( int pkIndex : std::as_const( pkIndexes ) )
        {
          pkAttributeNames.append( cLayer->fields().at( pkIndex ).name() );
        }

        const int nAtlasFeatures = atlasPk.size() / pkIndexesSize;
        if ( nAtlasFeatures * pkIndexesSize != atlasPk.size() ) //Test if atlasPk.size() is a multiple of pkIndexesSize. Bail out if not
        {
          throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                        QStringLiteral( "Wrong number of ATLAS_PK parameters" ) );
        }

        //number of atlas features might be restricted
        if ( nAtlasFeatures > maxAtlasFeatures )
        {
          throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                        QString( "%1 atlas features have been requested, but the project configuration only allows printing %2 atlas features at a time" )
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

          // If the layer has no PK attributes, assume FID
          if ( pkAttributeNames.isEmpty() )
          {
            filterString.append( QStringLiteral( "$id = %1" ).arg( atlasPk.at( currentAtlasPk ) ) );
            ++currentAtlasPk;
          }
          else
          {
            for ( int j = 0; j < pkIndexes.size(); ++j )
            {
              if ( j > 0 )
              {
                filterString.append( " AND " );
              }
              filterString.append( QgsExpression::createFieldEqualityExpression( pkAttributeNames.at( j ), atlasPk.at( currentAtlasPk ) ) );
              ++currentAtlasPk;
            }
          }

          filterString.append( " )" );

        }

        atlas->setFilterFeatures( true );

        QString errorString;
        atlas->setFilterExpression( filterString, errorString );

        if ( !errorString.isEmpty() )
        {
          throw QgsException( QStringLiteral( "An error occurred during the Atlas print: %1" ).arg( errorString ) );
        }

      }
    }

    // configure layers
    QgsMapSettings mapSettings;
    mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers, &mapSettings );

    // configure map settings (background, DPI, ...)
    std::unique_ptr<QImage> image( new QImage() );
    configureMapSettings( image.get(), mapSettings );

    // add layers to map settings
    mapSettings.setLayers( layers );

    // configure layout
    configurePrintLayout( layout.get(), mapSettings, atlas );

    QgsLayoutRenderContext &layoutRendererContext = layout->renderContext();
    QgsFeatureFilterProviderGroup filters;
    const QList<QgsMapLayer *> lyrs = mapSettings.layers();

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mContext.accessControl()->resolveFilterFeatures( lyrs );
    filters.addProvider( mContext.accessControl() );
#endif

    QHash<const QgsVectorLayer *, QStringList> fltrs;
    for ( QgsMapLayer *l : lyrs )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l ) )
      {
        fltrs.insert( vl, dimensionFilter( vl ) );
      }
    }

    QgsDimensionFilter dimFilter( fltrs );
    filters.addProvider( &dimFilter );
    layoutRendererContext.setFeatureFilterProvider( &filters );

    // Get the temporary output file
    const QgsWmsParameters::Format format = mWmsParameters.format();
    const QString extension = QgsWmsParameters::formatAsString( format ).toLower();

    QTemporaryFile tempOutputFile( QDir::tempPath() +  '/' + QStringLiteral( "XXXXXX.%1" ).arg( extension ) );
    if ( !tempOutputFile.open() )
    {
      throw QgsException( QStringLiteral( "Could not open temporary file for the GetPrint request." ) );

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
      // Set scales
      exportSettings.predefinedMapScales = QgsLayoutUtils::predefinedScales( layout.get( ) );
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
        if ( ok )
          dpi = _dpi;
      }
      exportSettings.dpi = dpi;
      // Set scales
      exportSettings.predefinedMapScales = QgsLayoutUtils::predefinedScales( layout.get( ) );
      // Draw selections
      exportSettings.flags |= QgsLayoutRenderContext::FlagDrawSelection;
      // Destination image size in px
      QgsLayoutSize layoutSize( layout->pageCollection()->page( 0 )->sizeWithUnits() );

      QgsLayoutMeasurement width( layout->convertFromLayoutUnits( layoutSize.width(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );
      QgsLayoutMeasurement height( layout->convertFromLayoutUnits( layoutSize.height(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );

      const QSize imageSize = QSize( static_cast<int>( width.length() * dpi / 25.4 ), static_cast<int>( height.length() * dpi / 25.4 ) );

      const QString paramWidth = mWmsParameters.width();
      const QString paramHeight = mWmsParameters.height();

      // Prefer width and height from the http request
      // Fallback to predefined values from layout
      // Preserve aspect ratio if only one value is specified
      if ( !paramWidth.isEmpty() && !paramHeight.isEmpty() )
      {
        exportSettings.imageSize = QSize( paramWidth.toInt(), paramHeight.toInt() );
      }
      else if ( !paramWidth.isEmpty() && paramHeight.isEmpty() )
      {
        exportSettings.imageSize = QSize( paramWidth.toInt(), static_cast<double>( paramWidth.toInt() ) / imageSize.width() * imageSize.height() );
      }
      else if ( paramWidth.isEmpty() && !paramHeight.isEmpty() )
      {
        exportSettings.imageSize = QSize( static_cast<double>( paramHeight.toInt() ) / imageSize.height() * imageSize.width(), paramHeight.toInt() );
      }
      else
      {
        exportSettings.imageSize = imageSize;
      }

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
        else
        {
          throw QgsServiceException( QStringLiteral( "Bad request" ), QStringLiteral( "Atlas error: empty atlas." ), QString(), 400 );
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
      // Set scales
      exportSettings.predefinedMapScales = QgsLayoutUtils::predefinedScales( layout.get( ) );

      // Export all pages
      if ( atlas )
      {
        QgsLayoutExporter::exportToPdf( atlas, tempOutputFile.fileName(), exportSettings, exportError );
      }
      else
      {
        QgsLayoutExporter exporter( layout.get() );
        exporter.exportToPdf( tempOutputFile.fileName(), exportSettings );
      }
    }
    else //unknown format
    {
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidFormat,
                                    mWmsParameters[QgsWmsParameter::FORMAT] );
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

  bool QgsRenderer::configurePrintLayout( QgsPrintLayout *c, const QgsMapSettings &mapSettings, QgsLayoutAtlas *atlas )
  {

    c->renderContext().setSelectionColor( mapSettings.selectionColor() );
    // Maps are configured first
    QList<QgsLayoutItemMap *> maps;
    c->layoutItems<QgsLayoutItemMap>( maps );
    // Layout maps now use a string UUID as "id", let's assume that the first map
    // has id 0 and so on ...
    int mapId = 0;

    for ( const auto &map : std::as_const( maps ) )
    {
      QgsWmsParametersComposerMap cMapParams = mWmsParameters.composerMapParameters( mapId );
      mapId++;

      // If there are no configured layers, we take layers from unprefixed LAYER(S) if any
      if ( cMapParams.mLayers.isEmpty() )
      {
        cMapParams.mLayers = mWmsParameters.composerMapParameters( -1 ).mLayers;
      }

      if ( !atlas || !map->atlasDriven() ) //No need to extent, scale, rotation set with atlas feature
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
          map->setScale( static_cast<double>( cMapParams.mScale ) );
        }

        // rotation
        if ( cMapParams.mRotation )
        {
          map->setMapRotation( cMapParams.mRotation );
        }
      }

      if ( !map->keepLayerSet() )
      {

        QList<QgsMapLayer *> layerSet;

        for ( const auto &layer : std::as_const( cMapParams.mLayers ) )
        {
          if ( mContext.isValidGroup( layer.mNickname ) )
          {
            QList<QgsMapLayer *> layersFromGroup;

            const QList<QgsMapLayer *> cLayersFromGroup = mContext.layersFromGroup( layer.mNickname );
            for ( QgsMapLayer *layerFromGroup : cLayersFromGroup )
            {

              if ( ! layerFromGroup )
              {
                continue;
              }

              layersFromGroup.push_front( layerFromGroup );
            }

            if ( !layersFromGroup.isEmpty() )
            {
              layerSet.append( layersFromGroup );
            }
          }
          else
          {
            QgsMapLayer *mlayer = mContext.layer( layer.mNickname );

            if ( ! mlayer )
            {
              continue;
            }

            setLayerStyle( mlayer, layer.mStyle );
            layerSet << mlayer;
          }
        }

        std::reverse( layerSet.begin(), layerSet.end() );

        // If the map is set to follow preset we need to disable follow preset and manually
        // configure the layers here or the map item internal logic will override and get
        // the layers from the map theme.
        QMap<QString, QString> layersStyle;
        if ( map->followVisibilityPreset() )
        {

          if ( atlas )
          {
            // Possibly triggers a refresh of the DD visibility preset (theme) name
            // see issue GH #54475
            atlas->updateFeatures();
            atlas->first();
          }

          const QString presetName = map->followVisibilityPresetName();
          if ( layerSet.isEmpty() )
          {
            // Get the layers from the theme
            const QgsExpressionContext ex { map->createExpressionContext() };
            layerSet = map->layersToRender( &ex );
          }
          // Disable the theme
          map->setFollowVisibilityPreset( false );

          // Collect the style of each layer in the theme that has been disabled
          const QList<QgsMapThemeCollection::MapThemeLayerRecord> mapThemeRecords = QgsProject::instance()->mapThemeCollection()->mapThemeState( presetName ).layerRecords();
          for ( const auto &layerMapThemeRecord : std::as_const( mapThemeRecords ) )
          {
            if ( layerSet.contains( layerMapThemeRecord.layer() ) )
            {
              layersStyle.insert( layerMapThemeRecord.layer()->id(),
                                  layerMapThemeRecord.layer()->styleManager()->style( layerMapThemeRecord.currentStyle ).xmlData() );
            }
          }
        }

        // Handle highlight layers
        const QList< QgsMapLayer *> highlights = highlightLayers( cMapParams.mHighlightLayers );
        for ( const auto &hl : std::as_const( highlights ) )
        {
          layerSet.prepend( hl );
        }

        map->setLayers( layerSet );
        map->setKeepLayerSet( true );

        // Set style override if a particular style should be used due to a map theme.
        // It will actualize linked legend symbols too.
        if ( !layersStyle.isEmpty() )
        {
          map->setLayerStyleOverrides( layersStyle );
          map->setKeepLayerStyles( true );
        }
      }

      //grid space x / y
      if ( cMapParams.mGridX > 0 && cMapParams.mGridY > 0 )
      {
        map->grid()->setIntervalX( static_cast<double>( cMapParams.mGridX ) );
        map->grid()->setIntervalY( static_cast<double>( cMapParams.mGridY ) );
      }
    }

    // Labels
    QList<QgsLayoutItemLabel *> labels;
    c->layoutItems<QgsLayoutItemLabel>( labels );
    for ( const auto &label : std::as_const( labels ) )
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
    for ( const auto &html : std::as_const( htmls ) )
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
    for ( const auto &legend : std::as_const( legends ) )
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
        QList<QgsMapLayer *> mapLayers;
        if ( map->layers().isEmpty() )
        {
          // in QGIS desktop, each layer has its legend, including invisible layers
          // and using maptheme, legend items are automatically filtered
          mapLayers = mProject->mapLayers( true ).values();
        }
        else
        {
          mapLayers = map->layers();
        }
        const QList<QgsMapLayer *> layerList = mapLayers;
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

  QImage *QgsRenderer::getMap()
  {
    // check size
    if ( ! mContext.isValidWidthHeight() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    QStringLiteral( "The requested map size is too large" ) );
    }

    // init layer restorer before doing anything
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QList<QgsMapLayer *> layers = mContext.layersToRender();

    QgsMapSettings mapSettings;
    mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
    configureLayers( layers, &mapSettings );

    // create the output image and the painter
    std::unique_ptr<QPainter> painter;
    std::unique_ptr<QImage> image( createImage( mContext.mapSize() ) );

    // configure map settings (background, DPI, ...)
    configureMapSettings( image.get(), mapSettings );

    // add layers to map settings
    mapSettings.setLayers( layers );

    // rendering step for layers
    painter.reset( layersRendering( mapSettings, *image ) );

    // rendering step for annotations
    annotationsRendering( painter.get(), mapSettings );

    // painting is terminated
    painter->end();

    // scale output image if necessary (required by WMS spec)
    QImage *scaledImage = scaleImage( image.get() );
    if ( scaledImage )
      image.reset( scaledImage );

    // return
    return image.release();
  }

  std::unique_ptr<QgsDxfExport> QgsRenderer::getDxf()
  {
    // init layer restorer before doing anything
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // configure layers
    QList<QgsMapLayer *> layers = mContext.layersToRender();
    configureLayers( layers );

    // get dxf layers
    const QStringList attributes = mWmsParameters.dxfLayerAttributes();
    QList< QgsDxfExport::DxfLayer > dxfLayers;
    int layerIdx = -1;
    for ( QgsMapLayer *layer : layers )
    {
      layerIdx++;
      if ( layer->type() != QgsMapLayerType::VectorLayer )
        continue;

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

    //map extent
    QgsRectangle mapExtent = mWmsParameters.bboxAsRectangle();

    QString crs = mWmsParameters.crs();
    if ( crs.compare( QStringLiteral( "CRS:84" ), Qt::CaseInsensitive ) == 0 )
    {
      crs = QStringLiteral( "EPSG:4326" );
      mapExtent.invert();
    }
    else if ( crs.isEmpty() )
    {
      crs = QStringLiteral( "EPSG:4326" );
    }

    QgsCoordinateReferenceSystem outputCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );

    if ( !outputCRS.isValid() )
    {
      QgsServiceException::ExceptionCode code;
      QgsWmsParameter parameter;

      if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) )
      {
        code = QgsServiceException::OGC_InvalidCRS;
        parameter = mWmsParameters[ QgsWmsParameter::CRS ];
      }
      else
      {
        code = QgsServiceException::OGC_InvalidSRS;
        parameter = mWmsParameters[ QgsWmsParameter::SRS ];
      }

      throw QgsBadRequestException( code, parameter );
    }

    //then set destinationCrs

    // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
    if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) && outputCRS.hasAxisInverted() )
    {
      mapExtent.invert();
    }


    // add layers to dxf
    std::unique_ptr<QgsDxfExport> dxf = std::make_unique<QgsDxfExport>();
    dxf->setExtent( mapExtent );
    dxf->setDestinationCrs( outputCRS );
    dxf->addLayers( dxfLayers );
    dxf->setLayerTitleAsName( mWmsParameters.dxfUseLayerTitleAsName() );
    dxf->setSymbologyExport( mWmsParameters.dxfMode() );
    if ( mWmsParameters.dxfFormatOptions().contains( QgsWmsParameters::DxfFormatOption::SCALE ) )
    {
      dxf->setSymbologyScale( mWmsParameters.dxfScale() );
    }

    dxf->setForce2d( mWmsParameters.isForce2D() );
    QgsDxfExport::Flags flags;
    if ( mWmsParameters.noMText() )
      flags.setFlag( QgsDxfExport::Flag::FlagNoMText );

    dxf->setFlags( flags );

    return dxf;
  }

  static void infoPointToMapCoordinates( int i, int j, QgsPointXY *infoPoint, const QgsMapSettings &mapSettings )
  {
    //check if i, j are in the pixel range of the image
    if ( i < 0 || i > mapSettings.outputSize().width() )
    {
      QgsWmsParameter param( QgsWmsParameter::I );
      param.mValue = i;
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidPoint,
                                    param );
    }

    if ( j < 0 || j > mapSettings.outputSize().height() )
    {
      QgsWmsParameter param( QgsWmsParameter::J );
      param.mValue = j;
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidPoint,
                                    param );
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
    if ( mWmsParameters.queryLayersNickname().isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_MissingParameterValue,
                                    mWmsParameters[QgsWmsParameter::QUERY_LAYERS] );
    }

    // The I/J parameters are Mandatory if they are not replaced by X/Y or FILTER or FILTER_GEOM
    const bool ijDefined = !mWmsParameters.i().isEmpty() && !mWmsParameters.j().isEmpty();
    const bool xyDefined = !mWmsParameters.x().isEmpty() && !mWmsParameters.y().isEmpty();
    const bool filtersDefined = !mWmsParameters.filters().isEmpty();
    const bool filterGeomDefined = !mWmsParameters.filterGeom().isEmpty();

    if ( !ijDefined && !xyDefined && !filtersDefined && !filterGeomDefined )
    {
      QgsWmsParameter parameter = mWmsParameters[QgsWmsParameter::I];

      if ( mWmsParameters.j().isEmpty() )
        parameter = mWmsParameters[QgsWmsParameter::J];

      throw QgsBadRequestException( QgsServiceException::QGIS_MissingParameterValue, parameter );
    }

    const QgsWmsParameters::Format infoFormat = mWmsParameters.infoFormat();
    if ( infoFormat == QgsWmsParameters::Format::NONE )
    {
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidFormat,
                                    mWmsParameters[QgsWmsParameter::INFO_FORMAT] );
    }

    // create the mapSettings and the output image
    std::unique_ptr<QImage> outputImage( createImage( mContext.mapSize() ) );

    // init layer restorer before doing anything
    std::unique_ptr<QgsWmsRestorer> restorer;
    restorer.reset( new QgsWmsRestorer( mContext ) );

    // The CRS parameter is considered as mandatory in configureMapSettings
    // but in the case of filter parameter, CRS parameter has not to be mandatory
    bool mandatoryCrsParam = true;
    if ( filtersDefined && !ijDefined && !xyDefined && mWmsParameters.crs().isEmpty() )
    {
      mandatoryCrsParam = false;
    }

    // configure map settings (background, DPI, ...)
    QgsMapSettings mapSettings;
    mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
    configureMapSettings( outputImage.get(), mapSettings, mandatoryCrsParam );

    // compute scale denominator
    QgsScaleCalculator scaleCalc( ( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2, mapSettings.destinationCrs().mapUnits() );
    const double scaleDenominator = scaleCalc.calculate( mWmsParameters.bboxAsRectangle(), outputImage->width() );

    // configure layers
    QgsWmsRenderContext context = mContext;
    context.setScaleDenominator( scaleDenominator );

    QList<QgsMapLayer *> layers = context.layersToRender();
    configureLayers( layers, &mapSettings );

    // add layers to map settings
    mapSettings.setLayers( layers );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mContext.accessControl()->resolveFilterFeatures( mapSettings.layers() );
#endif

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

  QImage *QgsRenderer::createImage( const QSize &size ) const
  {
    std::unique_ptr<QImage> image;

    // use alpha channel only if necessary because it slows down performance
    QgsWmsParameters::Format format = mWmsParameters.format();
    bool transparent = mWmsParameters.transparentAsBool();

    if ( transparent && format != QgsWmsParameters::JPG )
    {
      image = std::make_unique<QImage>( size, QImage::Format_ARGB32_Premultiplied );
      image->fill( 0 );
    }
    else
    {
      image = std::make_unique<QImage>( size, QImage::Format_RGB32 );
      image->fill( mWmsParameters.backgroundColorAsColor() );
    }

    // Check that image was correctly created
    if ( image->isNull() )
    {
      throw QgsException( QStringLiteral( "createImage: image could not be created, check for out of memory conditions" ) );
    }

    const int dpm = static_cast<int>( mContext.dotsPerMm() * 1000.0 );
    image->setDotsPerMeterX( dpm );
    image->setDotsPerMeterY( dpm );

    return image.release();
  }

  void QgsRenderer::configureMapSettings( const QPaintDevice *paintDevice, QgsMapSettings &mapSettings, bool mandatoryCrsParam )
  {
    if ( !paintDevice )
    {
      throw QgsException( QStringLiteral( "configureMapSettings: no paint device" ) );
    }

    mapSettings.setOutputSize( QSize( paintDevice->width(), paintDevice->height() ) );
    // Recalculate from input DPI: do not take the (integer) value from paint device
    // because it loose precision!
    mapSettings.setOutputDpi( mContext.dotsPerMm() * 25.4 );

    //map extent
    QgsRectangle mapExtent = mWmsParameters.bboxAsRectangle();
    if ( !mWmsParameters.bbox().isEmpty() && mapExtent.isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    mWmsParameters[QgsWmsParameter::BBOX] );
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
      QgsServiceException::ExceptionCode code;
      QgsWmsParameter parameter;

      if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) )
      {
        code = QgsServiceException::OGC_InvalidCRS;
        parameter = mWmsParameters[ QgsWmsParameter::CRS ];
      }
      else
      {
        code = QgsServiceException::OGC_InvalidSRS;
        parameter = mWmsParameters[ QgsWmsParameter::SRS ];
      }

      throw QgsBadRequestException( code, parameter );
    }

    //then set destinationCrs
    mapSettings.setDestinationCrs( outputCRS );

    // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
    if ( mWmsParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) && outputCRS.hasAxisInverted() )
    {
      mapExtent.invert();
    }

    mapSettings.setExtent( mapExtent );

    // set the extent buffer
    mapSettings.setExtentBuffer( mContext.mapTileBuffer( paintDevice->width() ) );

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
    mapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization );

    mapSettings.setFlag( Qgis::MapSettingsFlag::RenderMapTile, mContext.renderMapTiles() );

    // set selection color
    mapSettings.setSelectionColor( mProject->selectionColor() );

    // Set WMS temporal properties
    // Note that this cannot parse multiple time instants while the vector dimensions implementation can
    const QString timeString { mWmsParameters.dimensionValues().value( QStringLiteral( "TIME" ), QString() ) };
    if ( ! timeString.isEmpty() )
    {
      bool isValidTemporalRange { true };
      QgsDateTimeRange range;
      // First try with a simple date/datetime instant
      const QDateTime dt { QDateTime::fromString( timeString, Qt::DateFormat::ISODateWithMs ) };
      if ( dt.isValid() )
      {
        range = QgsDateTimeRange( dt, dt );
      }
      else  // parse as an interval
      {
        try
        {
          range = QgsServerApiUtils::parseTemporalDateTimeInterval( timeString );
        }
        catch ( const QgsServerApiBadRequestException &ex )
        {
          isValidTemporalRange = false;
          QgsMessageLog::logMessage( QStringLiteral( "Could not parse TIME parameter into a temporal range" ), "Server", Qgis::MessageLevel::Warning );
        }
      }

      if ( isValidTemporalRange )
      {
        mIsTemporal = true;
        mapSettings.setIsTemporal( true );
        mapSettings.setTemporalRange( range );
      }

    }
  }

  QgsRenderContext QgsRenderer::configureDefaultRenderContext( QPainter *painter )
  {
    QgsRenderContext context = QgsRenderContext::fromQPainter( painter );
    context.setScaleFactor( mContext.dotsPerMm() );
    const double mmPerMapUnit = 1 / QgsServerProjectUtils::wmsDefaultMapUnitsPerMm( *mProject );
    context.setMapToPixel( QgsMapToPixel( 1 / ( mmPerMapUnit * context.scaleFactor() ) ) );
    QgsDistanceArea distanceArea = QgsDistanceArea();
    distanceArea.setSourceCrs( QgsCoordinateReferenceSystem( mWmsParameters.crs() ), mProject->transformContext() );
    distanceArea.setEllipsoid( geoNone() );
    context.setDistanceArea( distanceArea );
    return context;
  }

  QDomDocument QgsRenderer::featureInfoDocument( QList<QgsMapLayer *> &layers, const QgsMapSettings &mapSettings,
      const QImage *outputImage, const QString &version ) const
  {
    const QStringList queryLayers = mContext.flattenedQueryLayers( mContext.parameters().queryLayersNickname() );

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
      i *= ( outputImage->width() /  static_cast<double>( width ) );
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
    const QDomNode header = result.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
    result.appendChild( header );

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
      for ( QgsMapLayer *layer : std::as_const( layers ) )
      {
        if ( queryLayer == mContext.layerNickname( *layer ) )
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
            QHash<QString, QString>::const_iterator layerAliasIt = layerAliasMap.constFind( layerName );
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
      if ( !validLayer && !mContext.isValidLayer( queryLayer ) && !mContext.isValidGroup( queryLayer ) )
      {
        QgsWmsParameter param( QgsWmsParameter::LAYER );
        param.mValue = queryLayer;
        throw QgsBadRequestException( QgsServiceException::OGC_LayerNotDefined,
                                      param );
      }
      else if ( ( validLayer && !queryableLayer ) || ( !validLayer && mContext.isValidGroup( queryLayer ) ) )
      {
        QgsWmsParameter param( QgsWmsParameter::LAYER );
        param.mValue = queryLayer;
        // Check if this layer belongs to a group and the group has any queryable layers
        bool hasGroupAndQueryable { false };
        if ( ! mContext.parameters().queryLayersNickname().contains( queryLayer ) )
        {
          // Find which group this layer belongs to
          const QStringList constNicks { mContext.parameters().queryLayersNickname() };
          for ( const QString &ql : constNicks )
          {
            if ( mContext.layerGroups().contains( ql ) )
            {
              const QList<QgsMapLayer *> constLayers { mContext.layerGroups()[ql] };
              for ( const QgsMapLayer *ml : constLayers )
              {
                if ( ( ! ml->shortName().isEmpty() &&  ml->shortName() == queryLayer ) || ( ml->name() == queryLayer ) )
                {
                  param.mValue = ql;
                }
                if ( ml->flags().testFlag( QgsMapLayer::Identifiable ) )
                {
                  hasGroupAndQueryable = true;
                  break;
                }
              }
              break;
            }
          }
        }
        // Only throw if it's not a group or the group has no queryable children
        if ( ! hasGroupAndQueryable )
        {
          throw QgsBadRequestException( QgsServiceException::OGC_LayerNotQueryable,
                                        param );
        }
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

    bool hasGeometry = QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) || addWktGeometry || featureBBox || layerFilterGeom;
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

    mFeatureFilter.filterFeatures( layer, fReq );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mContext.accessControl()->filterFeatures( layer, fReq );

    QStringList attributes;
    for ( const QgsField &field : fields )
    {
      attributes.append( field.name() );
    }
    attributes = mContext.accessControl()->layerAttributes( layer, attributes );
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
        QString typeName = mContext.layerNickname( *layer );
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
        featureElement.setAttribute( QStringLiteral( "id" ), QgsServerFeatureId::getServerFid( feature, layer->dataProvider()->pkAttributeIndexes() ) );
        layerElement.appendChild( featureElement );

        featureAttributes = feature.attributes();
        QgsEditFormConfig editConfig = layer->editFormConfig();
        if ( QgsServerProjectUtils::wmsFeatureInfoUseAttributeFormSettings( *mProject ) && editConfig.layout() == QgsEditFormConfig::TabLayout )
        {
          writeAttributesTabLayout( editConfig, layer, fields, featureAttributes, infoDocument, featureElement, renderContext
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                    , &attributes
#endif
                                  );
        }
        else
        {
          for ( int i = 0; i < featureAttributes.count(); ++i )
          {
            writeVectorLayerAttribute( i, layer, fields, featureAttributes, infoDocument, featureElement, renderContext
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                       , &attributes
#endif
                                     );
          }
        }

        //add maptip attribute based on html/expression (in case there is no maptip attribute)
        QString mapTip = layer->mapTipTemplate();
        if ( !mapTip.isEmpty() && mWmsParameters.withMapTip() )
        {
          QDomElement maptipElem = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          maptipElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "maptip" ) );
          QgsExpressionContext context { renderContext.expressionContext() };
          context.appendScope( QgsExpressionContextUtils::layerScope( layer ) );
          maptipElem.setAttribute( QStringLiteral( "value" ),  QgsExpression::replaceExpressionText( mapTip, &context ) );
          featureElement.appendChild( maptipElem );
        }

        //append feature bounding box to feature info xml
        if ( QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) &&
             layer->wkbType() != QgsWkbTypes::NoGeometry && hasGeometry )
        {
          QDomElement bBoxElem = infoDocument.createElement( QStringLiteral( "BoundingBox" ) );
          bBoxElem.setAttribute( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS", outputCrs.authid() );
          bBoxElem.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( box.xMinimum(), mContext.precision() ) );
          bBoxElem.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( box.xMaximum(), mContext.precision() ) );
          bBoxElem.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( box.yMinimum(), mContext.precision() ) );
          bBoxElem.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( box.yMaximum(), mContext.precision() ) );
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
            geometryElement.setAttribute( QStringLiteral( "value" ), geom.asWkt( mContext.precision() ) );
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

  void QgsRenderer::writeAttributesTabGroup( const QgsAttributeEditorElement *group, QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &parentElem, QgsRenderContext &renderContext, QStringList *attributes ) const
  {
    const QgsAttributeEditorContainer *container = dynamic_cast<const QgsAttributeEditorContainer *>( group );
    if ( container )
    {
      QString groupName = container->name();
      QDomElement nameElem;

      if ( !groupName.isEmpty() )
      {
        nameElem = doc.createElement( groupName );
        parentElem.appendChild( nameElem );
      }

      const QList<QgsAttributeEditorElement *> children =  container->children();
      for ( const QgsAttributeEditorElement *child : children )
      {
        if ( child->type() == QgsAttributeEditorElement::AeTypeContainer )
        {
          writeAttributesTabGroup( child, layer, fields, featureAttributes, doc, nameElem.isNull() ? parentElem : nameElem, renderContext );
        }
        else if ( child->type() == QgsAttributeEditorElement::AeTypeField )
        {
          const QgsAttributeEditorField *editorField = dynamic_cast<const QgsAttributeEditorField *>( child );
          if ( editorField )
          {
            const int idx { fields.indexFromName( editorField->name() ) };
            if ( idx >= 0 )
            {
              writeVectorLayerAttribute( idx, layer, fields, featureAttributes, doc, nameElem.isNull() ? parentElem : nameElem, renderContext, attributes );
            }
          }
        }
      }
    }
  }

  void QgsRenderer::writeAttributesTabLayout( QgsEditFormConfig &config, QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &featureElem, QgsRenderContext &renderContext, QStringList *attributes ) const
  {
    QgsAttributeEditorContainer *editorContainer = config.invisibleRootContainer();
    if ( !editorContainer )
    {
      return;
    }

    writeAttributesTabGroup( editorContainer, layer, fields, featureAttributes, doc, featureElem, renderContext, attributes );
  }

  void QgsRenderer::writeVectorLayerAttribute( int attributeIndex,  QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &featureElem, QgsRenderContext &renderContext, QStringList *attributes ) const
  {
#ifndef HAVE_SERVER_PYTHON_PLUGINS
    Q_UNUSED( attributes );
#endif

    if ( !layer )
    {
      return;
    }

    //skip attribute if it is explicitly excluded from WMS publication
    if ( fields.at( attributeIndex ).configurationFlags().testFlag( QgsField::ConfigurationFlag::HideFromWms ) )
    {
      return;
    }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    //skip attribute if it is excluded by access control
    if ( attributes && !attributes->contains( fields.at( attributeIndex ).name() ) )
    {
      return;
    }
#endif

    QString attributeName = layer->attributeDisplayName( attributeIndex );
    QDomElement attributeElement = doc.createElement( QStringLiteral( "Attribute" ) );
    attributeElement.setAttribute( QStringLiteral( "name" ), attributeName );
    const QgsEditorWidgetSetup setup = layer->editorWidgetSetup( attributeIndex );
    attributeElement.setAttribute( QStringLiteral( "value" ),
                                   QgsExpression::replaceExpressionText(
                                     replaceValueMapAndRelation(
                                       layer, attributeIndex,
                                       featureAttributes[attributeIndex] ),
                                     &renderContext.expressionContext() )
                                 );
    featureElem.appendChild( attributeElement );
  }

  bool QgsRenderer::featureInfoFromRasterLayer( QgsRasterLayer *layer,
      const QgsMapSettings &mapSettings,
      const QgsPointXY *infoPoint,
      QDomDocument &infoDocument,
      QDomElement &layerElement,
      const QString &version ) const
  {
    Q_UNUSED( version )

    if ( !infoPoint || !layer || !layer->dataProvider() )
    {
      return false;
    }

    QgsMessageLog::logMessage( QStringLiteral( "infoPoint: %1 %2" ).arg( infoPoint->x() ).arg( infoPoint->y() ) );

    if ( !( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyValue ) &&
         !( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyFeature ) )
    {
      return false;
    }

    const QgsRaster::IdentifyFormat identifyFormat(
      static_cast<bool>( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyFeature )
      ? QgsRaster::IdentifyFormat::IdentifyFormatFeature
      : QgsRaster::IdentifyFormat::IdentifyFormatValue );

    QgsRasterIdentifyResult identifyResult;
    if ( layer->crs() != mapSettings.destinationCrs() )
    {
      const QgsRectangle extent { mapSettings.extent() };
      const QgsCoordinateTransform transform { mapSettings.destinationCrs(), layer->crs(), mapSettings.transformContext() };
      if ( ! transform.isValid() )
      {
        throw QgsBadRequestException( QgsServiceException::OGC_InvalidCRS, QStringLiteral( "CRS transform error from %1 to %2 in layer %3" )
                                      .arg( mapSettings.destinationCrs().authid() )
                                      .arg( layer->crs().authid() )
                                      .arg( layer->name() ) );
      }
      identifyResult = layer->dataProvider()->identify( *infoPoint, identifyFormat, transform.transform( extent ), mapSettings.outputSize().width(), mapSettings.outputSize().height() );
    }
    else
    {
      identifyResult = layer->dataProvider()->identify( *infoPoint, identifyFormat, mapSettings.extent(), mapSettings.outputSize().width(), mapSettings.outputSize().height() );
    }

    if ( !identifyResult.isValid() )
      return false;

    QMap<int, QVariant> attributes = identifyResult.results();

    if ( mWmsParameters.infoFormat() == QgsWmsParameters::Format::GML )
    {
      QgsFeature feature;
      QgsFields fields;
      QgsCoordinateReferenceSystem layerCrs = layer->crs();
      int gmlVersion = mWmsParameters.infoFormatVersion();
      QString typeName = mContext.layerNickname( *layer );

      if ( identifyFormat == QgsRaster::IdentifyFormatValue )
      {
        feature.initAttributes( attributes.count() );
        int index = 0;
        for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
        {
          fields.append( QgsField( layer->bandName( it.key() ), QVariant::Double ) );
          feature.setAttribute( index++, QString::number( it.value().toDouble() ) );
        }
        feature.setFields( fields );
        QDomElement elem = createFeatureGML(
                             &feature, nullptr, infoDocument, layerCrs, mapSettings, typeName, false, gmlVersion, nullptr );
        layerElement.appendChild( elem );
      }
      else
      {
        const auto values = identifyResult.results();
        for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
        {
          QVariant value = it.value();
          if ( value.type() == QVariant::Bool && !value.toBool() )
          {
            // sublayer not visible or not queryable
            continue;
          }

          if ( value.type() == QVariant::String )
          {
            continue;
          }

          // list of feature stores for a single sublayer
          const QgsFeatureStoreList featureStoreList = it.value().value<QgsFeatureStoreList>();

          for ( const QgsFeatureStore &featureStore : featureStoreList )
          {
            const QgsFeatureList storeFeatures = featureStore.features();
            for ( const QgsFeature &feature : storeFeatures )
            {
              QDomElement elem = createFeatureGML(
                                   &feature, nullptr, infoDocument, layerCrs, mapSettings, typeName, false, gmlVersion, nullptr );
              layerElement.appendChild( elem );
            }
          }
        }
      }
    }
    else
    {
      if ( identifyFormat == QgsRaster::IdentifyFormatValue )
      {
        for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
        {
          QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          attributeElement.setAttribute( QStringLiteral( "name" ), layer->bandName( it.key() ) );

          QString value;
          if ( ! QgsVariantUtils::isNull( it.value() ) )
          {
            value  = QString::number( it.value().toDouble() );
          }

          attributeElement.setAttribute( QStringLiteral( "value" ), value );
          layerElement.appendChild( attributeElement );
        }
      }
      else  // feature
      {
        const auto values = identifyResult.results();
        for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
        {
          QVariant value = it.value();
          if ( value.type() == QVariant::Bool && !value.toBool() )
          {
            // sublayer not visible or not queryable
            continue;
          }

          if ( value.type() == QVariant::String )
          {
            continue;
          }

          // list of feature stores for a single sublayer
          const QgsFeatureStoreList featureStoreList = it.value().value<QgsFeatureStoreList>();
          for ( const QgsFeatureStore &featureStore : featureStoreList )
          {
            const QgsFeatureList storeFeatures = featureStore.features();
            for ( const QgsFeature &feature : storeFeatures )
            {
              for ( const auto &fld : feature.fields() )
              {
                const auto val { feature.attribute( fld.name() )};
                if ( val.isValid() )
                {
                  QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
                  attributeElement.setAttribute( QStringLiteral( "name" ), fld.name() );
                  attributeElement.setAttribute( QStringLiteral( "value" ), val.toString() );
                  layerElement.appendChild( attributeElement );
                }
              }
            }
          }
        }
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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList tokens = filter.split( ' ', QString::SkipEmptyParts );
#else
    QStringList tokens = filter.split( ' ', Qt::SkipEmptyParts );
#endif
    groupStringList( tokens, QStringLiteral( "'" ) );
    groupStringList( tokens, QStringLiteral( "\"" ) );

    for ( auto tokenIt = tokens.constBegin() ; tokenIt != tokens.constEnd(); ++tokenIt )
    {
      //allowlist of allowed characters and keywords
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
           || tokenIt->compare( QLatin1String( "IS" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "NOT" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "NULL" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "AND" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "OR" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "IN" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "LIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "ILIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "DMETAPHONE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "SOUNDEX" ), Qt::CaseInsensitive ) == 0
           || mContext.settings().allowedExtraSqlTokens().contains( *tokenIt, Qt::CaseSensitivity::CaseInsensitive ) )
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
          QString value = attributeElement.attribute( QStringLiteral( "value" ) );
          if ( value.isEmpty() )
          {
            value = QStringLiteral( "no data" );
          }
          featureInfoString.append( "<TR><TH>" + attributeElement.attribute( QStringLiteral( "name" ) ) +
                                    "</TH><TD>" + value + "</TD></TR>\n" );
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
          QString value = attributeElement.attribute( QStringLiteral( "value" ) );
          if ( value.isEmpty() )
          {
            value = QStringLiteral( "no data" );
          }
          featureInfoString.append( attributeElement.attribute( QStringLiteral( "name" ) ) + " = '" +
                                    value + "'\n" );
        }
      }

      featureInfoString.append( "\n" );
    }

    return featureInfoString.toUtf8();
  }

  QByteArray QgsRenderer::convertFeatureInfoToJson( const QList<QgsMapLayer *> &layers, const QDomDocument &doc ) const
  {
    json json
    {
      { "type", "FeatureCollection" },
      { "features", json::array() },
    };
    const bool withGeometry = ( QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) && mWmsParameters.withGeometry() );

    const QDomNodeList layerList = doc.elementsByTagName( QStringLiteral( "Layer" ) );
    for ( int i = 0; i < layerList.size(); ++i )
    {
      const QDomElement layerElem = layerList.at( i ).toElement();
      const QString layerName = layerElem.attribute( QStringLiteral( "name" ) );

      QgsMapLayer *layer = nullptr;
      for ( QgsMapLayer *l : layers )
      {
        if ( mContext.layerNickname( *l ).compare( layerName ) == 0 )
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

        QMap<QgsFeatureId, QString> fidMap;

        for ( int j = 0; j < featuresNode.size(); ++j )
        {
          const QDomElement featureNode = featuresNode.at( j ).toElement();
          const QString fid = featureNode.attribute( QStringLiteral( "id" ) );
          QgsFeature feature;
          const QString expression { QgsServerFeatureId::getExpressionFromServerFid( fid, static_cast<QgsVectorDataProvider *>( layer->dataProvider() ) ) };
          if ( expression.isEmpty() )
          {
            feature = vl->getFeature( fid.toLongLong() );
          }
          else
          {
            QgsFeatureRequest request { QgsExpression( expression )};
            request.setFlags( QgsFeatureRequest::Flag::NoGeometry );
            vl->getFeatures( request ).nextFeature( feature );
          }

          fidMap.insert( feature.id(), fid );

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
        exporter.setTransformGeometries( false );

        for ( const auto &feature : std::as_const( features ) )
        {
          const QString id = QStringLiteral( "%1.%2" ).arg( layerName ).arg( fidMap.value( feature.id() ) );
          json["features"].push_back( exporter.exportFeatureToJsonObject( feature, QVariantMap(), id ) );
        }
      }
      else // raster layer
      {
        auto properties = json::object();
        const QDomNodeList attributesNode = layerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        for ( int j = 0; j < attributesNode.size(); ++j )
        {
          const QDomElement attrElmt = attributesNode.at( j ).toElement();
          const QString name = attrElmt.attribute( QStringLiteral( "name" ) );

          QString value = attrElmt.attribute( QStringLiteral( "value" ) );
          if ( value.isEmpty() )
          {
            value = QStringLiteral( "null" );
          }

          properties[name.toStdString()] = value.toStdString();
        }

        json["features"].push_back(
        {
          {"type", "Feature" },
          {"id", layerName.toStdString() },
          {"properties", properties }
        } );
      }
    }
#ifdef QGISDEBUG
    // This is only useful to generate human readable reference files for tests
    return QByteArray::fromStdString( json.dump( 2 ) );
#else
    return QByteArray::fromStdString( json.dump() );
#endif
  }

  QDomElement QgsRenderer::createFeatureGML(
    const QgsFeature *feat,
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
    QString fid;
    if ( layer && layer->dataProvider() )
      fid = QgsServerFeatureId::getServerFid( *feat, layer->dataProvider()->pkAttributeIndexes() );
    else
      fid = FID_TO_STRING( feat->id() );

    typeNameElement.setAttribute( QStringLiteral( "fid" ), QStringLiteral( "%1.%2" ).arg( typeName, fid ) );

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

    // always add bounding box info if feature contains geometry and has been
    // explicitly configured in the project
    if ( QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *mProject ) &&
         !geom.isNull() && geom.type() != QgsWkbTypes::UnknownGeometry &&
         geom.type() != QgsWkbTypes::NullGeometry )
    {
      QgsRectangle box = feat->geometry().boundingBox();
      if ( transform.isValid() )
      {
        try
        {
          box = transform.transformBoundingBox( box );
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
        boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, mContext.precision() );
      }
      else
      {
        boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, mContext.precision() );
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
        gmlElem = QgsOgcUtils::geometryToGML( geom, doc, mContext.precision() );
      }
      else
      {
        gmlElem = QgsOgcUtils::geometryToGML( geom, doc, QStringLiteral( "GML3" ), mContext.precision() );
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
      if ( fields.at( i ).configurationFlags().testFlag( QgsField::ConfigurationFlag::HideFromWms ) )
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

    if ( setup.config().value( QStringLiteral( "AllowMulti" ) ).toBool() && value.startsWith( QLatin1Char( '{' ) ) && value.endsWith( QLatin1Char( '}' ) ) )
    {
      value = value.mid( 1, value.size() - 2 );
    }
    return value;
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

  QList<QgsMapLayer *> QgsRenderer::highlightLayers( QList<QgsWmsParametersHighlightLayer> params )
  {
    QList<QgsMapLayer *> highlightLayers;

    // try to create highlight layer for each geometry
    QString crs = mWmsParameters.crs();
    for ( const QgsWmsParametersHighlightLayer &param : params )
    {
      // create sld document from symbology
      QDomDocument sldDoc;
      QString errorMsg;
      int errorLine;
      int errorColumn;
      if ( !sldDoc.setContent( param.mSld, true, &errorMsg, &errorLine, &errorColumn ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error parsing SLD for layer %1 at line %2, column %3:\n%4" )
                                   .arg( param.mName )
                                   .arg( errorLine )
                                   .arg( errorColumn )
                                   .arg( errorMsg ),
                                   QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
        continue;
      }

      // create renderer from sld document
      std::unique_ptr<QgsFeatureRenderer> renderer;
      QDomElement el = sldDoc.documentElement();
      renderer.reset( QgsFeatureRenderer::loadSld( el, param.mGeom.type(), errorMsg ) );
      if ( !renderer )
      {
        QgsMessageLog::logMessage( errorMsg, "Server", Qgis::MessageLevel::Info );
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
      const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
      std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( url, param.mName, QLatin1String( "memory" ), options );
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
        palSettings.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );
        palSettings.placementSettings().setAllowDegradedPlacement( true );
        palSettings.dist = param.mLabelDistance;

        if ( !qgsDoubleNear( param.mLabelRotation, 0 ) )
        {
          QgsPalLayerSettings::Property pR = QgsPalLayerSettings::LabelRotation;
          palSettings.dataDefinedProperties().setProperty( pR, param.mLabelRotation );
        }

        Qgis::LabelPlacement placement = Qgis::LabelPlacement::AroundPoint;
        switch ( param.mGeom.type() )
        {
          case QgsWkbTypes::PointGeometry:
          {
            if ( param.mHali.isEmpty() || param.mVali.isEmpty() || QgsWkbTypes::flatType( param.mGeom.wkbType() ) != QgsWkbTypes::Point )
            {
              placement = Qgis::LabelPlacement::AroundPoint;
              palSettings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlags() );
            }
            else //set label directly on point if there is hali/vali
            {
              QgsPointXY pt = param.mGeom.asPoint();
              QgsPalLayerSettings::Property pX = QgsPalLayerSettings::PositionX;
              QVariant x( pt.x() );
              palSettings.dataDefinedProperties().setProperty( pX, x );
              QgsPalLayerSettings::Property pY = QgsPalLayerSettings::PositionY;
              QVariant y( pt.y() );
              palSettings.dataDefinedProperties().setProperty( pY, y );
              QgsPalLayerSettings::Property pHali = QgsPalLayerSettings::Hali;
              palSettings.dataDefinedProperties().setProperty( pHali, param.mHali );
              QgsPalLayerSettings::Property pVali = QgsPalLayerSettings::Vali;
              palSettings.dataDefinedProperties().setProperty( pVali, param.mVali );
            }

            break;
          }
          case QgsWkbTypes::PolygonGeometry:
          {
            QgsGeometry point = param.mGeom.pointOnSurface();
            QgsPointXY pt = point.asPoint();
            placement = Qgis::LabelPlacement::AroundPoint;

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
            placement = Qgis::LabelPlacement::Line;
            palSettings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );
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
          bufferSettings.setSize( static_cast<double>( param.mBufferSize ) );
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

  void QgsRenderer::removeTemporaryLayers()
  {
    qDeleteAll( mTemporaryLayers );
    mTemporaryLayers.clear();
  }

  QPainter *QgsRenderer::layersRendering( const QgsMapSettings &mapSettings, QImage &image ) const
  {
    QPainter *painter = nullptr;

    QgsFeatureFilterProviderGroup filters;
    filters.addProvider( &mFeatureFilter );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mContext.accessControl()->resolveFilterFeatures( mapSettings.layers() );
    filters.addProvider( mContext.accessControl() );
#endif
    QgsMapRendererJobProxy renderJob( mContext.settings().parallelRendering(), mContext.settings().maxThreads(), &filters );
    renderJob.render( mapSettings, &image );
    painter = renderJob.takePainter();

    if ( !renderJob.errors().isEmpty() )
    {
      const QgsMapRendererJob::Error e = renderJob.errors().at( 0 );

      QString layerWMSName;
      QgsMapLayer *errorLayer = mProject->mapLayer( e.layerID );
      if ( errorLayer )
      {
        layerWMSName = mContext.layerNickname( *errorLayer );
      }

      QString errorMessage = QStringLiteral( "Rendering error : '%1'" ).arg( e.message );
      if ( ! layerWMSName.isEmpty() )
      {
        errorMessage = QStringLiteral( "Rendering error : '%1' in layer '%2'" ).arg( e.message, layerWMSName );
      }
      throw QgsException( errorMessage );
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

        case QgsMapLayerType::VectorTileLayer:
        {
          QgsVectorTileLayer *vl = qobject_cast<QgsVectorTileLayer *>( layer );
          vl->setOpacity( opacity / 255. );
          break;
        }

        case QgsMapLayerType::MeshLayer:
        case QgsMapLayerType::PluginLayer:
        case QgsMapLayerType::AnnotationLayer:
        case QgsMapLayerType::PointCloudLayer:
        case QgsMapLayerType::GroupLayer:
          break;
      }
    }
  }

  void QgsRenderer::setLayerFilter( QgsMapLayer *layer, const QList<QgsWmsParametersFilter> &filters )
  {

    if ( layer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *filteredLayer = qobject_cast<QgsVectorLayer *>( layer );
      QStringList expList;
      for ( const QgsWmsParametersFilter &filter : filters )
      {
        if ( filter.mType == QgsWmsParametersFilter::OGC_FE )
        {
          // OGC filter
          QDomDocument filterXml;
          QString errorMsg;
          if ( !filterXml.setContent( filter.mFilter, true, &errorMsg ) )
          {
            throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                          QStringLiteral( "Filter string rejected. Error message: %1. The XML string was: %2" ).arg( errorMsg, filter.mFilter ) );
          }
          QDomElement filterElem = filterXml.firstChildElement();
          std::unique_ptr<QgsExpression> filterExp( QgsOgcUtils::expressionFromOgcFilter( filterElem, filter.mVersion, filteredLayer ) );

          if ( filterExp )
          {
            expList << filterExp->dump();
          }
        }
        else if ( filter.mType == QgsWmsParametersFilter::SQL )
        {
          // QGIS (SQL) filter
          if ( !testFilterStringSafety( filter.mFilter ) )
          {
            throw QgsSecurityException( QStringLiteral( "The filter string %1"
                                        " has been rejected because of security reasons."
                                        " Note: Text strings have to be enclosed in single or double quotes."
                                        " A space between each word / special character is mandatory."
                                        " Allowed Keywords and special characters are"
                                        " IS,NOT,NULL,AND,OR,IN,=,<,>=,>,>=,!=,',',(,),DMETAPHONE,SOUNDEX%2."
                                        " Not allowed are semicolons in the filter expression." ).arg(
                                          filter.mFilter, mContext.settings().allowedExtraSqlTokens().isEmpty() ?
                                          QString() :
                                          mContext.settings().allowedExtraSqlTokens().join( ',' ).prepend( ',' ) ) );
          }

          QString newSubsetString = filter.mFilter;
          if ( !filteredLayer->subsetString().isEmpty() )
          {
            newSubsetString.prepend( ") AND (" );
            newSubsetString.append( ")" );
            newSubsetString.prepend( filteredLayer->subsetString() );
            newSubsetString.prepend( "(" );
          }
          if ( ! filteredLayer->setSubsetString( newSubsetString ) )
          {
            QgsMessageLog::logMessage( QStringLiteral( "Error setting subset string from filter for layer %1, filter: %2" ).arg( layer->name(), newSubsetString ),
                                       QStringLiteral( "Server" ),
                                       Qgis::MessageLevel::Warning );
            throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                          QStringLiteral( "Filter not valid for layer %1: check the filter syntax and the field names." ).arg( layer->name() ) );

          }
        }
      }

      expList.append( dimensionFilter( filteredLayer ) );

      // Join and apply expressions provided by OGC filter and Dimensions
      QString exp;
      if ( expList.size() == 1 )
      {
        exp = expList[0];
      }
      else if ( expList.size() > 1 )
      {
        exp = QStringLiteral( "( %1 )" ).arg( expList.join( QLatin1String( " ) AND ( " ) ) );
      }
      if ( !exp.isEmpty() )
      {
        std::unique_ptr<QgsExpression> expression( new QgsExpression( exp ) );
        if ( expression )
        {
          mFeatureFilter.setFilter( filteredLayer, *expression );
        }
      }
    }
  }

  QStringList QgsRenderer::dimensionFilter( QgsVectorLayer *layer ) const
  {
    QStringList expList;
    // WMS Dimension filters
    QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( layer->serverProperties() );
    const QList<QgsMapLayerServerProperties::WmsDimensionInfo> wmsDims = serverProperties->wmsDimensions();
    if ( wmsDims.isEmpty() )
    {
      return expList;
    }

    QMap<QString, QString> dimParamValues = mContext.parameters().dimensionValues();
    for ( const QgsMapLayerServerProperties::WmsDimensionInfo &dim : wmsDims )
    {
      // Skip temporal properties for this layer, give precedence to the dimensions implementation
      if ( mIsTemporal && dim.name.toUpper() == QLatin1String( "TIME" ) && layer->temporalProperties()->isActive() )
      {
        layer->temporalProperties()->setIsActive( false );
      }
      // Check field index
      int fieldIndex = layer->fields().indexOf( dim.fieldName );
      if ( fieldIndex == -1 )
      {
        continue;
      }
      // Check end field index
      int endFieldIndex = -1;
      if ( !dim.endFieldName.isEmpty() )
      {
        endFieldIndex = layer->fields().indexOf( dim.endFieldName );
        if ( endFieldIndex == -1 )
        {
          continue;
        }
      }
      // Apply dimension filtering
      if ( !dimParamValues.contains( dim.name.toUpper() ) )
      {
        // Default value based on type configured by user
        QVariant defValue;
        if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::AllValues )
        {
          continue; // no filter by default for this dimension
        }
        else if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::ReferenceValue )
        {
          defValue = dim.referenceValue;
        }
        else
        {
          // get unique values
          QSet<QVariant> uniqueValues = layer->uniqueValues( fieldIndex );
          if ( endFieldIndex != -1 )
          {
            uniqueValues.unite( layer->uniqueValues( endFieldIndex ) );
          }
          // sort unique values
          QList<QVariant> values = qgis::setToList( uniqueValues );
          std::sort( values.begin(), values.end() );
          if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MinValue )
          {
            defValue = values.first();
          }
          else if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MaxValue )
          {
            defValue = values.last();
          }
        }
        // build expression
        if ( endFieldIndex == -1 )
        {
          expList << QgsExpression::createFieldEqualityExpression( dim.fieldName, defValue );
        }
        else
        {
          QStringList expElems;
          expElems << QgsExpression::quotedColumnRef( dim.fieldName )
                   << QStringLiteral( "<=" ) << QgsExpression::quotedValue( defValue )
                   << QStringLiteral( "AND" ) << QgsExpression::quotedColumnRef( dim.endFieldName )
                   << QStringLiteral( ">=" ) << QgsExpression::quotedValue( defValue );
          expList << expElems.join( ' ' );
        }
      }
      else
      {
        // Get field to convert value provided in parameters
        QgsField dimField = layer->fields().at( fieldIndex );
        // Value provided in parameters
        QString dimParamValue = dimParamValues[dim.name.toUpper()];
        // The expression list for this dimension
        QStringList dimExplist;
        // Multiple values are separated by ,
        QStringList dimValues = dimParamValue.split( ',' );
        for ( int i = 0; i < dimValues.size(); ++i )
        {
          QString dimValue = dimValues[i];
          // Trim value if necessary
          if ( dimValue.size() > 1 )
          {
            dimValue = dimValue.trimmed();
          }
          // Range value is separated by / for example 0/1
          if ( dimValue.contains( '/' ) )
          {
            QStringList rangeValues = dimValue.split( '/' );
            // Check range value size
            if ( rangeValues.size() != 2 )
            {
              continue; // throw an error
            }
            // Get range values
            QVariant rangeMin = QVariant( rangeValues[0] );
            QVariant rangeMax = QVariant( rangeValues[1] );
            // Convert and check range values
            if ( !dimField.convertCompatible( rangeMin ) )
            {
              continue; // throw an error
            }
            if ( !dimField.convertCompatible( rangeMax ) )
            {
              continue; // throw an error
            }
            // Build expression for this range
            QStringList expElems;
            if ( endFieldIndex == -1 )
            {
              // The field values are between min and max range
              expElems << QgsExpression::quotedColumnRef( dim.fieldName )
                       << QStringLiteral( ">=" ) << QgsExpression::quotedValue( rangeMin )
                       << QStringLiteral( "AND" ) << QgsExpression::quotedColumnRef( dim.fieldName )
                       << QStringLiteral( "<=" ) << QgsExpression::quotedValue( rangeMax );
            }
            else
            {
              // The start field or the end field are lesser than min range
              // or the start field or the end field are greater than min range
              expElems << QStringLiteral( "(" ) << QgsExpression::quotedColumnRef( dim.fieldName )
                       << QStringLiteral( ">=" ) << QgsExpression::quotedValue( rangeMin )
                       << QStringLiteral( "OR" ) << QgsExpression::quotedColumnRef( dim.endFieldName )
                       << QStringLiteral( ">=" ) << QgsExpression::quotedValue( rangeMin )
                       << QStringLiteral( ") AND (" ) << QgsExpression::quotedColumnRef( dim.fieldName )
                       << QStringLiteral( "<=" ) << QgsExpression::quotedValue( rangeMax )
                       << QStringLiteral( "OR" ) << QgsExpression::quotedColumnRef( dim.endFieldName )
                       << QStringLiteral( "<=" ) << QgsExpression::quotedValue( rangeMax )
                       << QStringLiteral( ")" );
            }
            dimExplist << expElems.join( ' ' );
          }
          else
          {
            QVariant dimVariant = QVariant( dimValue );
            if ( !dimField.convertCompatible( dimVariant ) )
            {
              continue; // throw an error
            }
            // Build expression for this value
            if ( endFieldIndex == -1 )
            {
              // Field is equal to
              dimExplist << QgsExpression::createFieldEqualityExpression( dim.fieldName, dimVariant );
            }
            else
            {
              // The start field is lesser or equal to
              // and the end field is greater or equal to
              QStringList expElems;
              expElems << QgsExpression::quotedColumnRef( dim.fieldName )
                       << QStringLiteral( "<=" ) << QgsExpression::quotedValue( dimVariant )
                       << QStringLiteral( "AND" ) << QgsExpression::quotedColumnRef( dim.endFieldName )
                       << QStringLiteral( ">=" ) << QgsExpression::quotedValue( dimVariant );
              dimExplist << expElems.join( ' ' );
            }
          }
        }
        // Build the expression for this dimension
        if ( dimExplist.size() == 1 )
        {
          expList << dimExplist;
        }
        else if ( dimExplist.size() > 1 )
        {
          expList << QStringLiteral( "( %1 )" ).arg( dimExplist.join( QLatin1String( " ) OR ( " ) ) );
        }
      }
    }
    return expList;
  }

  void QgsRenderer::setLayerSelection( QgsMapLayer *layer, const QStringList &fids ) const
  {
    if ( !fids.empty() && layer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );

      QgsFeatureRequest request;
      QgsServerFeatureId::updateFeatureRequestFromServerFids( request, fids, vl->dataProvider() );
      const QgsFeatureIds selectedIds = request.filterFids();

      if ( selectedIds.empty() )
      {
        vl->selectByExpression( request.filterExpression()->expression() );
      }
      else
      {
        vl->selectByIds( selectedIds );
      }
    }
  }

  void QgsRenderer::setLayerAccessControlFilter( QgsMapLayer *layer ) const
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( mContext.accessControl(), layer );
#else
    Q_UNUSED( layer )
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

  void QgsRenderer::annotationsRendering( QPainter *painter, const QgsMapSettings &mapSettings ) const
  {
    const QgsAnnotationManager *annotationManager = mProject->annotationManager();
    const QList< QgsAnnotation * > annotations = annotationManager->annotations();

    QgsRenderContext renderContext = QgsRenderContext::fromQPainter( painter );
    renderContext.setFlag( Qgis::RenderContextFlag::RenderBlocking );
    for ( QgsAnnotation *annotation : annotations )
    {
      if ( !annotation || !annotation->isVisible() )
        continue;

      //consider item position
      double offsetX = 0;
      double offsetY = 0;
      if ( annotation->hasFixedMapPosition() )
      {
        QgsPointXY mapPos = annotation->mapPosition();
        if ( mapSettings.destinationCrs() != annotation->mapPositionCrs() )
        {
          QgsCoordinateTransform coordTransform( annotation->mapPositionCrs(), mapSettings.destinationCrs(), mapSettings.transformContext() );
          try
          {
            mapPos = coordTransform.transform( mapPos );
          }
          catch ( const QgsCsException &e )
          {
            QgsMessageLog::logMessage( QStringLiteral( "Error transforming coordinates of annotation item: %1" ).arg( e.what() ) );
          }
        }
        const QgsPointXY devicePos = mapSettings.mapToPixel().transform( mapPos );
        offsetX = devicePos.x();
        offsetY = devicePos.y();
      }
      else
      {
        const QPointF relativePos = annotation->relativePosition();
        offsetX = mapSettings.outputSize().width() * relativePos.x();
        offsetY = mapSettings.outputSize().height() * relativePos.y();
      }

      painter->save();
      painter->translate( offsetX, offsetY );
      annotation->render( renderContext );
      painter->restore();
    }
  }

  QImage *QgsRenderer::scaleImage( const QImage *image ) const
  {
    // Test if width / height ratio of image is the same as the ratio of
    // WIDTH / HEIGHT parameters. If not, the image has to be scaled (required
    // by WMS spec)
    QImage *scaledImage = nullptr;
    const int width = mWmsParameters.widthAsInt();
    const int height = mWmsParameters.heightAsInt();
    if ( width != image->width() || height != image->height() )
    {
      scaledImage = new QImage( image->scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
    }

    return scaledImage;
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
        const QgsMapRendererJob::Error e = ( *mapIt )->renderingErrors().at( 0 );
        throw QgsException( QStringLiteral( "Rendering error : '%1' in layer %2" ).arg( e.message, e.layerID ) );
      }
    }
  }

  void QgsRenderer::configureLayers( QList<QgsMapLayer *> &layers, QgsMapSettings *settings )
  {
    const bool useSld = !mContext.parameters().sldBody().isEmpty();

    for ( auto layer : layers )
    {
      const QgsWmsParametersLayer param = mContext.parameters( *layer );

      if ( ! mContext.layersToRender().contains( layer ) )
      {
        continue;
      }

      if ( mContext.isExternalLayer( param.mNickname ) )
      {
        if ( mContext.testFlag( QgsWmsRenderContext::UseOpacity ) )
        {
          setLayerOpacity( layer, param.mOpacity );
        }
        continue;
      }

      if ( useSld )
      {
        setLayerSld( layer, mContext.sld( *layer ) );
      }
      else
      {
        setLayerStyle( layer, mContext.style( *layer ) );
      }

      if ( mContext.testFlag( QgsWmsRenderContext::UseOpacity ) )
      {
        setLayerOpacity( layer, param.mOpacity );
      }

      if ( mContext.testFlag( QgsWmsRenderContext::UseFilter ) )
      {
        setLayerFilter( layer, param.mFilter );
      }

      if ( mContext.testFlag( QgsWmsRenderContext::SetAccessControl ) )
      {
        setLayerAccessControlFilter( layer );
      }

      if ( mContext.testFlag( QgsWmsRenderContext::UseSelection ) )
      {
        setLayerSelection( layer, param.mSelection );
      }

      if ( settings && mContext.updateExtent() )
      {
        updateExtent( layer, *settings );
      }
    }

    if ( mContext.testFlag( QgsWmsRenderContext::AddHighlightLayers ) )
    {
      layers = highlightLayers( mWmsParameters.highlightLayersParameters() ) << layers;
    }
  }

  void QgsRenderer::setLayerStyle( QgsMapLayer *layer, const QString &style ) const
  {
    if ( style.isEmpty() )
    {
      return;
    }

    bool rc = layer->styleManager()->setCurrentStyle( style );
    if ( ! rc )
    {
      throw QgsBadRequestException( QgsServiceException::OGC_StyleNotDefined,
                                    QStringLiteral( "Style '%1' does not exist for layer '%2'" ).arg( style, layer->name() ) );
    }
  }

  void QgsRenderer::setLayerSld( QgsMapLayer *layer, const QDomElement &sld ) const
  {
    QString err;
    // Defined sld style name
    const QStringList styles = layer->styleManager()->styles();
    QString sldStyleName = "__sld_style";
    while ( styles.contains( sldStyleName ) )
    {
      sldStyleName.append( '@' );
    }
    layer->styleManager()->addStyleFromLayer( sldStyleName );
    layer->styleManager()->setCurrentStyle( sldStyleName );
    layer->readSld( sld, err );
    layer->setCustomProperty( "sldStyleName", sldStyleName );
  }

  QgsLegendSettings QgsRenderer::legendSettings()
  {
    // getting scale from bbox or default size
    QgsLegendSettings settings = mWmsParameters.legendSettings();

    if ( !mWmsParameters.bbox().isEmpty() )
    {
      QgsMapSettings mapSettings;
      mapSettings.setFlag( Qgis::MapSettingsFlag::RenderBlocking );
      std::unique_ptr<QImage> tmp( createImage( mContext.mapSize( false ) ) );
      configureMapSettings( tmp.get(), mapSettings );
      // QGIS 4.0 - require correct use of QgsRenderContext instead of these
      Q_NOWARN_DEPRECATED_PUSH
      settings.setMapScale( mapSettings.scale() );
      settings.setMapUnitsPerPixel( mapSettings.mapUnitsPerPixel() );
      Q_NOWARN_DEPRECATED_POP
    }
    else
    {
      // QGIS 4.0 - require correct use of QgsRenderContext instead of these
      Q_NOWARN_DEPRECATED_PUSH
      const double defaultMapUnitsPerPixel = QgsServerProjectUtils::wmsDefaultMapUnitsPerMm( *mContext.project() ) / mContext.dotsPerMm();
      settings.setMapUnitsPerPixel( defaultMapUnitsPerPixel );
      Q_NOWARN_DEPRECATED_POP
    }

    return settings;
  }
} // namespace QgsWms
