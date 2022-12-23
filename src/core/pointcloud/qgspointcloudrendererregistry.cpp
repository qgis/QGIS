/***************************************************************************
    qgspointcloudrendererregistry.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudrenderer.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

// default renderers
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudextentrenderer.h"

QgsPointCloudRendererRegistry::QgsPointCloudRendererRegistry()
{
  // add default renderers
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "extent" ),
               QObject::tr( "Extent Only" ),
               QgsPointCloudExtentRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "ramp" ),
               QObject::tr( "Attribute by Ramp" ),
               QgsPointCloudAttributeByRampRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "rgb" ),
               QObject::tr( "RGB" ),
               QgsPointCloudRgbRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "classified" ),
               QObject::tr( "Classification" ),
               QgsPointCloudClassifiedRenderer::create ) );
}

QgsPointCloudRendererRegistry::~QgsPointCloudRendererRegistry()
{
  qDeleteAll( mRenderers );
}

bool QgsPointCloudRendererRegistry::addRenderer( QgsPointCloudRendererAbstractMetadata *metadata )
{
  if ( !metadata || mRenderers.contains( metadata->name() ) )
    return false;

  mRenderers[metadata->name()] = metadata;
  mRenderersOrder << metadata->name();
  return true;
}

bool QgsPointCloudRendererRegistry::removeRenderer( const QString &rendererName )
{
  if ( !mRenderers.contains( rendererName ) )
    return false;

  delete mRenderers[rendererName];
  mRenderers.remove( rendererName );
  mRenderersOrder.removeAll( rendererName );
  return true;
}

QgsPointCloudRendererAbstractMetadata *QgsPointCloudRendererRegistry::rendererMetadata( const QString &rendererName )
{
  return mRenderers.value( rendererName );
}

QStringList QgsPointCloudRendererRegistry::renderersList() const
{
  QStringList renderers;
  for ( const QString &renderer : mRenderersOrder )
  {
    QgsPointCloudRendererAbstractMetadata *r = mRenderers.value( renderer );
    if ( r )
      renderers << renderer;
  }
  return renderers;
}

QgsPointCloudRenderer *QgsPointCloudRendererRegistry::defaultRenderer( const QgsPointCloudLayer *layer )
{
  const QgsPointCloudDataProvider *provider = layer->dataProvider();
  if ( !provider )
    return new QgsPointCloudAttributeByRampRenderer();

  const QgsPointCloudStatistics stats = layer->statistics();

  if ( ( provider->name() == QLatin1String( "pdal" ) ) && ( !provider->hasValidIndex() ) )
  {
    // for now, default to extent renderer only for las/laz files
    return new QgsPointCloudExtentRenderer();
  }

  // If we are calculating statistics, we default to the extent renderer until the statistics calculation finishes
  if ( layer->statisticsCalculationState() == QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculating )
  {
    return new QgsPointCloudExtentRenderer();
  }

  const QgsPointCloudAttributeCollection attributes = provider->attributes();

  //if red/green/blue attributes are present, then default to a RGB renderer
  if ( attributes.indexOf( QLatin1String( "Red" ) ) >= 0 && attributes.indexOf( QLatin1String( "Green" ) ) >= 0 && attributes.indexOf( QLatin1String( "Blue" ) ) >= 0 )
  {
    std::unique_ptr< QgsPointCloudRgbRenderer > renderer = std::make_unique< QgsPointCloudRgbRenderer >();

    // set initial guess for rgb ranges
    const double redMax = stats.maximum( QStringLiteral( "Red" ) );
    const double greenMax = stats.maximum( QStringLiteral( "Red" ) );
    const double blueMax = stats.maximum( QStringLiteral( "Red" ) );
    if ( !std::isnan( redMax ) && !std::isnan( greenMax ) && !std::isnan( blueMax ) )
    {
      const int maxValue = std::max( blueMax, std::max( redMax, greenMax ) );

      if ( maxValue == 0 )
      {
        // r/g/b max value is 0 -- likely these attributes have been created by some process, but are empty.
        // in any case they won't result in a useful render, so don't use RGB renderer for this dataset.
        renderer.reset();
      }
      else
      {
        // try and guess suitable range from input max values -- we don't just take the provider max value directly here, but rather see if it's
        // likely to be 8 bit or 16 bit color values
        const int rangeGuess = maxValue > 255 ? 65535 : 255;

        if ( rangeGuess > 255 )
        {
          // looks like 16 bit colors, so default to a stretch contrast enhancement
          QgsContrastEnhancement contrast( Qgis::DataType::UnknownDataType );
          contrast.setMinimumValue( 0 );
          contrast.setMaximumValue( rangeGuess );
          contrast.setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum );
          renderer->setRedContrastEnhancement( new QgsContrastEnhancement( contrast ) );
          renderer->setGreenContrastEnhancement( new QgsContrastEnhancement( contrast ) );
          renderer->setBlueContrastEnhancement( new QgsContrastEnhancement( contrast ) );
        }
      }
    }
    else
    {
      QgsContrastEnhancement contrast( Qgis::DataType::UInt16 );
      contrast.setMinimumValue( std::numeric_limits<uint16_t>::lowest() );
      contrast.setMaximumValue( std::numeric_limits<uint16_t>::max() );
      contrast.setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum );
      renderer->setRedContrastEnhancement( new QgsContrastEnhancement( contrast ) );
      renderer->setGreenContrastEnhancement( new QgsContrastEnhancement( contrast ) );
      renderer->setBlueContrastEnhancement( new QgsContrastEnhancement( contrast ) );
    }

    if ( renderer )
      return renderer.release();
  }

  // otherwise try a classified renderer...
  if ( attributes.indexOf( QLatin1String( "Classification" ) ) >= 0 )
  {
    // are any classifications present?
    QList<int> classes = stats.classesOf( QStringLiteral( "Classification" ) );
    // ignore "not classified" classes, and see if any are left...
    classes.removeAll( 0 );
    classes.removeAll( 1 );
    if ( !classes.empty() )
    {
      const QgsPointCloudCategoryList categories = classificationAttributeCategories( layer );
      std::unique_ptr< QgsPointCloudClassifiedRenderer > renderer = std::make_unique< QgsPointCloudClassifiedRenderer >( QLatin1String( "Classification" ), categories );
      return renderer.release();
    }
  }

  // fallback to shading by Z
  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > renderer = std::make_unique< QgsPointCloudAttributeByRampRenderer >();
  renderer->setAttribute( QStringLiteral( "Z" ) );

  // set initial range for z values if possible
  const double zMin = stats.minimum( QStringLiteral( "Z" ) );
  const double zMax = stats.maximum( QStringLiteral( "Z" ) );
  if ( !std::isnan( zMin ) && !std::isnan( zMax ) )
  {
    renderer->setMinimum( zMin );
    renderer->setMaximum( zMax );

    QgsColorRampShader shader = renderer->colorRampShader();
    shader.setMinimumValue( zMin );
    shader.setMaximumValue( zMax );
    shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
    renderer->setColorRampShader( shader );
  }
  return renderer.release();
}

QgsPointCloudCategoryList QgsPointCloudRendererRegistry::classificationAttributeCategories( const QgsPointCloudLayer *layer )
{
  if ( !layer )
    return QgsPointCloudCategoryList();

  const QgsPointCloudStatistics stats = layer->statistics();
  const QList<int> layerClasses = stats.classesOf( QStringLiteral( "Classification" ) );
  const QgsPointCloudCategoryList defaultCategories = QgsPointCloudClassifiedRenderer::defaultCategories();

  QgsPointCloudCategoryList categories;
  for ( const int &layerClass : layerClasses )
  {
    const bool isDefaultCategory = layerClass >= 0 && layerClass < defaultCategories.size();
    const QColor color = isDefaultCategory ? defaultCategories.at( layerClass ).color() : QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
    const QString label = isDefaultCategory ? QgsPointCloudDataProvider::translatedLasClassificationCodes().value( layerClass, QString::number( layerClass ) ) : QString::number( layerClass );
    categories.append( QgsPointCloudCategory( layerClass, color, label ) );
  }
  return categories;
}
