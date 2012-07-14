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

QgsRasterRendererRegistry* QgsRasterRendererRegistry::mInstance = 0;

QgsRasterRendererRegistry* QgsRasterRendererRegistry::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsRasterRendererRegistry();
  }
  return mInstance;
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
  // return QStringList( mEntries.keys() );
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

QgsRasterRenderer* QgsRasterRendererRegistry::defaultRendererForDrawingStyle( const QgsRasterLayer::DrawingStyle&  theDrawingStyle, QgsRasterDataProvider* provider ) const
{
  if ( !provider || provider->bandCount() < 1 )
  {
    return 0;
  }


  QgsRasterRenderer* renderer = 0;
  switch ( theDrawingStyle )
  {
    case QgsRasterLayer::PalettedColor:
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
      for ( ; colorIt != colorEntries.constEnd(); ++colorIt )
      {
        colorArray[( int )( colorIt->value )] = colorIt->color;
      }

      renderer = new QgsPalettedRasterRenderer( provider,
          grayBand,
          colorArray,
          colorArraySize );
    }
    break;
    case QgsRasterLayer::MultiBandSingleBandGray:
    case QgsRasterLayer::SingleBandGray:
    {
      int grayBand = 1;
      renderer = new QgsSingleBandGrayRenderer( provider, grayBand );
      QgsContrastEnhancement* ce = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
            provider->dataType( grayBand ) ) );

      QSettings s;
      QgsContrastEnhancement::ContrastEnhancementAlgorithm ceAlgorithm;
      ceAlgorithm = ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )QgsRasterRendererRegistry::contrastEnhancementFromString(
                      s.value( "/Raster/defaultContrastEnhancementAlgorithm", "NoEnhancement" ).toString() );
      ce->setContrastEnhancementAlgorithm( ceAlgorithm );

      if ( ceAlgorithm != QgsContrastEnhancement::NoEnhancement )
      {
        double minValue = 0;
        double maxValue = 0;
        minMaxValuesForBand( grayBand, provider, minValue, maxValue );
        ce->setMinimumValue( minValue );
        ce->setMaximumValue( maxValue );
      }
      (( QgsSingleBandGrayRenderer* )renderer )->setContrastEnhancement( ce );
      break;
    }
    case QgsRasterLayer::SingleBandPseudoColor:
    {
      int bandNo = 1;
      double minValue = 0;
      double maxValue = 0;
      minMaxValuesForBand( bandNo, provider, minValue, maxValue );
      QgsRasterShader* shader = new QgsRasterShader( minValue, maxValue );
      renderer = new QgsSingleBandPseudoColorRenderer( provider, bandNo, shader );
      break;
    }
    case QgsRasterLayer::MultiBandColor:
    {
      QSettings s;
      QgsContrastEnhancement::ContrastEnhancementAlgorithm ceAlgorithm;
      ceAlgorithm = ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )QgsRasterRendererRegistry::contrastEnhancementFromString(
                      s.value( "/Raster/defaultContrastEnhancementAlgorithm", "NoEnhancement" ).toString() );

      QgsContrastEnhancement* redEnhancement = 0;
      QgsContrastEnhancement* greenEnhancement = 0;
      QgsContrastEnhancement* blueEnhancement = 0;

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

      double minValue = 0;
      double maxValue = 0;
      if ( ceAlgorithm !=  QgsContrastEnhancement::NoEnhancement )
      {
        if ( redBand > 0 )
        {
          redEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
                provider->dataType( redBand ) ) );
          minMaxValuesForBand( redBand, provider, minValue, maxValue );
          redEnhancement->setMinimumValue( minValue );
          redEnhancement->setMaximumValue( maxValue );
          redEnhancement->setContrastEnhancementAlgorithm( ceAlgorithm );
        }

        if ( greenBand > 0 )
        {
          greenEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
                provider->dataType( greenBand ) ) );
          minMaxValuesForBand( greenBand, provider, minValue, maxValue );
          greenEnhancement->setMinimumValue( minValue );
          greenEnhancement->setMaximumValue( maxValue );
          greenEnhancement->setContrastEnhancementAlgorithm( ceAlgorithm );
        }

        if ( blueBand > 0 )
        {
          blueEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
                provider->dataType( blueBand ) ) );
          minMaxValuesForBand( blueBand, provider, minValue, maxValue );
          blueEnhancement->setMinimumValue( minValue );
          blueEnhancement->setMaximumValue( maxValue );
          blueEnhancement->setContrastEnhancementAlgorithm( ceAlgorithm );
        }
      }

      renderer = new QgsMultiBandColorRenderer( provider, redBand, greenBand, blueBand,
          redEnhancement, greenEnhancement, blueEnhancement );
      break;
    }
    case QgsRasterLayer::SingleBandColorDataStyle:
    {
      renderer = new QgsSingleBandColorDataRenderer( provider, 1 );
      break;
    }
    default:
      break;
  }

  QgsRasterTransparency* tr = new QgsRasterTransparency(); //renderer takes ownership
  int bandCount = renderer->usesBands().size();
  if ( bandCount == 1 )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleList;
    QgsRasterTransparency::TransparentSingleValuePixel singleEntry;
    singleEntry.pixelValue = provider->noDataValue();
    singleEntry.percentTransparent = 100;
    transparentSingleList.push_back( singleEntry );
    tr->setTransparentSingleValuePixelList( transparentSingleList );
  }
  else if ( bandCount == 3 )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValueList;
    QgsRasterTransparency::TransparentThreeValuePixel threeValueEntry;
    threeValueEntry.red = provider->noDataValue();
    threeValueEntry.green = provider->noDataValue();
    threeValueEntry.blue = provider->noDataValue();
    threeValueEntry.percentTransparent = 100;
    transparentThreeValueList.push_back( threeValueEntry );
    tr->setTransparentThreeValuePixelList( transparentThreeValueList );
  }
  renderer->setRasterTransparency( tr );
#if 0
  if ( !renderer )
  {
    return;
  }

  renderer->setOpacity( mTransparencyLevel / 255.0 );

  QgsRasterTransparency* tr = new QgsRasterTransparency(); //renderer takes ownership
  if ( mDataProvider->bandCount() == 1 )
  {
    tr->setTransparentSingleValuePixelList( mRasterTransparency.transparentSingleValuePixelList() );
  }
  else if ( mDataProvider->bandCount() == 3 )
  {
    tr->setTransparentThreeValuePixelList( mRasterTransparency.transparentThreeValuePixelList() );
  }
  renderer->setRasterTransparency( tr );

  if ( mTransparencyBandName != TRSTRING_NOT_SET )
  {
    int tBand = bandNumber( mTransparencyBandName );
    if ( tBand > 0 )
    {
      renderer->setAlphaBand( tBand );
    }
  }
  renderer->setInvertColor( mInvertColor );
#endif //0
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
    double stdDevFactor = s.value( "/Raster/defaultStandardDeviation", 2.0 ).toDouble();
    QgsRasterBandStats stats = provider->bandStatistics( band );
    double diff = stdDevFactor * stats.stdDev;
    minValue = stats.mean - diff;
    maxValue = stats.mean + diff;
  }
  else
  {
    minValue = provider->minimumValue( band );
    maxValue = provider->maximumValue( band );
  }
  return true;
}

int QgsRasterRendererRegistry::contrastEnhancementFromString( const QString& contrastEnhancementString )
{
  if ( contrastEnhancementString == "StretchToMinimumMaximum" )
  {
    return ( int )QgsContrastEnhancement::StretchToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "StretchAndClipToMinimumMaximum" )
  {
    return ( int )QgsContrastEnhancement::StretchAndClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "ClipToMinimumMaximum" )
  {
    return ( int )QgsContrastEnhancement::ClipToMinimumMaximum;
  }
  else
  {
    return ( int )QgsContrastEnhancement::NoEnhancement;
  }
}


