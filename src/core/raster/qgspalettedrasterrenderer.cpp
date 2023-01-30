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
#include "qgsstyleentityvisitor.h"
#include "qgsmessagelog.h"
#include "qgsrasteriterator.h"
#include "qgslayertreemodellegendnode.h"
#include "qgscolorrampimpl.h"
#include "qgsrasterattributetable.h"

#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QVector>
#include <memory>
#include <set>
#include <QRegularExpression>
#include <QTextStream>

const int QgsPalettedRasterRenderer::MAX_FLOAT_CLASSES = 65536;

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes )
  : QgsRasterRenderer( input, QStringLiteral( "paletted" ) )
  , mBand( bandNumber )
{
  for ( const Class &klass : std::as_const( classes ) )
  {
    MultiValueClassData::iterator it = std::find_if( mMultiValueClassData.begin(), mMultiValueClassData.end(), [&klass]( const MultiValueClass & val ) -> bool
    {
      return val.label == klass.label && val.color == klass.color ;
    } );
    if ( it != mMultiValueClassData.end() )
    {
      it->values.push_back( klass.value );
    }
    else
    {
      mMultiValueClassData.push_back( MultiValueClass{ { klass.value }, klass.color, klass.label } );
    }
  }
  updateArrays();
}

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const MultiValueClassData &classes )
  : QgsRasterRenderer( input, QStringLiteral( "paletted" ) )
  , mBand( bandNumber )
  , mMultiValueClassData( classes )
{
  updateArrays();
}

QgsPalettedRasterRenderer *QgsPalettedRasterRenderer::clone() const
{

  std::unique_ptr< QgsPalettedRasterRenderer > renderer = std::make_unique< QgsPalettedRasterRenderer >( nullptr, mBand, mMultiValueClassData );

  if ( mSourceColorRamp )
    renderer->setSourceColorRamp( mSourceColorRamp->clone() );

  renderer->copyCommonProperties( this );

  return renderer.release();
}

Qgis::RasterRendererFlags QgsPalettedRasterRenderer::flags() const
{
  return Qgis::RasterRendererFlag::InternalLayerOpacityHandling;
}

QgsRasterRenderer *QgsPalettedRasterRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const int bandNumber = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  ClassData classData;

  const QDomElement paletteElem = elem.firstChildElement( QStringLiteral( "colorPalette" ) );
  if ( !paletteElem.isNull() )
  {
    const QDomNodeList paletteEntries = paletteElem.elementsByTagName( QStringLiteral( "paletteEntry" ) );

    QDomElement entryElem;
    double value;

    for ( int i = 0; i < paletteEntries.size(); ++i )
    {
      QColor color;
      QString label;
      entryElem = paletteEntries.at( i ).toElement();
      value = entryElem.attribute( QStringLiteral( "value" ), QStringLiteral( "0" ) ).toDouble();
      color = QColor( entryElem.attribute( QStringLiteral( "color" ), QStringLiteral( "#000000" ) ) );
      color.setAlpha( entryElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt() );
      label = entryElem.attribute( QStringLiteral( "label" ) );
      QgsDebugMsgLevel( QStringLiteral( "Value: %1, label: %2, color: %3" ).arg( value ).arg( label, entryElem.attribute( QStringLiteral( "color" ) ) ), 4 );
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
  return classData();
}

QgsPalettedRasterRenderer::MultiValueClassData QgsPalettedRasterRenderer::multiValueClasses() const
{
  return mMultiValueClassData;
}

void QgsPalettedRasterRenderer::setMultiValueClasses( const MultiValueClassData &classes )
{
  mMultiValueClassData = classes;
  updateArrays();
}

QString QgsPalettedRasterRenderer::label( double idx ) const
{
  if ( ! mMultiValueClassData.isEmpty() )
  {
    const auto constMClassData = mMultiValueClassData;
    for ( const MultiValueClass &c : std::as_const( constMClassData ) )
    {
      if ( c.values.contains( idx ) )
        return c.label;
    }
  }

  return QString();
}

void QgsPalettedRasterRenderer::setLabel( double idx, const QString &label )
{
  MultiValueClassData::iterator cMvIt = mMultiValueClassData.begin();
  for ( ; cMvIt != mMultiValueClassData.end(); ++cMvIt )
  {
    if ( cMvIt->values.contains( idx ) )
    {
      cMvIt->label = label;
      return;
    }
  }
}

QgsRasterBlock *QgsPalettedRasterRenderer::block( int, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || mMultiValueClassData.isEmpty() )
  {
    return outputBlock.release();
  }

  const std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  //rendering is faster without considering user-defined transparency
  const bool hasTransparency = usesTransparency();

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

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  const QRgb myDefaultColor = renderColorForNodataPixel();

  //use direct data access instead of QgsRasterBlock::setValue
  //because of performance
  Q_ASSERT( outputBlock ); // to make cppcheck happy
  unsigned int *outputData = ( unsigned int * )( outputBlock->bits() );

  const qgssize rasterSize = ( qgssize )width * height;
  bool isNoData = false;
  for ( qgssize i = 0; i < rasterSize; ++i )
  {
    const double value = inputBlock->valueAndNoData( i, isNoData );
    if ( isNoData )
    {
      outputData[i] = myDefaultColor;
      continue;
    }
    if ( !mColors.contains( value ) )
    {
      outputData[i] = myDefaultColor;
      continue;
    }

    if ( !hasTransparency )
    {
      outputData[i] = mColors.value( value );
    }
    else
    {
      double currentOpacity = mOpacity;
      if ( mRasterTransparency )
      {
        currentOpacity = mRasterTransparency->alphaValue( value, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentOpacity *= alphaBlock->value( i ) / 255.0;
      }

      const QRgb c = mColors.value( value );
      outputData[i] = qRgba( currentOpacity * qRed( c ), currentOpacity * qGreen( c ), currentOpacity * qBlue( c ), currentOpacity * qAlpha( c ) );
    }
  }

  return outputBlock.release();
}

int QgsPalettedRasterRenderer::nColors() const
{
  return mMultiValueClassData.size();
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
  const ClassData klassData { classData() };
  ClassData::const_iterator it = klassData.constBegin();
  for ( ; it != klassData.constEnd(); ++it )
  {
    const QColor color = it->color;
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
    const QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mSourceColorRamp.get(), doc );
    rasterRendererElem.appendChild( colorRampElem );
  }

  parentElem.appendChild( rasterRendererElem );
}

void QgsPalettedRasterRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // create base structure
  QgsRasterRenderer::toSld( doc, element, props );

  // look for RasterSymbolizer tag
  const QDomNodeList elements = element.elementsByTagName( QStringLiteral( "sld:RasterSymbolizer" ) );
  if ( elements.size() == 0 )
    return;

  // there SHOULD be only one
  QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

  // add Channel Selection tags
  QDomElement channelSelectionElem = doc.createElement( QStringLiteral( "sld:ChannelSelection" ) );
  rasterSymbolizerElem.appendChild( channelSelectionElem );

  // for the mapped band
  QDomElement channelElem = doc.createElement( QStringLiteral( "sld:GrayChannel" ) );
  channelSelectionElem.appendChild( channelElem );

  // set band
  QDomElement sourceChannelNameElem = doc.createElement( QStringLiteral( "sld:SourceChannelName" ) );
  sourceChannelNameElem.appendChild( doc.createTextNode( QString::number( band() ) ) );
  channelElem.appendChild( sourceChannelNameElem );

  // add ColorMap tag
  QDomElement colorMapElem = doc.createElement( QStringLiteral( "sld:ColorMap" ) );
  colorMapElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "values" ) );
  if ( this->classes().size() >= 255 )
    colorMapElem.setAttribute( QStringLiteral( "extended" ), QStringLiteral( "true" ) );
  rasterSymbolizerElem.appendChild( colorMapElem );

  // for each color set a ColorMapEntry tag nested into "sld:ColorMap" tag
  // e.g. <ColorMapEntry color="#EEBE2F" quantity="-300" label="label" opacity="0"/>
  const QList<QgsPalettedRasterRenderer::Class> classes = this->classes();
  QList<QgsPalettedRasterRenderer::Class>::const_iterator classDataIt = classes.constBegin();
  for ( ; classDataIt != classes.constEnd();  ++classDataIt )
  {
    QDomElement colorMapEntryElem = doc.createElement( QStringLiteral( "sld:ColorMapEntry" ) );
    colorMapElem.appendChild( colorMapEntryElem );

    // set colorMapEntryElem attributes
    colorMapEntryElem.setAttribute( QStringLiteral( "color" ), classDataIt->color.name() );
    colorMapEntryElem.setAttribute( QStringLiteral( "quantity" ), QString::number( classDataIt->value ) );
    colorMapEntryElem.setAttribute( QStringLiteral( "label" ), classDataIt->label );
    if ( classDataIt->color.alphaF() != 1.0 )
    {
      colorMapEntryElem.setAttribute( QStringLiteral( "opacity" ), QString::number( classDataIt->color.alphaF() ) );
    }
  }
}

bool QgsPalettedRasterRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mSourceColorRamp )
  {
    QgsStyleColorRampEntity entity( mSourceColorRamp.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }

  return true;
}

QList< QPair< QString, QColor > > QgsPalettedRasterRenderer::legendSymbologyItems() const
{
  QList< QPair< QString, QColor > > symbolItems;
  for ( const QgsPalettedRasterRenderer::MultiValueClass &classData : mMultiValueClassData )
  {
    QString lab { classData.label };
    if ( lab.isEmpty() )
    {
      QStringList values;
      for ( const QVariant &val : std::as_const( classData.values ) )
      {
        // Be tolerant here: if we can convert it to double use locale, if not just pass through.
        bool ok;
        const double numericValue { val.toDouble( &ok ) };
        if ( ok )
        {
          values.push_back( QLocale().toString( numericValue ) );
        }
        else
        {
          values.push_back( val.toString() );
        }
      }
      lab = values.join( QChar( ' ' ) );
    }
    symbolItems << qMakePair( lab, classData.color );
  }
  return symbolItems;
}


QList<QgsLayerTreeModelLegendNode *> QgsPalettedRasterRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> res;

  const QString name = displayBandName( mBand );
  if ( !name.isEmpty() )
  {
    res << new QgsSimpleLegendNode( nodeLayer, name );
  }

  const QList< QPair< QString, QColor > > items = legendSymbologyItems();
  res.reserve( res.size() + items.size() );
  for ( const QPair< QString, QColor > &item : items )
  {
    res << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
  }

  return res;
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
    classes << QgsPalettedRasterRenderer::Class( colorIt->value, colorIt->color, colorIt->label );
  }
  return classes;
}

QgsPalettedRasterRenderer::MultiValueClassData QgsPalettedRasterRenderer::rasterAttributeTableToClassData( const QgsRasterAttributeTable *attributeTable, const int classificationColumn, QgsColorRamp *ramp )
{
  if ( ! attributeTable || ! attributeTable->isValid() )
  {
    return QgsPalettedRasterRenderer::MultiValueClassData();
  }

  QgsPalettedRasterRenderer::MultiValueClassData classData;

  const QList<QgsRasterAttributeTable::MinMaxClass> minMaxClasses { attributeTable->minMaxClasses( classificationColumn ) };
  if ( minMaxClasses.empty() )
    return QgsPalettedRasterRenderer::MultiValueClassData();

  for ( const QgsRasterAttributeTable::MinMaxClass &minMaxClass : std::as_const( minMaxClasses ) )
  {
    QVector<QVariant> values;
    for ( const double val : std::as_const( minMaxClass.minMaxValues ) )
    {
      values.push_back( QVariant( val ) );
    }
    classData.push_back( { values, minMaxClass.color, minMaxClass.name  } );
  }

  int numClasses { static_cast<int>( classData.count( ) ) };

  // assign colors from ramp
  if ( ramp && numClasses > 0 )
  {
    int i = 0;

    if ( QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp ) )
    {
      //ramp is a random colors ramp, so inform it of the total number of required colors
      //this allows the ramp to pregenerate a set of visually distinctive colors
      randomRamp->setTotalColorCount( numClasses );
    }

    if ( numClasses > 1 )
      numClasses -= 1; //avoid duplicate first color

    QgsPalettedRasterRenderer::MultiValueClassData::iterator cIt = classData.begin();
    for ( ; cIt != classData.end(); ++cIt )
    {
      cIt->color = ramp->color( i / static_cast<double>( numClasses ) );
      i++;
    }
  }

  return classData;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classDataFromString( const QString &string )
{
  QgsPalettedRasterRenderer::ClassData classes;

  const QRegularExpression linePartRx( QStringLiteral( "[\\s,:]+" ) );

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  const QStringList parts = string.split( '\n', QString::SkipEmptyParts );
#else
  const QStringList parts = string.split( '\n', Qt::SkipEmptyParts );
#endif
  for ( const QString &part : parts )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList lineParts = part.split( linePartRx, QString::SkipEmptyParts );
#else
    const QStringList lineParts = part.split( linePartRx, Qt::SkipEmptyParts );
#endif
    bool ok = false;
    switch ( lineParts.count() )
    {
      case 1:
      {
        const int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        classes << Class( value );
        break;
      }

      case 2:
      {
        const int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        const QColor c( lineParts.at( 1 ) );

        classes << Class( value, c );
        break;
      }

      default:
      {
        if ( lineParts.count() < 4 )
          continue;

        const int value = lineParts.at( 0 ).toInt( &ok );
        if ( !ok )
          continue;

        bool rOk = false;
        const double r = lineParts.at( 1 ).toDouble( &rOk );
        bool gOk = false;
        const double g = lineParts.at( 2 ).toDouble( &gOk );
        bool bOk = false;
        const double b = lineParts.at( 3 ).toDouble( &bOk );

        QColor c;
        if ( rOk && gOk && bOk )
        {
          c = QColor( r, g, b );
        }

        if ( lineParts.count() >= 5 )
        {
          const double alpha = lineParts.at( 4 ).toDouble( &ok );
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

  const auto constCd = cd;
  for ( const Class &c : constCd )
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

  ClassData data;

  if ( bandNumber > 0 && bandNumber <= raster->bandCount() )
  {
    qlonglong numClasses = 0;

    if ( feedback )
      feedback->setProgress( 0 );

    // Collect unique values for float rasters
    if ( raster->dataType( bandNumber ) == Qgis::DataType::Float32 || raster->dataType( bandNumber ) == Qgis::DataType::Float64 )
    {

      if ( feedback && feedback->isCanceled() )
      {
        return data;
      }

      std::set<double> values;

      const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
      const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;

      QgsRasterIterator iter( raster );
      iter.startRasterRead( bandNumber, raster->xSize(), raster->ySize(), raster->extent(), feedback );

      const int nbBlocksWidth = static_cast< int >( std::ceil( 1.0 * raster->xSize() / maxWidth ) );
      const int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * raster->ySize() / maxHeight ) );
      const int nbBlocks = nbBlocksWidth * nbBlocksHeight;

      int iterLeft = 0;
      int iterTop = 0;
      int iterCols = 0;
      int iterRows = 0;
      std::unique_ptr< QgsRasterBlock > rasterBlock;
      QgsRectangle blockExtent;
      bool isNoData = false;
      while ( iter.readNextRasterPart( bandNumber, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
      {
        if ( feedback )
          feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );

        if ( feedback && feedback->isCanceled() )
          break;

        for ( int row = 0; row < iterRows; row++ )
        {
          if ( feedback && feedback->isCanceled() )
            break;

          for ( int column = 0; column < iterCols; column++ )
          {
            if ( feedback && feedback->isCanceled() )
              break;

            const double currentValue = rasterBlock->valueAndNoData( row, column, isNoData );
            if ( numClasses >= MAX_FLOAT_CLASSES )
            {
              QgsMessageLog::logMessage( QStringLiteral( "Number of classes exceeded maximum (%1)." ).arg( MAX_FLOAT_CLASSES ), QStringLiteral( "Raster" ) );
              break;
            }
            if ( !isNoData && values.find( currentValue ) == values.end() )
            {
              values.insert( currentValue );
              data.push_back( Class( currentValue, QColor(), QLocale().toString( currentValue ) ) );
              numClasses++;
            }
          }
        }
      }
      // must be sorted
      std::sort( data.begin(), data.end(), []( const Class & a, const Class & b ) -> bool
      {
        return a.value < b.value;
      } );
    }
    else
    {
      // get min and max value from raster
      const QgsRasterBandStats stats = raster->bandStatistics( bandNumber, QgsRasterBandStats::Min | QgsRasterBandStats::Max, QgsRectangle(), 0, feedback );
      if ( feedback && feedback->isCanceled() )
        return ClassData();

      const double min = stats.minimumValue;
      const double max = stats.maximumValue;
      // need count of every individual value
      const int bins = std::ceil( max - min ) + 1;
      if ( bins <= 0 )
        return ClassData();

      const QgsRasterHistogram histogram = raster->histogram( bandNumber, bins, min, max, QgsRectangle(), 0, false, feedback );
      if ( feedback && feedback->isCanceled() )
        return ClassData();

      const double interval = ( histogram.maximum - histogram.minimum + 1 ) / histogram.binCount;
      double currentValue = histogram.minimum;
      for ( int idx = 0; idx < histogram.binCount; ++idx )
      {
        const int count = histogram.histogramVector.at( idx );
        if ( count > 0 )
        {
          data << Class( currentValue, QColor(), QLocale().toString( currentValue ) );
          numClasses++;
        }
        currentValue += interval;
      }
    }

    // assign colors from ramp
    if ( ramp && numClasses > 0 )
    {
      int i = 0;

      if ( QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp ) )
      {
        //ramp is a random colors ramp, so inform it of the total number of required colors
        //this allows the ramp to pregenerate a set of visually distinctive colors
        randomRamp->setTotalColorCount( data.count() );
      }

      if ( numClasses > 1 )
        numClasses -= 1; //avoid duplicate first color

      QgsPalettedRasterRenderer::ClassData::iterator cIt = data.begin();
      for ( ; cIt != data.end(); ++cIt )
      {
        if ( feedback )
        {
          // Show no less than 1%, then the max between class fill and real progress
          feedback->setProgress( std::max<int>( 1, 100 * ( i + 1 ) / numClasses ) );
        }
        cIt->color = ramp->color( i / static_cast<double>( numClasses ) );
        i++;
      }
    }
  }
  return data;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classData() const
{
  QgsPalettedRasterRenderer::ClassData data;
  for ( const MultiValueClass &klass : std::as_const( mMultiValueClassData ) )
  {
    for ( const QVariant &entry : std::as_const( klass.values ) )
    {
      bool ok;
      const double value { entry.toDouble( &ok )};
      if ( ok )
      {
        data.push_back( { value, klass.color, klass.label } );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Could not convert class value '%1' to double when creating classes." ).arg( entry.toString() ), 2 );
      }
    }
  }
  return data;
}

void QgsPalettedRasterRenderer::updateArrays()
{
  mColors.clear();

  MultiValueClassData::const_iterator it = mMultiValueClassData.constBegin();
  for ( ; it != mMultiValueClassData.constEnd(); ++it )
  {
    for ( const QVariant &entry : std::as_const( it->values ) )
    {
      bool ok;
      const double value { entry.toDouble( &ok )};
      if ( ok )
      {
        mColors[value] = qPremultiply( it->color.rgba() );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Could not convert class value '%1' to double for color lookup." ).arg( entry.toString() ), 2 );
      }
    }
  }
}

bool QgsPalettedRasterRenderer::canCreateRasterAttributeTable( ) const
{
  return true;
}

QgsPalettedRasterRenderer::MultiValueClass::MultiValueClass( const QVector< QVariant > &values, const QColor &color, const QString &label )
  : values( values )
  , color( color )
  , label( label )
{}
