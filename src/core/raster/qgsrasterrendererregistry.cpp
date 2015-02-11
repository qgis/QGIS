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
#include "qgsmultibandcolorrenderer.h"
#include "qgspalettedrasterrenderer.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include <QSettings>

QgsRasterRendererRegistryEntry::QgsRasterRendererRegistryEntry( const QString& theName, const QString& theVisibleName,
    QgsRasterRendererCreateFunc rendererFunction,
    QgsRasterRendererWidgetCreateFunc widgetFunction ):
    name( theName ), visibleName( theVisibleName ), rendererCreateFunction( rendererFunction ),
    widgetCreateFunction( widgetFunction )
{
}

QgsRasterRendererRegistryEntry::QgsRasterRendererRegistryEntry(): rendererCreateFunction( 0 ), widgetCreateFunction( 0 )
{
}

QgsRasterRendererRegistry* QgsRasterRendererRegistry::instance()
{
  static QgsRasterRendererRegistry mInstance;
  return &mInstance;
}

QgsRasterRendererRegistry::QgsRasterRendererRegistry()
{
  // insert items in a particular order, which is returned in renderersList()
  insert( QgsRasterRendererRegistryEntry( "multibandcolor", QObject::tr( "Multiband color" ),
                                          QgsMultiBandColorRenderer::create, 0 ) );
  insert( QgsRasterRendererRegistryEntry( "paletted", QObject::tr( "Paletted" ), QgsPalettedRasterRenderer::create, 0 ) );
  insert( QgsRasterRendererRegistryEntry( "singlebandgray", QObject::tr( "Singleband gray" ),
                                          QgsSingleBandGrayRenderer::create, 0 ) );
  insert( QgsRasterRendererRegistryEntry( "singlebandpseudocolor", QObject::tr( "Singleband pseudocolor" ),
                                          QgsSingleBandPseudoColorRenderer::create, 0 ) );
  insert( QgsRasterRendererRegistryEntry( "singlebandcolordata", QObject::tr( "Singleband color data" ),
                                          QgsSingleBandColorDataRenderer::create, 0 ) );
}

QgsRasterRendererRegistry::~QgsRasterRendererRegistry()
{
}

void QgsRasterRendererRegistry::insert( QgsRasterRendererRegistryEntry entry )
{
  mEntries.insert( entry.name, entry );
  mSortedEntries.append( entry.name );
}

void QgsRasterRendererRegistry::insertWidgetFunction( const QString& rendererName, QgsRasterRendererWidgetCreateFunc func )
{
  if ( !mEntries.contains( rendererName ) )
  {
    return;
  }
  mEntries[rendererName].widgetCreateFunction = func;
}

bool QgsRasterRendererRegistry::rendererData( const QString& rendererName, QgsRasterRendererRegistryEntry& data ) const
{
  QHash< QString, QgsRasterRendererRegistryEntry >::const_iterator it = mEntries.find( rendererName );
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

QgsRasterRenderer* QgsRasterRendererRegistry::defaultRendererForDrawingStyle( const QgsRaster::DrawingStyle&  theDrawingStyle, QgsRasterDataProvider* provider ) const
{
  if ( !provider || provider->bandCount() < 1 )
  {
    return 0;
  }


  QgsRasterRenderer* renderer = 0;
  switch ( theDrawingStyle )
  {
    case QgsRaster::PalettedColor:
    {
      int grayBand = 1; //reasonable default
      QList<QgsColorRampShader::ColorRampItem> colorEntries = provider->colorTable( grayBand );

      //go through list and take maximum value (it could be that entries don't start at 0 or indices are not contiguous)
      int colorArraySize = 0;
      QList<QgsColorRampShader::ColorRampItem>::const_iterator colorIt = colorEntries.constBegin();
      for ( ; colorIt != colorEntries.constEnd(); ++colorIt )
      {
        if ( colorIt->value > colorArraySize )
        {
          colorArraySize = ( int )( colorIt->value );
        }
      }

      colorArraySize += 1; //usually starts at 0
      QColor* colorArray = new QColor[ colorArraySize ];
      colorIt = colorEntries.constBegin();
      QVector<QString> labels;
      for ( ; colorIt != colorEntries.constEnd(); ++colorIt )
      {
        int idx = ( int )( colorIt->value );
        colorArray[idx] = colorIt->color;
        if ( !colorIt->label.isEmpty() )
        {
          if ( labels.size() <= idx ) labels.resize( idx + 1 );
          labels[idx] = colorIt->label;
        }
      }

      renderer = new QgsPalettedRasterRenderer( provider,
          grayBand,
          colorArray,
          colorArraySize,
          labels );
    }
    break;
    case QgsRaster::MultiBandSingleBandGray:
    case QgsRaster::SingleBandGray:
    {
      int grayBand = 1;
      renderer = new QgsSingleBandGrayRenderer( provider, grayBand );

      QgsContrastEnhancement* ce = new QgsContrastEnhancement(( QGis::DataType )(
            provider->dataType( grayBand ) ) );

// Default contrast enhancement is set from QgsRasterLayer, it has already setContrastEnhancementAlgorithm(). Default enhancement must only be set if default style was not loaded (to avoid stats calculation).
      (( QgsSingleBandGrayRenderer* )renderer )->setContrastEnhancement( ce );
      break;
    }
    case QgsRaster::SingleBandPseudoColor:
    {
      int bandNo = 1;
      double minValue = 0;
      double maxValue = 0;
      // TODO: avoid calculating statistics if not necessary (default style loaded)
      minMaxValuesForBand( bandNo, provider, minValue, maxValue );
      QgsRasterShader* shader = new QgsRasterShader( minValue, maxValue );
      renderer = new QgsSingleBandPseudoColorRenderer( provider, bandNo, shader );
      break;
    }
    case QgsRaster::MultiBandColor:
    {
      QSettings s;

      int redBand = s.value( "/Raster/defaultRedBand", 1 ).toInt();
      if ( redBand < 0 || redBand > provider->bandCount() )
      {
        redBand = -1;
      }
      int greenBand = s.value( "/Raster/defaultGreenBand", 2 ).toInt();
      if ( greenBand < 0 || greenBand > provider->bandCount() )
      {
        greenBand = -1;
      }
      int blueBand = s.value( "/Raster/defaultBlueBand", 3 ).toInt();
      if ( blueBand < 0 || blueBand > provider->bandCount() )
      {
        blueBand = -1;
      }

      renderer = new QgsMultiBandColorRenderer( provider, redBand, greenBand, blueBand );
      break;
    }
    case QgsRaster::SingleBandColorDataStyle:
    {
      renderer = new QgsSingleBandColorDataRenderer( provider, 1 );
      break;
    }
    default:
      return 0;
  }

  QgsRasterTransparency* tr = new QgsRasterTransparency(); //renderer takes ownership
  int bandCount = renderer->usesBands().size();
  if ( bandCount == 1 )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleList;
    tr->setTransparentSingleValuePixelList( transparentSingleList );
  }
  else if ( bandCount == 3 )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValueList;
    tr->setTransparentThreeValuePixelList( transparentThreeValueList );
  }
  renderer->setRasterTransparency( tr );
  return renderer;
}

bool QgsRasterRendererRegistry::minMaxValuesForBand( int band, QgsRasterDataProvider* provider, double& minValue, double& maxValue ) const
{
  if ( !provider )
  {
    return false;
  }

  minValue = 0;
  maxValue = 0;

  QSettings s;
  if ( s.value( "/Raster/useStandardDeviation", false ).toBool() )
  {
    QgsRasterBandStats stats = provider->bandStatistics( band, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev );

    double stdDevFactor = s.value( "/Raster/defaultStandardDeviation", 2.0 ).toDouble();
    double diff = stdDevFactor * stats.stdDev;
    minValue = stats.mean - diff;
    maxValue = stats.mean + diff;
  }
  else
  {
    QgsRasterBandStats stats = provider->bandStatistics( band, QgsRasterBandStats::Min | QgsRasterBandStats::Max );
    minValue = stats.minimumValue;
    maxValue = stats.maximumValue;
  }
  return true;
}
