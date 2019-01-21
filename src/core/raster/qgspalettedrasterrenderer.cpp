/***************************************************************************
                         qgspalettedrasterrenderer.cpp
                         -----------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
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

#include "qgspalettedrasterrenderer.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgssymbollayerutils.h"

#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QVector>
#include <memory>

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes )
  : QgsRasterRenderer( input, QStringLiteral( "paletted" ) )
  , mBand( bandNumber )
  , mClassData( classes )
{
  updateArrays();
}

QgsPalettedRasterRenderer *QgsPalettedRasterRenderer::clone() const
{
  QgsPalettedRasterRenderer *renderer = new QgsPalettedRasterRenderer( nullptr, mBand, mClassData );
  if ( mSourceColorRamp )
    renderer->setSourceColorRamp( mSourceColorRamp->clone() );

  renderer->copyCommonProperties( this );
  return renderer;
}

QgsRasterRenderer *QgsPalettedRasterRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int bandNumber = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  ClassData classData;

  QDomElement paletteElem = elem.firstChildElement( QStringLiteral( "colorPalette" ) );
  if ( !paletteElem.isNull() )
  {
    QDomNodeList paletteEntries = paletteElem.elementsByTagName( QStringLiteral( "paletteEntry" ) );

    QDomElement entryElem;
    int value;

    for ( int i = 0; i < paletteEntries.size(); ++i )
    {
      QColor color;
      QString label;
      entryElem = paletteEntries.at( i ).toElement();
      value = static_cast<int>( entryElem.attribute( QStringLiteral( "value" ), QStringLiteral( "0" ) ).toDouble() );
      QgsDebugMsgLevel( entryElem.attribute( "color", "#000000" ), 4 );
      color = QColor( entryElem.attribute( QStringLiteral( "color" ), QStringLiteral( "#000000" ) ) );
      color.setAlpha( entryElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt() );
      label = entryElem.attribute( QStringLiteral( "label" ) );
      classData << Class( value, color, label );
    }
  }

  QgsPalettedRasterRenderer *r = new QgsPalettedRasterRenderer( input, bandNumber, classData );
  r->readXml( elem );

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = elem.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  return r;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classes() const
{
  return mClassData;
}

QString QgsPalettedRasterRenderer::label( int idx ) const
{
  Q_FOREACH ( const Class &c, mClassData )
  {
    if ( c.value == idx )
      return c.label;
  }

  return QString();
}

void QgsPalettedRasterRenderer::setLabel( int idx, const QString &label )
{
  ClassData::iterator cIt = mClassData.begin();
  for ( ; cIt != mClassData.end(); ++cIt )
  {
    if ( cIt->value == idx )
    {
      cIt->label = label;
      return;
    }
  }
}

QgsRasterBlock *QgsPalettedRasterRenderer::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || mClassData.isEmpty() )
  {
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNo, extent, width, height, feedback ) );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  double currentOpacity = mOpacity;

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand == mBand )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  QRgb myDefaultColor = NODATA_COLOR;

  //use direct data access instead of QgsRasterBlock::setValue
  //because of performance
  unsigned int *outputData = ( unsigned int * )( outputBlock->bits() );

  qgssize rasterSize = ( qgssize )width * height;
  bool isNoData = false;
  for ( qgssize i = 0; i < rasterSize; ++i )
  {
    const double value = inputBlock->valueAndNoData( i, isNoData );
    if ( isNoData )
    {
      outputData[i] = myDefaultColor;
      continue;
    }
    int val = static_cast< int >( value );
    if ( !mColors.contains( val ) )
    {
      outputData[i] = myDefaultColor;
      continue;
    }

    if ( !hasTransparency )
    {
      outputData[i] = mColors.value( val );
    }
    else
    {
      currentOpacity = mOpacity;
      if ( mRasterTransparency )
      {
        currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentOpacity *= alphaBlock->value( i ) / 255.0;
      }

      QRgb c = mColors.value( val );
      outputData[i] = qRgba( currentOpacity * qRed( c ), currentOpacity * qGreen( c ), currentOpacity * qBlue( c ), currentOpacity * qAlpha( c ) );
    }
  }

  return outputBlock.release();
}

void QgsPalettedRasterRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  QDomElement colorPaletteElem = doc.createElement( QStringLiteral( "colorPalette" ) );
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    QColor color = it->color;
    QDomElement colorElem = doc.createElement( QStringLiteral( "paletteEntry" ) );
    colorElem.setAttribute( QStringLiteral( "value" ), it->value );
    colorElem.setAttribute( QStringLiteral( "color" ), color.name() );
    colorElem.setAttribute( QStringLiteral( "alpha" ), color.alpha() );
    if ( !it->label.isEmpty() )
    {
      colorElem.setAttribute( QStringLiteral( "label" ), it->label );
    }
    colorPaletteElem.appendChild( colorElem );
  }
  rasterRendererElem.appendChild( colorPaletteElem );

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mSourceColorRamp.get(), doc );
    rasterRendererElem.appendChild( colorRampElem );
  }

  parentElem.appendChild( rasterRendererElem );
}

void QgsPalettedRasterRenderer::legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const
{
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    QString lab = it->label.isEmpty() ? QString::number( it->value ) : it->label;
    symbolItems << qMakePair( lab, it->color );
  }
}

QList<int> QgsPalettedRasterRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}

void QgsPalettedRasterRenderer::setSourceColorRamp( QgsColorRamp *ramp )
{
  mSourceColorRamp.reset( ramp );
}

QgsColorRamp *QgsPalettedRasterRenderer::sourceColorRamp() const
{
  return mSourceColorRamp.get();
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::colorTableToClassData( const QList<QgsColorRampShader::ColorRampItem> &table )
{
  QList<QgsColorRampShader::ColorRampItem>::const_iterator colorIt = table.constBegin();
  QgsPalettedRasterRenderer::ClassData classes;
  for ( ; colorIt != table.constEnd(); ++colorIt )
  {
    int idx = ( int )( colorIt->value );
    classes << QgsPalettedRasterRenderer::Class( idx, colorIt->color, colorIt->label );
  }
  return classes;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classDataFromString( const QString &string )
{
  QgsPalettedRasterRenderer::ClassData classes;

  QRegularExpression linePartRx( QStringLiteral( "[\\s,:]+" ) );

  QStringList parts = string.split( '\n', QString::SkipEmptyParts );
  Q_FOREACH ( const QString &part, parts )
  {
    QStringList lineParts = part.split( linePartRx, QString::SkipEmptyParts );
    bool ok = false;
    switch ( lineParts.count() )
    {
      case 1:
      {
        int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        classes << Class( value );
        break;
      }

      case 2:
      {
        int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        QColor c( lineParts.at( 1 ) );

        classes << Class( value, c );
        break;
      }

      default:
      {
        if ( lineParts.count() < 4 )
          continue;

        int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        bool rOk = false;
        double r = lineParts.at( 1 ).toDouble( &rOk );
        bool gOk = false;
        double g = lineParts.at( 2 ).toDouble( &gOk );
        bool bOk = false;
        double b = lineParts.at( 3 ).toDouble( &bOk );

        QColor c;
        if ( rOk && gOk && bOk )
        {
          c = QColor( r, g, b );
        }

        if ( lineParts.count() >= 5 )
        {
          double alpha = lineParts.at( 4 ).toDouble( &ok );
          if ( ok )
            c.setAlpha( alpha );
        }

        QString label;
        if ( lineParts.count() > 5 )
        {
          label = lineParts.mid( 5 ).join( ' ' );
        }

        classes << Class( value, c, label );
        break;
      }
    }

  }
  return classes;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classDataFromFile( const QString &path )
{
  QFile inputFile( path );
  QString input;
  if ( inputFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream in( &inputFile );
    input = in.readAll();
    inputFile.close();
  }
  return classDataFromString( input );
}

QString QgsPalettedRasterRenderer::classDataToString( const QgsPalettedRasterRenderer::ClassData &classes )
{
  QStringList out;
  // must be sorted
  QgsPalettedRasterRenderer::ClassData cd = classes;
  std::sort( cd.begin(), cd.end(), []( const Class & a, const Class & b ) -> bool
  {
    return a.value < b.value;
  } );

  Q_FOREACH ( const Class &c, cd )
  {
    out << QStringLiteral( "%1 %2 %3 %4 %5 %6" ).arg( c.value ).arg( c.color.red() )
        .arg( c.color.green() ).arg( c.color.blue() ).arg( c.color.alpha() ).arg( c.label );
  }
  return out.join( '\n' );
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classDataFromRaster( QgsRasterInterface *raster, int bandNumber, QgsColorRamp *ramp, QgsRasterBlockFeedback *feedback )
{
  if ( !raster )
    return ClassData();

  // get min and max value from raster
  QgsRasterBandStats stats = raster->bandStatistics( bandNumber, QgsRasterBandStats::Min | QgsRasterBandStats::Max, QgsRectangle(), 0, feedback );
  if ( feedback && feedback->isCanceled() )
    return ClassData();

  double min = stats.minimumValue;
  double max = stats.maximumValue;
  // need count of every individual value
  int bins = std::ceil( max - min ) + 1;
  if ( bins <= 0 )
    return ClassData();

  QgsRasterHistogram histogram = raster->histogram( bandNumber, bins, min, max, QgsRectangle(), 0, false, feedback );
  if ( feedback && feedback->isCanceled() )
    return ClassData();

  double interval = ( histogram.maximum - histogram.minimum + 1 ) / histogram.binCount;

  ClassData data;

  double currentValue = histogram.minimum;
  double presentValues = 0;
  for ( int idx = 0; idx < histogram.binCount; ++idx )
  {
    int count = histogram.histogramVector.at( idx );
    if ( count > 0 )
    {
      data << Class( currentValue, QColor(), QString::number( currentValue ) );
      presentValues++;
    }
    currentValue += interval;
  }

  // assign colors from ramp
  if ( ramp )
  {
    int i = 0;

    if ( QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp ) )
    {
      //ramp is a random colors ramp, so inform it of the total number of required colors
      //this allows the ramp to pregenerate a set of visually distinctive colors
      randomRamp->setTotalColorCount( data.count() );
    }

    if ( presentValues > 1 )
      presentValues -= 1; //avoid duplicate first color

    QgsPalettedRasterRenderer::ClassData::iterator cIt = data.begin();
    for ( ; cIt != data.end(); ++cIt )
    {
      cIt->color = ramp->color( i / presentValues );
      i++;
    }
  }
  return data;
}

void QgsPalettedRasterRenderer::updateArrays()
{
  mColors.clear();
  int i = 0;
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    mColors[it->value] = qPremultiply( it->color.rgba() );
    i++;
  }
}
