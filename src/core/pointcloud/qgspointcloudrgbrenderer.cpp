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
  std::unique_ptr< QgsPointCloudRgbRenderer > res = qgis::make_unique< QgsPointCloudRgbRenderer >();
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
  const QgsMapToPixel mapToPixel = context.renderContext().mapToPixel();

  QgsRectangle mapExtent = context.renderContext().mapExtent();

  QPen pen;
  pen.setWidth( mPainterPenWidth );
  pen.setCapStyle( Qt::FlatCap );
  //pen.setJoinStyle( Qt::MiterJoin );

  const char *ptr = block->data();
  int count = block->pointCount();
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

  int rendered = 0;
  double x = 0;
  double y = 0;
  for ( int i = 0; i < count; ++i )
  {
    pointXY( context, ptr, i, x, y );

    if ( mapExtent.contains( QgsPointXY( x, y ) ) )
    {
      int red = 0;
      switch ( redType )
      {
        case QgsPointCloudAttribute::Char:
          continue;

        case QgsPointCloudAttribute::Int32:
          red = *( qint32 * )( ptr + i * recordSize + redOffset );
          break;

        case QgsPointCloudAttribute::Short:
          red = *( short * )( ptr + i * recordSize + redOffset );
          break;

        case QgsPointCloudAttribute::Float:
          red = *( float * )( ptr + i * recordSize + redOffset );
          break;

        case QgsPointCloudAttribute::Double:
          red = *( double * )( ptr + i * recordSize + redOffset );
          break;
      }

      int green = 0;
      switch ( greenType )
      {
        case QgsPointCloudAttribute::Char:
          continue;

        case QgsPointCloudAttribute::Int32:
          green = *( qint32 * )( ptr + i * recordSize + greenOffset );
          break;

        case QgsPointCloudAttribute::Short:
          green = *( short * )( ptr + i * recordSize + greenOffset );
          break;

        case QgsPointCloudAttribute::Float:
          green = *( float * )( ptr + i * recordSize + greenOffset );
          break;

        case QgsPointCloudAttribute::Double:
          green = *( double * )( ptr + i * recordSize + greenOffset );
          break;
      }

      int blue = 0;
      switch ( blueType )
      {
        case QgsPointCloudAttribute::Char:
          continue;

        case QgsPointCloudAttribute::Int32:
          blue = *( qint32 * )( ptr + i * recordSize + blueOffset );
          break;

        case QgsPointCloudAttribute::Short:
          blue = *( short * )( ptr + i * recordSize + blueOffset );
          break;

        case QgsPointCloudAttribute::Float:
          blue = *( float * )( ptr + i * recordSize + blueOffset );
          break;

        case QgsPointCloudAttribute::Double:
          blue = *( double * )( ptr + i * recordSize + blueOffset );
          break;
      }

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

      mapToPixel.transformInPlace( x, y );

      pen.setColor( QColor( red, green, blue ) );
      context.renderContext().painter()->setPen( pen );
      context.renderContext().painter()->drawPoint( QPointF( x, y ) );

      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}


QgsPointCloudRenderer *QgsPointCloudRgbRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudRgbRenderer > r = qgis::make_unique< QgsPointCloudRgbRenderer >();

  r->setRedAttribute( element.attribute( QStringLiteral( "red" ), QStringLiteral( "Red" ) ) );
  r->setGreenAttribute( element.attribute( QStringLiteral( "green" ), QStringLiteral( "Green" ) ) );
  r->setBlueAttribute( element.attribute( QStringLiteral( "blue" ), QStringLiteral( "Blue" ) ) );

  r->restoreCommonProperties( element, context );

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  QDomElement redContrastElem = element.firstChildElement( QStringLiteral( "redContrastEnhancement" ) );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
    redContrastEnhancement->readXml( redContrastElem );
    r->setRedContrastEnhancement( redContrastEnhancement );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  QDomElement greenContrastElem = element.firstChildElement( QStringLiteral( "greenContrastEnhancement" ) );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
    greenContrastEnhancement->readXml( greenContrastElem );
    r->setGreenContrastEnhancement( greenContrastEnhancement );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  QDomElement blueContrastElem = element.firstChildElement( QStringLiteral( "blueContrastEnhancement" ) );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
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

void QgsPointCloudRgbRenderer::startRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::startRender( context );

  mPainterPenWidth = context.renderContext().convertToPainterUnits( pointSize(), pointSizeUnit(), pointSizeMapUnitScale() );
}

void QgsPointCloudRgbRenderer::stopRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::stopRender( context );
}

QSet<QString> QgsPointCloudRgbRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  return QSet<QString>() << mRedAttribute << mGreenAttribute << mBlueAttribute;
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
