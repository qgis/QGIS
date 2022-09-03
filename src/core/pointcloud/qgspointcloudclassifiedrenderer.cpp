/***************************************************************************
                         qgspointcloudclassifiedrenderer.h
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

#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudblock.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgssymbollayerutils.h"
#include "qgslayertreemodellegendnode.h"
#include "qgspointclouddataprovider.h"

QgsPointCloudCategory::QgsPointCloudCategory( const int value, const QColor &color, const QString &label, bool render )
  : mValue( value )
  , mColor( color )
  , mLabel( label )
  , mRender( render )
{
}

bool QgsPointCloudCategory::operator==( const QgsPointCloudCategory &other ) const
{
  return mValue == other.value() &&
         mColor == other.color() &&
         mLabel == other.label() &&
         mRender == other.renderState();
}

//
// QgsPointCloudClassifiedRenderer
//

QgsPointCloudClassifiedRenderer::QgsPointCloudClassifiedRenderer( const QString &attributeName, const QgsPointCloudCategoryList &categories )
  : mAttribute( attributeName )
  , mCategories( categories )
{
}

QString QgsPointCloudClassifiedRenderer::type() const
{
  return QStringLiteral( "classified" );
}

QgsPointCloudRenderer *QgsPointCloudClassifiedRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudClassifiedRenderer > res = std::make_unique< QgsPointCloudClassifiedRenderer >();
  res->mAttribute = mAttribute;
  res->mCategories = mCategories;

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudClassifiedRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsRectangle visibleExtent = context.renderContext().extent();

  const char *ptr = block->data();
  int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int attributeOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mAttribute, attributeOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType attributeType = attribute->type();

  const bool renderElevation = context.elevationMap();
  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite() || renderElevation;

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();

  QHash< int, QColor > colors;
  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    if ( !category.renderState() )
      continue;

    colors.insert( category.value(), category.color() );
  }

  for ( int i = 0; i < count; ++i )
  {
    if ( context.renderContext().renderingStopped() )
    {
      break;
    }

    // z value filtering is cheapest, if we're doing it...
    if ( considerZ )
    {
      z = pointZ( context, ptr, i );
      if ( !zRange.contains( z ) )
        continue;
    }

    int attributeValue = 0;
    context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, attributeValue );
    const QColor color = colors.value( attributeValue );
    if ( !color.isValid() )
      continue;

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

      drawPoint( x, y, color, context );
      if ( renderElevation )
        drawPointToElevationMap( x, y, z, context );
      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}

bool QgsPointCloudClassifiedRenderer::willRenderPoint( const QVariantMap &pointAttributes )
{
  if ( !pointAttributes.contains( mAttribute ) )
    return false;
  bool parsedCorrectly;
  int attributeInt = pointAttributes[ mAttribute ].toInt( &parsedCorrectly );
  if ( !parsedCorrectly )
    return false;
  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    if ( category.value() == attributeInt )
      return category.renderState();
  }
  return false;
}

QgsPointCloudRenderer *QgsPointCloudClassifiedRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudClassifiedRenderer > r = std::make_unique< QgsPointCloudClassifiedRenderer >();

  r->setAttribute( element.attribute( QStringLiteral( "attribute" ), QStringLiteral( "Classification" ) ) );

  QgsPointCloudCategoryList categories;
  const QDomElement catsElem = element.firstChildElement( QStringLiteral( "categories" ) );
  if ( !catsElem.isNull() )
  {
    QDomElement catElem = catsElem.firstChildElement();
    while ( !catElem.isNull() )
    {
      if ( catElem.tagName() == QLatin1String( "category" ) )
      {
        const int value = catElem.attribute( QStringLiteral( "value" ) ).toInt();
        const QString label = catElem.attribute( QStringLiteral( "label" ) );
        const bool render = catElem.attribute( QStringLiteral( "render" ) ) != QLatin1String( "false" );
        const QColor color = QgsSymbolLayerUtils::decodeColor( catElem.attribute( QStringLiteral( "color" ) ) );
        categories.append( QgsPointCloudCategory( value, color, label, render ) );
      }
      catElem = catElem.nextSiblingElement();
    }
    r->setCategories( categories );
  }

  r->restoreCommonProperties( element, context );

  return r.release();
}

QgsPointCloudCategoryList QgsPointCloudClassifiedRenderer::defaultCategories()
{
  return QgsPointCloudCategoryList() << QgsPointCloudCategory( 0, QColor( "#BABABA" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 0 ) )
         << QgsPointCloudCategory( 1, QColor( "#AAAAAA" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 1 ) )
         << QgsPointCloudCategory( 2, QColor( "#AA5500" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 2 ) )
         << QgsPointCloudCategory( 3, QColor( "#00AAAA" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 3 ) )
         << QgsPointCloudCategory( 4, QColor( "#55FF55" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 4 ) )
         << QgsPointCloudCategory( 5, QColor( "#00AA00" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 5 ) )
         << QgsPointCloudCategory( 6, QColor( "#FF5555" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 6 ) )
         << QgsPointCloudCategory( 7, QColor( "#AA0000" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 7 ) )
         << QgsPointCloudCategory( 8, QColor( "#555555" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 8 ) )
         << QgsPointCloudCategory( 9, QColor( "#55FFFF" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 9 ) )
         << QgsPointCloudCategory( 10, QColor( "#AA00AA" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 10 ) )
         << QgsPointCloudCategory( 11, QColor( "#000000" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 11 ) )
         << QgsPointCloudCategory( 12, QColor( "#555555" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 12 ) )
         << QgsPointCloudCategory( 13, QColor( "#FFFF55" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 13 ) )
         << QgsPointCloudCategory( 14, QColor( "#FFFF55" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 14 ) )
         << QgsPointCloudCategory( 15, QColor( "#FF55FF" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 15 ) )
         << QgsPointCloudCategory( 16, QColor( "#FFFF55" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 16 ) )
         << QgsPointCloudCategory( 17, QColor( "#5555FF" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 17 ) )
         << QgsPointCloudCategory( 18, QColor( "#646464" ), QgsPointCloudDataProvider::translatedLasClassificationCodes().value( 18 ) );
}

QDomElement QgsPointCloudClassifiedRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "classified" ) );
  rendererElem.setAttribute( QStringLiteral( "attribute" ), mAttribute );

  // categories
  QDomElement catsElem = doc.createElement( QStringLiteral( "categories" ) );
  for ( const QgsPointCloudCategory &category : mCategories )
  {
    QDomElement catElem = doc.createElement( QStringLiteral( "category" ) );
    catElem.setAttribute( QStringLiteral( "value" ), QString::number( category.value() ) );
    catElem.setAttribute( QStringLiteral( "label" ), category.label() );
    catElem.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( category.color() ) );
    catElem.setAttribute( QStringLiteral( "render" ), category.renderState() ? "true" : "false" );
    catsElem.appendChild( catElem );
  }
  rendererElem.appendChild( catsElem );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

QSet<QString> QgsPointCloudClassifiedRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mAttribute;
  return res;
}

QList<QgsLayerTreeModelLegendNode *> QgsPointCloudClassifiedRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    nodes << new QgsRasterSymbolLegendNode( nodeLayer, category.color(), category.label(), nullptr, true, QString::number( category.value() ) );
  }

  return nodes;
}

QStringList QgsPointCloudClassifiedRenderer::legendRuleKeys() const
{
  QStringList res;
  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    res << QString::number( category.value() );
  }
  return res;
}

bool QgsPointCloudClassifiedRenderer::legendItemChecked( const QString &key )
{
  bool ok = false;
  const int value = key.toInt( &ok );
  if ( !ok )
    return false;

  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    if ( category.value() == value )
      return category.renderState();
  }
  return false;
}

void QgsPointCloudClassifiedRenderer::checkLegendItem( const QString &key, bool state )
{
  bool ok = false;
  const int value = key.toInt( &ok );
  if ( !ok )
    return;

  for ( auto it = mCategories.begin(); it != mCategories.end(); ++it )
  {
    if ( it->value() == value )
    {
      it->setRenderState( state );
      return;
    }
  }
}

QString QgsPointCloudClassifiedRenderer::attribute() const
{
  return mAttribute;
}

void QgsPointCloudClassifiedRenderer::setAttribute( const QString &attribute )
{
  mAttribute = attribute;
}

QgsPointCloudCategoryList QgsPointCloudClassifiedRenderer::categories() const
{
  return mCategories;
}

void QgsPointCloudClassifiedRenderer::setCategories( const QgsPointCloudCategoryList &categories )
{
  mCategories = categories;
}

void QgsPointCloudClassifiedRenderer::addCategory( const QgsPointCloudCategory &category )
{
  mCategories.append( category );
}

std::unique_ptr<QgsPreparedPointCloudRendererData> QgsPointCloudClassifiedRenderer::prepare()
{
  std::unique_ptr< QgsPointCloudClassifiedRendererPreparedData > data = std::make_unique< QgsPointCloudClassifiedRendererPreparedData >();
  data->attributeName = mAttribute;

  for ( const QgsPointCloudCategory &category : std::as_const( mCategories ) )
  {
    if ( !category.renderState() )
      continue;

    data->colors.insert( category.value(), category.color() );
  }

  return data;
}

QSet<QString> QgsPointCloudClassifiedRendererPreparedData::usedAttributes() const
{
  return { attributeName };
}

bool QgsPointCloudClassifiedRendererPreparedData::prepareBlock( const QgsPointCloudBlock *block )
{
  const QgsPointCloudAttributeCollection attributes = block->attributes();
  const QgsPointCloudAttribute *attribute = attributes.find( attributeName, attributeOffset );
  if ( !attribute )
    return false;

  attributeType = attribute->type();
  return true;
}

QColor QgsPointCloudClassifiedRendererPreparedData::pointColor( const QgsPointCloudBlock *block, int i, double )
{
  int attributeValue = 0;
  QgsPointCloudRenderContext::getAttribute( block->data(), i * block->pointRecordSize() + attributeOffset, attributeType, attributeValue );
  return colors.value( attributeValue );
}


