/***************************************************************************
  qgsvectortilebasiclabeling.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilebasiclabeling.h"

#include "qgsexpressioncontextutils.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilerenderer.h"

void QgsVectorTileBasicLabelingStyle::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"name"_s, mStyleName );
  elem.setAttribute( u"layer"_s, mLayerName );
  elem.setAttribute( u"geometry"_s, static_cast<int>( mGeometryType ) );
  elem.setAttribute( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  elem.setAttribute( u"expression"_s, mExpression );
  elem.setAttribute( u"min-zoom"_s, mMinZoomLevel );
  elem.setAttribute( u"max-zoom"_s, mMaxZoomLevel );

  QDomDocument doc = elem.ownerDocument();
  QDomElement elemLabelSettings = mLabelSettings.writeXml( doc, context );
  elem.appendChild( elemLabelSettings );
}

void QgsVectorTileBasicLabelingStyle::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyleName = elem.attribute( u"name"_s );
  mLayerName = elem.attribute( u"layer"_s );
  mGeometryType = static_cast<Qgis::GeometryType>( elem.attribute( u"geometry"_s ).toInt() );
  mEnabled = elem.attribute( u"enabled"_s ).toInt();
  mExpression = elem.attribute( u"expression"_s );
  mMinZoomLevel = elem.attribute( u"min-zoom"_s ).toInt();
  mMaxZoomLevel = elem.attribute( u"max-zoom"_s ).toInt();

  QDomElement elemLabelSettings = elem.firstChildElement( u"settings"_s );
  mLabelSettings.readXml( elemLabelSettings, context );
}


//


QgsVectorTileBasicLabeling::QgsVectorTileBasicLabeling()
{
}

QString QgsVectorTileBasicLabeling::type() const
{
  return u"basic"_s;
}

QgsVectorTileLabeling *QgsVectorTileBasicLabeling::clone() const
{
  QgsVectorTileBasicLabeling *l = new QgsVectorTileBasicLabeling;
  l->mStyles = mStyles;
  return l;
}

QgsVectorTileLabelProvider *QgsVectorTileBasicLabeling::provider( QgsVectorTileLayer *layer ) const
{
  return new QgsVectorTileBasicLabelProvider( layer, mStyles );
}

void QgsVectorTileBasicLabeling::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemStyles = doc.createElement( u"styles"_s );
  for ( const QgsVectorTileBasicLabelingStyle &layerStyle : mStyles )
  {
    QDomElement elemStyle = doc.createElement( u"style"_s );
    layerStyle.writeXml( elemStyle, context );
    elemStyles.appendChild( elemStyle );
  }
  elem.appendChild( elemStyles );
}

void QgsVectorTileBasicLabeling::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyles.clear();

  QDomElement elemStyles = elem.firstChildElement( u"styles"_s );
  QDomElement elemStyle = elemStyles.firstChildElement( u"style"_s );
  while ( !elemStyle.isNull() )
  {
    QgsVectorTileBasicLabelingStyle layerStyle;
    layerStyle.readXml( elemStyle, context );
    mStyles.append( layerStyle );
    elemStyle = elemStyle.nextSiblingElement( u"style"_s );
  }
}


//


QgsVectorTileBasicLabelProvider::QgsVectorTileBasicLabelProvider( QgsVectorTileLayer *layer, const QList<QgsVectorTileBasicLabelingStyle> &styles )
  : QgsVectorTileLabelProvider( layer )
  , mStyles( styles )
{

  for ( int i = 0; i < mStyles.count(); ++i )
  {
    const QgsVectorTileBasicLabelingStyle &style = mStyles[i];
    //QgsFields fields = QgsVectorTileUtils::makeQgisFields( mRequiredFields[style.layerName()] );
    QString providerId = QString::number( i );
    QgsPalLayerSettings labelSettings = style.labelSettings();
    mSubProviders.append( new QgsVectorLayerLabelProvider( style.geometryType(), QgsFields(), layer->crs(), providerId, &labelSettings, layer ) );
  }
}

QMap<QString, QSet<QString> > QgsVectorTileBasicLabelProvider::usedAttributes( const QgsRenderContext &context, int tileZoom ) const
{
  QMap<QString, QSet<QString> > requiredFields;
  for ( const QgsVectorTileBasicLabelingStyle &layerStyle : std::as_const( mStyles ) )
  {
    if ( !layerStyle.isActive( tileZoom ) )
      continue;

    if ( !layerStyle.filterExpression().isEmpty() )
    {
      QgsExpression expr( layerStyle.filterExpression() );
      requiredFields[layerStyle.layerName()].unite( expr.referencedColumns() );
    }

    requiredFields[layerStyle.layerName()].unite( layerStyle.labelSettings().referencedFields( context ) );
  }
  return requiredFields;
}

QSet<QString> QgsVectorTileBasicLabelProvider::requiredLayers( QgsRenderContext &, int tileZoom ) const
{
  QSet< QString > res;
  for ( const QgsVectorTileBasicLabelingStyle &layerStyle : std::as_const( mStyles ) )
  {
    if ( layerStyle.isActive( tileZoom ) )
    {
      res.insert( layerStyle.layerName() );
    }
  }
  return res;
}

void QgsVectorTileBasicLabelProvider::setFields( const QMap<QString, QgsFields> &perLayerFields )
{
  mPerLayerFields = perLayerFields;
}

QList<QgsAbstractLabelProvider *> QgsVectorTileBasicLabelProvider::subProviders()
{
  QList<QgsAbstractLabelProvider *> lst;
  for ( QgsVectorLayerLabelProvider *subprovider : std::as_const( mSubProviders ) )
  {
    if ( subprovider )  // sub-providers that failed to initialize are set to null
      lst << subprovider;
  }
  return lst;
}

bool QgsVectorTileBasicLabelProvider::prepare( QgsRenderContext &context, QSet<QString> &attributeNames )
{
  for ( QgsVectorLayerLabelProvider *provider : std::as_const( mSubProviders ) )
    provider->setEngine( mEngine );

  // populate sub-providers
  for ( int i = 0; i < mSubProviders.count(); ++i )
  {
    QgsFields fields = mPerLayerFields[mStyles[i].layerName()];

    QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) ); // will be deleted by popper
    scope->setFields( fields );
    QgsExpressionContextScopePopper popper( context.expressionContext(), scope );

    mSubProviders[i]->setFields( fields );
    // check is required as fields are not available through the GUI, which can lead to isExpression wrongly set to true
    mSubProviders[i]->mSettings.isExpression = !fields.names().contains( mSubProviders[i]->mSettings.fieldName );
    if ( !mSubProviders[i]->prepare( context, attributeNames ) )
    {
      QgsDebugError( u"Failed to prepare labeling for style index"_s + QString::number( i ) );
      mSubProviders[i] = nullptr;
    }
  }
  return true;
}

void QgsVectorTileBasicLabelProvider::registerTileFeatures( const QgsVectorTileRendererData &tile, QgsRenderContext &context )
{
  const QgsVectorTileFeatures tileData = tile.features();
  const int zoomLevel = tile.renderZoomLevel();

  for ( int i = 0; i < mStyles.count(); ++i )
  {
    const QgsVectorTileBasicLabelingStyle &layerStyle = mStyles.at( i );
    if ( !layerStyle.isActive( zoomLevel ) )
      continue;

    QgsFields fields = mPerLayerFields[layerStyle.layerName()];

    QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) ); // will be deleted by popper
    scope->setFields( fields );
    QgsExpressionContextScopePopper popper( context.expressionContext(), scope );

    QgsExpression filterExpression( layerStyle.filterExpression() );
    filterExpression.prepare( &context.expressionContext() );

    QgsVectorLayerLabelProvider *subProvider = mSubProviders[i];
    if ( !subProvider )
      continue;  // sub-providers that failed to initialize are set to null

    if ( layerStyle.layerName().isEmpty() )
    {
      // matching all layers
      for ( const auto &features : tileData )
      {
        for ( const QgsFeature &f : features )
        {
          scope->setFeature( f );
          if ( filterExpression.isValid() && !filterExpression.evaluate( &context.expressionContext() ).toBool() )
            continue;

          const Qgis::GeometryType featureType = QgsWkbTypes::geometryType( f.geometry().wkbType() );
          if ( featureType == layerStyle.geometryType() )
          {
            subProvider->registerFeature( f, context );
          }
          else if ( featureType == Qgis::GeometryType::Polygon && layerStyle.geometryType() == Qgis::GeometryType::Point )
          {
            // be tolerant and permit labeling polygons with a point layer style, as some style definitions use this approach
            // to label the polygon center
            QgsFeature centroid = f;
            const QgsRectangle boundingBox = f.geometry().boundingBox();
            centroid.setGeometry( f.geometry().poleOfInaccessibility( std::min( boundingBox.width(), boundingBox.height() ) / 20 ) );
            subProvider->registerFeature( centroid, context );
          }
        }
      }
    }
    else if ( tileData.contains( layerStyle.layerName() ) )
    {
      // matching one particular layer
      for ( const QgsFeature &f : tileData[layerStyle.layerName()] )
      {
        scope->setFeature( f );
        if ( filterExpression.isValid() && !filterExpression.evaluate( &context.expressionContext() ).toBool() )
          continue;

        const Qgis::GeometryType featureType = QgsWkbTypes::geometryType( f.geometry().wkbType() );
        if ( featureType == layerStyle.geometryType() )
        {
          subProvider->registerFeature( f, context );
        }
        else if ( featureType == Qgis::GeometryType::Polygon && layerStyle.geometryType() == Qgis::GeometryType::Point )
        {
          // be tolerant and permit labeling polygons with a point layer style, as some style definitions use this approach
          // to label the polygon center
          QgsFeature centroid = f;
          const QgsRectangle boundingBox = f.geometry().boundingBox();
          centroid.setGeometry( f.geometry().poleOfInaccessibility( std::min( boundingBox.width(), boundingBox.height() ) / 20 ) );
          subProvider->registerFeature( centroid, context );
        }
      }
    }
  }
}
