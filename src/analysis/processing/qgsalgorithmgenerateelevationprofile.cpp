/***************************************************************************
                         qgsalgorithmgenerateelevationprofile.cpp
                         ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmgenerateelevationprofile.h"

#include "qgis.h"
#include "qgsabstractprofilesource.h"
#include "qgscurve.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsfontutils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsplot.h"
#include "qgsprofilerequest.h"
#include "qgsterrainprovider.h"
#include "qgstextformat.h"

///@cond PRIVATE

class QgsAlgorithmElevationProfilePlotItem : public Qgs2DXyPlot
{
  public:
    explicit QgsAlgorithmElevationProfilePlotItem( int width, int height, int dpi )
      : mDpi( dpi )
    {
      setYMinimum( 0 );
      setYMaximum( 10 );
      setSize( QSizeF( width, height ) );
    }

    void setRenderer( QgsProfilePlotRenderer *renderer )
    {
      mRenderer = renderer;
    }

    QRectF plotArea()
    {
      if ( !mPlotArea.isNull() )
      {
        return mPlotArea;
      }

      // calculate plot area
      QgsRenderContext context;
      context.setScaleFactor( mDpi / 25.4 );

      QgsPlotRenderContext plotContext;
      calculateOptimisedIntervals( context, plotContext );
      mPlotArea = interiorPlotArea( context, plotContext );
      return mPlotArea;
    }

    void renderContent( QgsRenderContext &rc, QgsPlotRenderContext &, const QRectF &plotArea, const QgsPlotData & ) override
    {
      mPlotArea = plotArea;

      if ( !mRenderer )
        return;

      rc.painter()->translate( mPlotArea.left(), mPlotArea.top() );
      const QStringList sourceIds = mRenderer->sourceIds();
      for ( const QString &source : sourceIds )
      {
        mRenderer->render( rc, mPlotArea.width(), mPlotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum(), source );
      }
      rc.painter()->translate( -mPlotArea.left(), -mPlotArea.top() );
    }

  private:
    int mDpi = 96;
    QRectF mPlotArea;
    QgsProfilePlotRenderer *mRenderer = nullptr;
};

void QgsGenerateElevationProfileAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterGeometry( u"CURVE"_s, QObject::tr( "Profile curve" ), QVariant(), false, QList<int>() << static_cast<int>( Qgis::GeometryType::Line ) ) );
  addParameter( new QgsProcessingParameterMultipleLayers( u"MAP_LAYERS"_s, QObject::tr( "Map layers" ), Qgis::ProcessingSourceType::MapLayer, QVariant(), false ) );
  addParameter( new QgsProcessingParameterNumber( u"WIDTH"_s, QObject::tr( "Chart width (in pixels)" ), Qgis::ProcessingNumberParameterType::Integer, 400, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"HEIGHT"_s, QObject::tr( "Chart height (in pixels)" ), Qgis::ProcessingNumberParameterType::Integer, 300, false, 0 ) );
  addParameter( new QgsProcessingParameterMapLayer( u"TERRAIN_LAYER"_s, QObject::tr( "Terrain layer" ), QVariant(), true, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Raster ) << static_cast<int>( Qgis::ProcessingSourceType::Mesh ) ) );

  auto minimumDistanceParam = std::make_unique<QgsProcessingParameterNumber>( u"MINIMUM_DISTANCE"_s, QObject::tr( "Chart minimum distance (X axis)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  minimumDistanceParam->setFlags( minimumDistanceParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( minimumDistanceParam.release() );
  auto maximumDistanceParam = std::make_unique<QgsProcessingParameterNumber>( u"MAXIMUM_DISTANCE"_s, QObject::tr( "Chart maximum distance (X axis)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  maximumDistanceParam->setFlags( maximumDistanceParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( maximumDistanceParam.release() );
  auto minimumElevationParam = std::make_unique<QgsProcessingParameterNumber>( u"MINIMUM_ELEVATION"_s, QObject::tr( "Chart minimum elevation (Y axis)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  minimumElevationParam->setFlags( minimumElevationParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( minimumElevationParam.release() );
  auto maximumElevationParam = std::make_unique<QgsProcessingParameterNumber>( u"MAXIMUM_ELEVATION"_s, QObject::tr( "Chart maximum elevation (Y axis)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  maximumElevationParam->setFlags( maximumElevationParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( maximumElevationParam.release() );

  auto textColorParam = std::make_unique<QgsProcessingParameterColor>( u"TEXT_COLOR"_s, QObject::tr( "Chart text color" ), QColor( 0, 0, 0 ), true, true );
  textColorParam->setFlags( textColorParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( textColorParam.release() );

  auto textFontFamilyParam = std::make_unique<QgsProcessingParameterString>( u"TEXT_FONT_FAMILY"_s, QObject::tr( "Chart text font family" ), QVariant(), false, true );
  textFontFamilyParam->setFlags( textFontFamilyParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( textFontFamilyParam.release() );

  auto textFontStyleParam = std::make_unique<QgsProcessingParameterString>( u"TEXT_FONT_STYLE"_s, QObject::tr( "Chart text font style" ), QVariant(), false, true );
  textFontStyleParam->setFlags( textFontStyleParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( textFontStyleParam.release() );

  auto textFontSizeParam = std::make_unique<QgsProcessingParameterNumber>( u"TEXT_FONT_SIZE"_s, QObject::tr( "Chart text font size" ), Qgis::ProcessingNumberParameterType::Double, 0, true );
  textFontSizeParam->setFlags( textFontSizeParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( textFontSizeParam.release() );

  auto backgroundColorParam = std::make_unique<QgsProcessingParameterColor>( u"BACKGROUND_COLOR"_s, QObject::tr( "Chart background color" ), QColor( 255, 255, 255 ), true, true );
  backgroundColorParam->setFlags( backgroundColorParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( backgroundColorParam.release() );
  auto borderColorParam = std::make_unique<QgsProcessingParameterColor>( u"BORDER_COLOR"_s, QObject::tr( "Chart border color" ), QColor( 99, 99, 99 ), true, true );
  borderColorParam->setFlags( borderColorParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( borderColorParam.release() );

  auto toleranceParam = std::make_unique<QgsProcessingParameterNumber>( u"TOLERANCE"_s, QObject::tr( "Profile tolerance" ), Qgis::ProcessingNumberParameterType::Double, 5.0, false, 0 );
  toleranceParam->setFlags( toleranceParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( toleranceParam.release() );

  auto dpiParam = std::make_unique<QgsProcessingParameterNumber>( u"DPI"_s, QObject::tr( "Chart DPI" ), Qgis::ProcessingNumberParameterType::Integer, 96, false, 0 );
  dpiParam->setFlags( dpiParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dpiParam.release() );

  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Output image" ) ) );
}

QString QgsGenerateElevationProfileAlgorithm::name() const
{
  return u"generateelevationprofileimage"_s;
}

QString QgsGenerateElevationProfileAlgorithm::displayName() const
{
  return QObject::tr( "Generate elevation profile image" );
}

QStringList QgsGenerateElevationProfileAlgorithm::tags() const
{
  return QObject::tr( "altitude,elevation,terrain,dem" ).split( ',' );
}

QString QgsGenerateElevationProfileAlgorithm::group() const
{
  return QObject::tr( "Plots" );
}

QString QgsGenerateElevationProfileAlgorithm::groupId() const
{
  return u"plots"_s;
}

QString QgsGenerateElevationProfileAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates an elevation profile image from a list of map layer and an optional terrain." );
}

QString QgsGenerateElevationProfileAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates an elevation profile image from a list of map layer and an optional terrain." );
}

QgsGenerateElevationProfileAlgorithm *QgsGenerateElevationProfileAlgorithm::createInstance() const
{
  return new QgsGenerateElevationProfileAlgorithm();
}

bool QgsGenerateElevationProfileAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QgsGeometry curveGeom = parameterAsGeometry( parameters, u"CURVE"_s, context );
  const QgsCoordinateReferenceSystem curveCrs = parameterAsGeometryCrs( parameters, u"CURVE"_s, context );

  QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"MAP_LAYERS"_s, context );
  QgsMapLayer *terrainLayer = parameterAsLayer( parameters, u"TERRAIN_LAYER"_s, context );

  const double tolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );

  QList<QgsAbstractProfileSource *> sources;
  for ( QgsMapLayer *layer : layers )
  {
    if ( QgsAbstractProfileSource *source = layer->profileSource() )
      sources.append( source );
  }

  QgsProfileRequest request( static_cast<QgsCurve *>( curveGeom.constGet()->clone() ) );
  request.setCrs( curveCrs );
  request.setTolerance( tolerance );
  request.setTransformContext( context.transformContext() );
  request.setExpressionContext( context.expressionContext() );

  if ( terrainLayer )
  {
    if ( QgsRasterLayer *rasterLayer = dynamic_cast<QgsRasterLayer *>( terrainLayer ) )
    {
      auto terrainProvider = std::make_unique<QgsRasterDemTerrainProvider>();
      terrainProvider->setLayer( rasterLayer );
      request.setTerrainProvider( terrainProvider.release() );
    }
    else if ( QgsMeshLayer *meshLayer = dynamic_cast<QgsMeshLayer *>( terrainLayer ) )
    {
      auto terrainProvider = std::make_unique<QgsMeshTerrainProvider>();
      terrainProvider->setLayer( meshLayer );
      request.setTerrainProvider( terrainProvider.release() );
    }
  }


  mRenderer = std::make_unique<QgsProfilePlotRenderer>( sources, request );

  return true;
}

QVariantMap QgsGenerateElevationProfileAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QgsGeometry curveGeom = parameterAsGeometry( parameters, u"CURVE"_s, context );

  const bool hasMinimumDistance = parameters.value( u"MINIMUM_DISTANCE"_s ).isValid();
  const double minimumDistance = parameterAsDouble( parameters, u"MINIMUM_DISTANCE"_s, context );
  const bool hasMaximumDistance = parameters.value( u"MAXIMUM_DISTANCE"_s ).isValid();
  const double maximumDistance = parameterAsDouble( parameters, u"MAXIMUM_DISTANCE"_s, context );
  const bool hasMinimumElevation = parameters.value( u"MINIMUM_ELEVATION"_s ).isValid();
  const double minimumElevation = parameterAsDouble( parameters, u"MINIMUM_ELEVATION"_s, context );
  const bool hasMaximumElevation = parameters.value( u"MAXIMUM_ELEVATION"_s ).isValid();
  const double maximumElevation = parameterAsDouble( parameters, u"MAXIMUM_ELEVATION"_s, context );

  const int width = parameterAsInt( parameters, u"WIDTH"_s, context );
  const int height = parameterAsInt( parameters, u"HEIGHT"_s, context );
  const int dpi = parameterAsInt( parameters, u"DPI"_s, context );

  const QString outputImage = parameterAsString( parameters, u"OUTPUT"_s, context );

  const QColor textColor = parameterAsColor( parameters, u"TEXT_COLOR"_s, context );
  const QColor backgroundColor = parameterAsColor( parameters, u"BACKGROUND_COLOR"_s, context );
  const QColor borderColor = parameterAsColor( parameters, u"BORDER_COLOR"_s, context );

  QgsAlgorithmElevationProfilePlotItem plotItem( width, height, dpi );

  const QString textFontFamily = parameterAsString( parameters, u"TEXT_FONT_FAMILY"_s, context );
  const QString textFontStyle = parameterAsString( parameters, u"TEXT_FONT_STYLE"_s, context );
  const double textFontSize = parameterAsDouble( parameters, u"TEXT_FONT_SIZE"_s, context );

  if ( !textFontFamily.isEmpty() || !textFontStyle.isEmpty() || textFontSize > 0 )
  {
    QgsTextFormat textFormat = plotItem.xAxis().textFormat();
    QFont font = textFormat.font();
    if ( !textFontFamily.isEmpty() )
    {
      QgsFontUtils::setFontFamily( font, textFontFamily );
    }
    if ( !textFontStyle.isEmpty() )
    {
      QgsFontUtils::updateFontViaStyle( font, textFontStyle );
    }
    textFormat.setFont( font );
    if ( textFontSize > 0 )
    {
      textFormat.setSize( textFontSize );
      textFormat.setSizeUnit( Qgis::RenderUnit::Points );
    }

    plotItem.xAxis().setTextFormat( textFormat );
    plotItem.yAxis().setTextFormat( textFormat );
  }

  if ( textColor.isValid() )
  {
    QgsTextFormat textFormat = plotItem.xAxis().textFormat();
    textFormat.setColor( textColor );
    plotItem.xAxis().setTextFormat( textFormat );
    textFormat = plotItem.yAxis().textFormat();
    textFormat.setColor( textColor );
    plotItem.yAxis().setTextFormat( textFormat );
  }

  if ( borderColor.isValid() )
  {
    auto lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>( borderColor, 0.1 );
    lineSymbolLayer->setPenCapStyle( Qt::FlatCap );
    plotItem.xAxis().setGridMinorSymbol( new QgsLineSymbol( QgsSymbolLayerList( { lineSymbolLayer->clone() } ) ) );
    plotItem.yAxis().setGridMinorSymbol( new QgsLineSymbol( QgsSymbolLayerList( { lineSymbolLayer->clone() } ) ) );
    plotItem.xAxis().setGridMajorSymbol( new QgsLineSymbol( QgsSymbolLayerList( { lineSymbolLayer->clone() } ) ) );
    plotItem.yAxis().setGridMajorSymbol( new QgsLineSymbol( QgsSymbolLayerList( { lineSymbolLayer->clone() } ) ) );
    plotItem.setChartBorderSymbol( new QgsFillSymbol( QgsSymbolLayerList( { lineSymbolLayer.release() } ) ) );
  }

  if ( backgroundColor.isValid() )
  {
    auto fillSymbolLayer = std::make_unique<QgsSimpleFillSymbolLayer>( backgroundColor, Qt::SolidPattern, backgroundColor );
    plotItem.setChartBackgroundSymbol( new QgsFillSymbol( QgsSymbolLayerList( { fillSymbolLayer.release() } ) ) );
  }

  QgsProfileGenerationContext generationContext;
  generationContext.setDpi( dpi );
  generationContext.setMaximumErrorMapUnits( MAX_ERROR_PIXELS * ( curveGeom.constGet()->length() ) / plotItem.plotArea().width() );
  generationContext.setMapUnitsPerDistancePixel( curveGeom.constGet()->length() / plotItem.plotArea().width() );

  mRenderer->setContext( generationContext );

  mRenderer->startGeneration();
  mRenderer->waitForFinished();

  const QgsDoubleRange zRange = mRenderer->zRange();
  double zMinimum = 0;
  double zMaximum = 0;
  if ( zRange.upper() < zRange.lower() )
  {
    // invalid range, e.g. no features found in plot!
    zMinimum = 0;
    zMaximum = 10;
  }
  else if ( qgsDoubleNear( zRange.lower(), zRange.upper(), 0.0000001 ) )
  {
    // corner case ... a zero height plot! Just pick an arbitrary +/- 5 height range.
    zMinimum = zRange.lower() - 5;
    zMaximum = zRange.lower() + 5;
  }
  else
  {
    // add 5% margin to height range
    const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
    zMinimum = zRange.lower() - margin;
    zMaximum = zRange.upper() + margin;
  }

  plotItem.setYMinimum( hasMinimumElevation ? minimumElevation : zMinimum );
  plotItem.setYMaximum( hasMaximumElevation ? maximumElevation : zMaximum );
  plotItem.setXMinimum( hasMinimumDistance ? minimumDistance : 0 );
  plotItem.setXMaximum( hasMaximumDistance ? maximumDistance : curveGeom.constGet()->length() );

  plotItem.setRenderer( mRenderer.get() );

  QImage image( static_cast<int>( plotItem.size().width() ), static_cast<int>( plotItem.size().height() ), QImage::Format_ARGB32_Premultiplied );
  image.fill( Qt::transparent );

  QPainter painter( &image );
  painter.setRenderHint( QPainter::Antialiasing, true );
  QgsRenderContext renderContext = QgsRenderContext::fromQPainter( &painter );
  renderContext.setScaleFactor( dpi / 25.4 );
  renderContext.setExpressionContext( context.expressionContext() );
  QgsPlotRenderContext plotContext;
  plotItem.calculateOptimisedIntervals( renderContext, plotContext );
  plotItem.render( renderContext, plotContext );
  painter.end();
  image.save( outputImage );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputImage );
  return outputs;
}

///@endcond
