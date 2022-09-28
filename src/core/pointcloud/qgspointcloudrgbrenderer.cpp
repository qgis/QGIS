/***************************************************************************
                         qgspointcloudrgbrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudblock.h"
#include "qgscontrastenhancement.h"

QgsPointCloudRgbRenderer::QgsPointCloudRgbRenderer()
{

}

QString QgsPointCloudRgbRenderer::type() const
{
  return QStringLiteral( "rgb" );
}

QgsPointCloudRenderer *QgsPointCloudRgbRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudRgbRenderer > res = std::make_unique< QgsPointCloudRgbRenderer >();
  res->mRedAttribute = mRedAttribute;
  res->mGreenAttribute = mGreenAttribute;
  res->mBlueAttribute = mBlueAttribute;

  if ( mRedContrastEnhancement )
  {
    res->setRedContrastEnhancement( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  }
  if ( mGreenContrastEnhancement )
  {
    res->setGreenContrastEnhancement( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  }
  if ( mBlueContrastEnhancement )
  {
    res->setBlueContrastEnhancement( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );
  }

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudRgbRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsRectangle visibleExtent = context.renderContext().extent();

  const char *ptr = block->data();
  const int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int redOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mRedAttribute, redOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType redType = attribute->type();

  int greenOffset = 0;
  attribute = request.find( mGreenAttribute, greenOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType greenType = attribute->type();

  int blueOffset = 0;
  attribute = request.find( mBlueAttribute, blueOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType blueType = attribute->type();

  const bool useRedContrastEnhancement = mRedContrastEnhancement && mRedContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useBlueContrastEnhancement = mBlueContrastEnhancement && mBlueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useGreenContrastEnhancement = mGreenContrastEnhancement && mGreenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;

  const bool renderElevation = context.elevationMap();
  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite() || renderElevation;

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();
  for ( int i = 0; i < count; ++i )
  {
    if ( context.renderContext().renderingStopped() )
    {
      break;
    }

    if ( considerZ )
    {
      // z value filtering is cheapest, if we're doing it...
      z = pointZ( context, ptr, i );
      if ( !zRange.contains( z ) )
        continue;
    }

    pointXY( context, ptr, i, x, y );
    if ( visibleExtent.contains( x, y ) )
    {
      if ( reproject )
      {
        try
        {
          ct.transformInPlace( x, y, z );
        }
        catch ( QgsCsException & )
        {
          continue;
        }
      }

      int red = 0;
      context.getAttribute( ptr, i * recordSize + redOffset, redType, red );
      int green = 0;
      context.getAttribute( ptr, i * recordSize + greenOffset, greenType, green );
      int blue = 0;
      context.getAttribute( ptr, i * recordSize + blueOffset, blueType, blue );

      //skip if red, green or blue not in displayable range
      if ( ( useRedContrastEnhancement && !mRedContrastEnhancement->isValueInDisplayableRange( red ) )
           || ( useGreenContrastEnhancement && !mGreenContrastEnhancement->isValueInDisplayableRange( green ) )
           || ( useBlueContrastEnhancement && !mBlueContrastEnhancement->isValueInDisplayableRange( blue ) ) )
      {
        continue;
      }

      //stretch color values
      if ( useRedContrastEnhancement )
      {
        red = mRedContrastEnhancement->enhanceContrast( red );
      }
      if ( useGreenContrastEnhancement )
      {
        green = mGreenContrastEnhancement->enhanceContrast( green );
      }
      if ( useBlueContrastEnhancement )
      {
        blue = mBlueContrastEnhancement->enhanceContrast( blue );
      }

      red = std::max( 0, std::min( 255, red ) );
      green = std::max( 0, std::min( 255, green ) );
      blue = std::max( 0, std::min( 255, blue ) );

      drawPoint( x, y, QColor( red, green, blue ), context );
      if ( renderElevation )
        drawPointToElevationMap( x, y, z, context );
      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}


QgsPointCloudRenderer *QgsPointCloudRgbRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudRgbRenderer > r = std::make_unique< QgsPointCloudRgbRenderer >();

  r->setRedAttribute( element.attribute( QStringLiteral( "red" ), QStringLiteral( "Red" ) ) );
  r->setGreenAttribute( element.attribute( QStringLiteral( "green" ), QStringLiteral( "Green" ) ) );
  r->setBlueAttribute( element.attribute( QStringLiteral( "blue" ), QStringLiteral( "Blue" ) ) );

  r->restoreCommonProperties( element, context );

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  const QDomElement redContrastElem = element.firstChildElement( QStringLiteral( "redContrastEnhancement" ) );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    redContrastEnhancement->readXml( redContrastElem );
    r->setRedContrastEnhancement( redContrastEnhancement );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  const QDomElement greenContrastElem = element.firstChildElement( QStringLiteral( "greenContrastEnhancement" ) );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    greenContrastEnhancement->readXml( greenContrastElem );
    r->setGreenContrastEnhancement( greenContrastEnhancement );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  const QDomElement blueContrastElem = element.firstChildElement( QStringLiteral( "blueContrastEnhancement" ) );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    blueContrastEnhancement->readXml( blueContrastElem );
    r->setBlueContrastEnhancement( blueContrastEnhancement );
  }

  return r.release();
}

QDomElement QgsPointCloudRgbRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "rgb" ) );

  rendererElem.setAttribute( QStringLiteral( "red" ), mRedAttribute );
  rendererElem.setAttribute( QStringLiteral( "green" ), mGreenAttribute );
  rendererElem.setAttribute( QStringLiteral( "blue" ), mBlueAttribute );

  saveCommonProperties( rendererElem, context );

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( QStringLiteral( "redContrastEnhancement" ) );
    mRedContrastEnhancement->writeXml( doc, redContrastElem );
    rendererElem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( QStringLiteral( "greenContrastEnhancement" ) );
    mGreenContrastEnhancement->writeXml( doc, greenContrastElem );
    rendererElem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( QStringLiteral( "blueContrastEnhancement" ) );
    mBlueContrastEnhancement->writeXml( doc, blueContrastElem );
    rendererElem.appendChild( blueContrastElem );
  }

  return rendererElem;
}

QSet<QString> QgsPointCloudRgbRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mRedAttribute << mGreenAttribute << mBlueAttribute;
  return res;
}

std::unique_ptr<QgsPreparedPointCloudRendererData> QgsPointCloudRgbRenderer::prepare()
{
  std::unique_ptr< QgsPointCloudRgbRendererPreparedData > data = std::make_unique< QgsPointCloudRgbRendererPreparedData >();
  data->redAttribute = mRedAttribute;
  if ( mRedContrastEnhancement )
    data->redContrastEnhancement.reset( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  data->greenAttribute = mGreenAttribute;
  if ( mGreenContrastEnhancement )
    data->greenContrastEnhancement.reset( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  data->blueAttribute = mBlueAttribute;
  if ( mBlueContrastEnhancement )
    data->blueContrastEnhancement.reset( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );

  data->useRedContrastEnhancement = mRedContrastEnhancement && mRedContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  data->useBlueContrastEnhancement = mBlueContrastEnhancement && mBlueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  data->useGreenContrastEnhancement = mGreenContrastEnhancement && mGreenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;

  return data;
}

QSet<QString> QgsPointCloudRgbRendererPreparedData::usedAttributes() const
{
  return { redAttribute, greenAttribute, blueAttribute };
}

bool QgsPointCloudRgbRendererPreparedData::prepareBlock( const QgsPointCloudBlock *block )
{
  const QgsPointCloudAttributeCollection request = block->attributes();
  redOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( redAttribute, redOffset );
  if ( !attribute )
    return false;
  redType = attribute->type();

  greenOffset = 0;
  attribute = request.find( greenAttribute, greenOffset );
  if ( !attribute )
    return false;
  greenType = attribute->type();

  blueOffset = 0;
  attribute = request.find( blueAttribute, blueOffset );
  if ( !attribute )
    return false;
  blueType = attribute->type();
  return true;
}

QColor QgsPointCloudRgbRendererPreparedData::pointColor( const QgsPointCloudBlock *block, int i, double )
{
  const char *ptr = block->data();
  const int pointRecordSize = block->pointRecordSize();

  int red = 0;
  QgsPointCloudRenderContext::getAttribute( ptr, i * pointRecordSize + redOffset, redType, red );
  int green = 0;
  QgsPointCloudRenderContext::getAttribute( ptr, i * pointRecordSize + greenOffset, greenType, green );
  int blue = 0;
  QgsPointCloudRenderContext::getAttribute( ptr, i * pointRecordSize + blueOffset, blueType, blue );

  //skip if red, green or blue not in displayable range
  if ( ( useRedContrastEnhancement && !redContrastEnhancement->isValueInDisplayableRange( red ) )
       || ( useGreenContrastEnhancement && !greenContrastEnhancement->isValueInDisplayableRange( green ) )
       || ( useBlueContrastEnhancement && !blueContrastEnhancement->isValueInDisplayableRange( blue ) ) )
  {
    return QColor();
  }

  //stretch color values
  if ( useRedContrastEnhancement )
  {
    red = redContrastEnhancement->enhanceContrast( red );
  }
  if ( useGreenContrastEnhancement )
  {
    green = greenContrastEnhancement->enhanceContrast( green );
  }
  if ( useBlueContrastEnhancement )
  {
    blue = blueContrastEnhancement->enhanceContrast( blue );
  }

  red = std::max( 0, std::min( 255, red ) );
  green = std::max( 0, std::min( 255, green ) );
  blue = std::max( 0, std::min( 255, blue ) );

  return QColor( red, green, blue );
}

QString QgsPointCloudRgbRenderer::redAttribute() const
{
  return mRedAttribute;
}

void QgsPointCloudRgbRenderer::setRedAttribute( const QString &redAttribute )
{
  mRedAttribute = redAttribute;
}

QString QgsPointCloudRgbRenderer::greenAttribute() const
{
  return mGreenAttribute;
}

void QgsPointCloudRgbRenderer::setGreenAttribute( const QString &greenAttribute )
{
  mGreenAttribute = greenAttribute;
}

QString QgsPointCloudRgbRenderer::blueAttribute() const
{
  return mBlueAttribute;
}

void QgsPointCloudRgbRenderer::setBlueAttribute( const QString &blueAttribute )
{
  mBlueAttribute = blueAttribute;
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::redContrastEnhancement() const
{
  return mRedContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setRedContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mRedContrastEnhancement.reset( enhancement );
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::greenContrastEnhancement() const
{
  return mGreenContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setGreenContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mGreenContrastEnhancement.reset( enhancement );
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::blueContrastEnhancement() const
{
  return mBlueContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setBlueContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mBlueContrastEnhancement.reset( enhancement );
}
