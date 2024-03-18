/***************************************************************************
                         qgsrasterrendererregistry.cpp
                         -----------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgspalettedrasterrenderer.h"
#include "qgscolorrampimpl.h"
#include "qgsrastercontourrenderer.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgshillshaderenderer.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgscontrastenhancement.h"

#include <QIcon>

QgsRasterRendererRegistryEntry::QgsRasterRendererRegistryEntry( const QString &name, const QString &visibleName,
    QgsRasterRendererCreateFunc rendererFunction,
    QgsRasterRendererWidgetCreateFunc widgetFunction, Qgis::RasterRendererCapabilities capabilities )
  : name( name )
  , visibleName( visibleName )
  , capabilities( capabilities )
  , rendererCreateFunction( rendererFunction )
  , widgetCreateFunction( widgetFunction )
{
}

QIcon QgsRasterRendererRegistryEntry::icon()
{
  return QgsApplication::getThemeIcon( QString( "styleicons/%1.svg" ).arg( name ) );
}

QgsRasterRendererRegistry::QgsRasterRendererRegistry()
{
  // insert items in a particular order, which is returned in renderersList()
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "multibandcolor" ), QObject::tr( "Multiband color" ),
                                          QgsMultiBandColorRenderer::create, nullptr,
                                          Qgis::RasterRendererCapability::UsesMultipleBands ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "paletted" ), QObject::tr( "Paletted/Unique values" ), QgsPalettedRasterRenderer::create, nullptr ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "singlebandgray" ), QObject::tr( "Singleband gray" ),
                                          QgsSingleBandGrayRenderer::create, nullptr ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "singlebandpseudocolor" ), QObject::tr( "Singleband pseudocolor" ),
                                          QgsSingleBandPseudoColorRenderer::create, nullptr ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "singlebandcolordata" ), QObject::tr( "Singleband color data" ),
                                          QgsSingleBandColorDataRenderer::create, nullptr ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "hillshade" ), QObject::tr( "Hillshade" ),
                                          QgsHillshadeRenderer::create, nullptr ) );
  insert( QgsRasterRendererRegistryEntry( QStringLiteral( "contour" ), QObject::tr( "Contours" ),
                                          QgsRasterContourRenderer::create, nullptr ) );
}

void QgsRasterRendererRegistry::insert( const QgsRasterRendererRegistryEntry &entry )
{
  mEntries.insert( entry.name, entry );
  mSortedEntries.append( entry.name );
}

void QgsRasterRendererRegistry::insertWidgetFunction( const QString &rendererName, QgsRasterRendererWidgetCreateFunc func )
{
  if ( !mEntries.contains( rendererName ) )
  {
    return;
  }
  mEntries[rendererName].widgetCreateFunction = func;
}

bool QgsRasterRendererRegistry::rendererData( const QString &rendererName, QgsRasterRendererRegistryEntry &data ) const
{
  const QHash< QString, QgsRasterRendererRegistryEntry >::const_iterator it = mEntries.find( rendererName );
  if ( it == mEntries.constEnd() )
  {
    return false;
  }
  data = it.value();
  return true;
}

QStringList QgsRasterRendererRegistry::renderersList() const
{
  return mSortedEntries;
}

QList< QgsRasterRendererRegistryEntry > QgsRasterRendererRegistry::entries() const
{
  QList< QgsRasterRendererRegistryEntry > result;

  QHash< QString, QgsRasterRendererRegistryEntry >::const_iterator it = mEntries.constBegin();
  for ( ; it != mEntries.constEnd(); ++it )
  {
    result.push_back( it.value() );
  }
  return result;
}

Qgis::RasterRendererCapabilities QgsRasterRendererRegistry::rendererCapabilities( const QString &rendererName ) const
{
  const QHash< QString, QgsRasterRendererRegistryEntry >::const_iterator it = mEntries.constFind( rendererName );
  if ( it != mEntries.constEnd() )
  {
    return it.value().capabilities;
  }
  return Qgis::RasterRendererCapabilities();
}

QgsRasterRenderer *QgsRasterRendererRegistry::defaultRendererForDrawingStyle( Qgis::RasterDrawingStyle drawingStyle, QgsRasterDataProvider *provider ) const
{
  if ( !provider || provider->bandCount() < 1 )
  {
    return nullptr;
  }

  std::unique_ptr< QgsRasterRenderer > renderer;
  switch ( drawingStyle )
  {
    case Qgis::RasterDrawingStyle::PalettedColor:
    {
      const int grayBand = 1; //reasonable default

      // first preference -- use attribute table to generate classes
      if ( provider->attributeTable( grayBand ) )
      {
        std::unique_ptr<QgsColorRamp> ramp;
        if ( ! provider->attributeTable( grayBand )->hasColor() )
        {
          ramp = std::make_unique< QgsRandomColorRamp >();
        }
        const QgsPalettedRasterRenderer::MultiValueClassData classes = QgsPalettedRasterRenderer::rasterAttributeTableToClassData( provider->attributeTable( grayBand ), -1, ramp.get() );
        if ( !classes.empty() )
        {
          renderer = std::make_unique< QgsPalettedRasterRenderer >( provider, grayBand, classes );
        }
      }

      // second preference -- use raster color table to generate classes
      if ( !renderer )
      {
        const QgsPalettedRasterRenderer::ClassData classes = QgsPalettedRasterRenderer::colorTableToClassData( provider->colorTable( grayBand ) );
        if ( !classes.empty() )
        {
          renderer = std::make_unique< QgsPalettedRasterRenderer >( provider, grayBand, classes );
        }
      }

      // last preference -- just fallback to single band gray renderer if we couldn't determine color palette
      if ( ! renderer )
      {
        renderer = std::make_unique< QgsSingleBandGrayRenderer >( provider, grayBand );

        QgsContrastEnhancement *ce = new QgsContrastEnhancement( ( Qgis::DataType )(
              provider->dataType( grayBand ) ) );

        // Default contrast enhancement is set from QgsRasterLayer, it has already setContrastEnhancementAlgorithm(). Default enhancement must only be set if default style was not loaded (to avoid stats calculation).
        qgis::down_cast< QgsSingleBandGrayRenderer * >( renderer.get() )->setContrastEnhancement( ce );
      }
    }
    break;

    case Qgis::RasterDrawingStyle::MultiBandSingleBandGray:
    case Qgis::RasterDrawingStyle::SingleBandGray:
    {
      const int grayBand = 1;

      // If the raster band has an attribute table try to use it.
      QString ratErrorMessage;
      if ( QgsRasterAttributeTable *rat = provider->attributeTable( grayBand ); rat && rat->isValid( &ratErrorMessage ) )
      {
        renderer.reset( rat->createRenderer( provider, grayBand ) );
      }

      if ( ! ratErrorMessage.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Invalid RAT from band 1, RAT was not used to create the renderer: %1." ).arg( ratErrorMessage ), 2 );
      }

      if ( ! renderer )
      {
        renderer = std::make_unique< QgsSingleBandGrayRenderer >( provider, grayBand );

        QgsContrastEnhancement *ce = new QgsContrastEnhancement( ( Qgis::DataType )(
              provider->dataType( grayBand ) ) );

        // Default contrast enhancement is set from QgsRasterLayer, it has already setContrastEnhancementAlgorithm(). Default enhancement must only be set if default style was not loaded (to avoid stats calculation).
        qgis::down_cast< QgsSingleBandGrayRenderer * >( renderer.get() )->setContrastEnhancement( ce );
      }
      break;
    }

    case Qgis::RasterDrawingStyle::SingleBandPseudoColor:
    {
      const int bandNo = 1;
      double minValue = 0;
      double maxValue = 0;
      // TODO: avoid calculating statistics if not necessary (default style loaded)
      minMaxValuesForBand( bandNo, provider, minValue, maxValue );
      QgsRasterShader *shader = new QgsRasterShader( minValue, maxValue );
      renderer = std::make_unique< QgsSingleBandPseudoColorRenderer >( provider, bandNo, shader );
      break;
    }
    case Qgis::RasterDrawingStyle::MultiBandColor:
    {
      const QgsSettings s;

      int redBand = s.value( QStringLiteral( "/Raster/defaultRedBand" ), 1 ).toInt();
      if ( redBand < 0 || redBand > provider->bandCount() )
      {
        redBand = -1;
      }
      int greenBand = s.value( QStringLiteral( "/Raster/defaultGreenBand" ), 2 ).toInt();
      if ( greenBand < 0 || greenBand > provider->bandCount() )
      {
        greenBand = -1;
      }
      int blueBand = s.value( QStringLiteral( "/Raster/defaultBlueBand" ), 3 ).toInt();
      if ( blueBand < 0 || blueBand > provider->bandCount() )
      {
        blueBand = -1;
      }

      renderer = std::make_unique< QgsMultiBandColorRenderer >( provider, redBand, greenBand, blueBand );
      break;
    }
    case Qgis::RasterDrawingStyle::SingleBandColorData:
    {
      renderer = std::make_unique< QgsSingleBandColorDataRenderer >( provider, 1 );
      break;
    }
    default:
      return nullptr;
  }

  std::unique_ptr< QgsRasterTransparency > tr = std::make_unique< QgsRasterTransparency >();
  const int bandCount = renderer->usesBands().size();
  if ( bandCount == 1 )
  {
    tr->setTransparentSingleValuePixelList( {} );
  }
  else if ( bandCount == 3 )
  {
    tr->setTransparentThreeValuePixelList( {} );
  }
  renderer->setRasterTransparency( tr.release() );
  return renderer.release();
}

bool QgsRasterRendererRegistry::minMaxValuesForBand( int band, QgsRasterDataProvider *provider, double &minValue, double &maxValue ) const
{
  if ( !provider )
  {
    return false;
  }

  minValue = 0;
  maxValue = 0;

  const QgsSettings s;
  if ( s.value( QStringLiteral( "/Raster/useStandardDeviation" ), false ).toBool() )
  {
    const QgsRasterBandStats stats = provider->bandStatistics( band, Qgis::RasterBandStatistic::Mean | Qgis::RasterBandStatistic::StdDev );

    const double stdDevFactor = s.value( QStringLiteral( "/Raster/defaultStandardDeviation" ), 2.0 ).toDouble();
    const double diff = stdDevFactor * stats.stdDev;
    minValue = stats.mean - diff;
    maxValue = stats.mean + diff;
  }
  else
  {
    const QgsRasterBandStats stats = provider->bandStatistics( band, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max );
    minValue = stats.minimumValue;
    maxValue = stats.maximumValue;
  }
  return true;
}
