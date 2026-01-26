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

#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgspointcloudrenderer.h"

// default renderers
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudextentrenderer.h"

QgsPointCloudRendererRegistry::QgsPointCloudRendererRegistry()
{
  // add default renderers
  addRenderer( new QgsPointCloudRendererMetadata( u"extent"_s,
               QObject::tr( "Extent Only" ),
               QgsPointCloudExtentRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( u"ramp"_s,
               QObject::tr( "Attribute by Ramp" ),
               QgsPointCloudAttributeByRampRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( u"rgb"_s,
               QObject::tr( "RGB" ),
               QgsPointCloudRgbRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( u"classified"_s,
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

  if ( ( provider->name() == "pdal"_L1 ) && ( !provider->hasValidIndex() ) )
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
  if ( attributes.indexOf( "Red"_L1 ) >= 0 && attributes.indexOf( "Green"_L1 ) >= 0 && attributes.indexOf( "Blue"_L1 ) >= 0 )
  {
    auto renderer = std::make_unique< QgsPointCloudRgbRenderer >();

    // set initial guess for rgb ranges
    const double redMax = stats.maximum( u"Red"_s );
    const double greenMax = stats.maximum( u"Red"_s );
    const double blueMax = stats.maximum( u"Red"_s );
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
  if ( attributes.indexOf( "Classification"_L1 ) >= 0 )
  {
    // are any classifications present?
    QList<int> classes = stats.classesOf( u"Classification"_s );
    // ignore "not classified" classes, and see if any are left...
    classes.removeAll( 0 );
    classes.removeAll( 1 );
    if ( !classes.empty() )
    {
      const QgsPointCloudCategoryList categories = classificationAttributeCategories( layer );
      auto renderer = std::make_unique< QgsPointCloudClassifiedRenderer >( "Classification"_L1, categories );
      return renderer.release();
    }
  }

  // fallback to shading by Z
  auto renderer = std::make_unique< QgsPointCloudAttributeByRampRenderer >();
  renderer->setAttribute( u"Z"_s );

  // set initial range for z values if possible
  const double zMin = stats.minimum( u"Z"_s );
  const double zMax = stats.maximum( u"Z"_s );
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
  const QList<int> layerClasses = stats.classesOf( u"Classification"_s );
  const QgsPointCloudCategoryList defaultCategories = QgsPointCloudClassifiedRenderer::defaultCategories();

  if ( layerClasses.isEmpty() )
    return defaultCategories;

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
