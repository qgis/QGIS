/***************************************************************************
                              qgslayoutitemmap.cpp
                             ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoututils.h"
#include "qgslayoutmodel.h"
#include "qgsmapthemecollection.h"
#include "qgsannotationmanager.h"
#include "qgsannotation.h"
#include "qgsmapsettingsutils.h"
#include "qgslayertree.h"
#include "qgsmaplayerref.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontext.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsannotationlayer.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsprojoperation.h"
#include "qgslabelingresults.h"
#include "qgsvectortileutils.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

QgsLayoutItemMap::QgsLayoutItemMap( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mAtlasClippingSettings( new QgsLayoutItemMapAtlasClippingSettings( this ) )
  , mItemClippingSettings( new QgsLayoutItemMapItemClipPathSettings( this ) )
{
  mBackgroundUpdateTimer = new QTimer( this );
  mBackgroundUpdateTimer->setSingleShot( true );
  connect( mBackgroundUpdateTimer, &QTimer::timeout, this, &QgsLayoutItemMap::recreateCachedImageInBackground );

  assignFreeId();

  setCacheMode( QGraphicsItem::NoCache );

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [ = ]
  {
    shapeChanged();
  } );

  mGridStack = std::make_unique< QgsLayoutItemMapGridStack >( this );
  mOverviewStack = std::make_unique< QgsLayoutItemMapOverviewStack >( this );

  connect( mAtlasClippingSettings, &QgsLayoutItemMapAtlasClippingSettings::changed, this, [ = ]
  {
    refresh();
  } );

  connect( mItemClippingSettings, &QgsLayoutItemMapItemClipPathSettings::changed, this, [ = ]
  {
    refresh();
  } );

  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [ = ]
  {
    QgsCoordinateReferenceSystem crs = mCrs;
    crs.updateDefinition();
    if ( mCrs != crs )
    {
      setCrs( crs );
      invalidateCache();
    }
  } );

  if ( layout )
    connectUpdateSlot();
}

QgsLayoutItemMap::~QgsLayoutItemMap()
{
  if ( mPainterJob )
  {
    disconnect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsLayoutItemMap::painterJobFinished );
    emit backgroundTaskCountChanged( 0 );
    mPainterJob->cancel(); // blocks
    mPainter->end();
  }
}

int QgsLayoutItemMap::type() const
{
  return QgsLayoutItemRegistry::LayoutMap;
}

QIcon QgsLayoutItemMap::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemMap.svg" ) );
}

QgsLayoutItem::Flags QgsLayoutItemMap::itemFlags() const
{
  return QgsLayoutItem::FlagOverridesPaint;
}

void QgsLayoutItemMap::assignFreeId()
{
  if ( !mLayout )
    return;

  QList<QgsLayoutItemMap *> mapsList;
  mLayout->layoutItems( mapsList );

  int maxId = -1;
  bool used = false;
  for ( QgsLayoutItemMap *map : std::as_const( mapsList ) )
  {
    if ( map == this )
      continue;

    if ( map->mMapId == mMapId )
      used = true;

    maxId = std::max( maxId, map->mMapId );
  }
  if ( used )
  {
    mMapId = maxId + 1;
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
  updateToolTip();
}

QString QgsLayoutItemMap::displayName() const
{
  if ( !QgsLayoutItem::id().isEmpty() )
  {
    return QgsLayoutItem::id();
  }

  return tr( "Map %1" ).arg( mMapId );
}

QgsLayoutItemMap *QgsLayoutItemMap::create( QgsLayout *layout )
{
  return new QgsLayoutItemMap( layout );
}

void QgsLayoutItemMap::refresh()
{
  QgsLayoutItem::refresh();

  mCachedLayerStyleOverridesPresetName.clear();

  invalidateCache();

  updateAtlasFeature();
}

double QgsLayoutItemMap::scale() const
{
  if ( rect().isEmpty() )
    return 0;

  QgsScaleCalculator calculator;
  calculator.setMapUnits( crs().mapUnits() );
  calculator.setDpi( 25.4 );  //Using mm
  double widthInMm = mLayout->convertFromLayoutUnits( rect().width(), QgsUnitTypes::LayoutMillimeters ).length();
  return calculator.calculate( extent(), widthInMm );
}

void QgsLayoutItemMap::setScale( double scaleDenominator, bool forceUpdate )
{
  double currentScaleDenominator = scale();

  if ( qgsDoubleNear( scaleDenominator, currentScaleDenominator ) || qgsDoubleNear( scaleDenominator, 0.0 ) )
  {
    return;
  }

  double scaleRatio = scaleDenominator / currentScaleDenominator;
  mExtent.scale( scaleRatio );

  if ( mAtlasDriven && mAtlasScalingMode == Fixed )
  {
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanent
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( crs().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    scaleRatio = scaleDenominator / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  invalidateCache();
  if ( forceUpdate )
  {
    emit changed();
    update();
  }
  emit extentChanged();
}

void QgsLayoutItemMap::setExtent( const QgsRectangle &extent )
{
  if ( mExtent == extent )
  {
    return;
  }
  mExtent = extent;

  //recalculate data defined scale and extents, since that may override extent
  refreshMapExtents();

  //adjust height
  QRectF currentRect = rect();

  double newHeight = currentRect.width() * mExtent.height() / mExtent.width();

  attemptSetSceneRect( QRectF( pos().x(), pos().y(), currentRect.width(), newHeight ) );
  update();
}

void QgsLayoutItemMap::zoomToExtent( const QgsRectangle &extent )
{
  QgsRectangle newExtent = extent;
  QgsRectangle currentExtent = mExtent;
  //Make sure the width/height ratio is the same as the current layout map extent.
  //This is to keep the map item frame size fixed
  double currentWidthHeightRatio = 1.0;
  if ( !currentExtent.isNull() )
    currentWidthHeightRatio = currentExtent.width() / currentExtent.height();
  else
    currentWidthHeightRatio = rect().width() / rect().height();
  double newWidthHeightRatio = newExtent.width() / newExtent.height();

  if ( currentWidthHeightRatio < newWidthHeightRatio )
  {
    //enlarge height of new extent, ensuring the map center stays the same
    double newHeight = newExtent.width() / currentWidthHeightRatio;
    double deltaHeight = newHeight - newExtent.height();
    newExtent.setYMinimum( newExtent.yMinimum() - deltaHeight / 2 );
    newExtent.setYMaximum( newExtent.yMaximum() + deltaHeight / 2 );
  }
  else
  {
    //enlarge width of new extent, ensuring the map center stays the same
    double newWidth = currentWidthHeightRatio * newExtent.height();
    double deltaWidth = newWidth - newExtent.width();
    newExtent.setXMinimum( newExtent.xMinimum() - deltaWidth / 2 );
    newExtent.setXMaximum( newExtent.xMaximum() + deltaWidth / 2 );
  }

  if ( mExtent == newExtent )
  {
    return;
  }
  mExtent = newExtent;

  //recalculate data defined scale and extents, since that may override extent
  refreshMapExtents();

  invalidateCache();
  emit changed();
  emit extentChanged();
}

QgsRectangle QgsLayoutItemMap::extent() const
{
  return mExtent;
}

QPolygonF QgsLayoutItemMap::calculateVisibleExtentPolygon( bool includeClipping ) const
{
  QPolygonF poly;
  mapPolygon( mExtent, poly );

  if ( includeClipping && mItemClippingSettings->isActive() )
  {
    const QgsGeometry geom = mItemClippingSettings->clippedMapExtent();
    if ( !geom.isEmpty() )
    {
      poly = poly.intersected( geom.asQPolygonF() );
    }
  }

  return poly;
}

QPolygonF QgsLayoutItemMap::visibleExtentPolygon() const
{
  return calculateVisibleExtentPolygon( true );
}

QgsCoordinateReferenceSystem QgsLayoutItemMap::crs() const
{
  if ( mCrs.isValid() )
    return mCrs;
  else if ( mLayout && mLayout->project() )
    return mLayout->project()->crs();
  return QgsCoordinateReferenceSystem();
}

void QgsLayoutItemMap::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCrs == crs )
    return;

  mCrs = crs;
  emit crsChanged();
}

QList<QgsMapLayer *> QgsLayoutItemMap::layers() const
{
  return _qgis_listRefToRaw( mLayers );
}

void QgsLayoutItemMap::setLayers( const QList<QgsMapLayer *> &layers )
{
  mLayers = _qgis_listRawToRef( layers );
}

void QgsLayoutItemMap::setLayerStyleOverrides( const QMap<QString, QString> &overrides )
{
  if ( overrides == mLayerStyleOverrides )
    return;

  mLayerStyleOverrides = overrides;
  emit layerStyleOverridesChanged();  // associated legends may listen to this

}

void QgsLayoutItemMap::storeCurrentLayerStyles()
{
  mLayerStyleOverrides.clear();
  for ( const QgsMapLayerRef &layerRef : std::as_const( mLayers ) )
  {
    if ( QgsMapLayer *layer = layerRef.get() )
    {
      QgsMapLayerStyle style;
      style.readFromLayer( layer );
      mLayerStyleOverrides.insert( layer->id(), style.xmlData() );
    }
  }
}

void QgsLayoutItemMap::setFollowVisibilityPreset( bool follow )
{
  if ( mFollowVisibilityPreset == follow )
    return;

  mFollowVisibilityPreset = follow;

  if ( !mFollowVisibilityPresetName.isEmpty() )
    emit themeChanged( mFollowVisibilityPreset ? mFollowVisibilityPresetName : QString() );
}

void QgsLayoutItemMap::setFollowVisibilityPresetName( const QString &name )
{
  if ( name == mFollowVisibilityPresetName )
    return;

  mFollowVisibilityPresetName = name;
  if ( mFollowVisibilityPreset )
    emit themeChanged( mFollowVisibilityPresetName );
}

void QgsLayoutItemMap::moveContent( double dx, double dy )
{
  mLastRenderedImageOffsetX -= dx;
  mLastRenderedImageOffsetY -= dy;
  if ( !mDrawing )
  {
    transformShift( dx, dy );
    mExtent.setXMinimum( mExtent.xMinimum() + dx );
    mExtent.setXMaximum( mExtent.xMaximum() + dx );
    mExtent.setYMinimum( mExtent.yMinimum() + dy );
    mExtent.setYMaximum( mExtent.yMaximum() + dy );

    //in case data defined extents are set, these override the calculated values
    refreshMapExtents();

    invalidateCache();
    emit changed();
    emit extentChanged();
  }
}

void QgsLayoutItemMap::zoomContent( double factor, QPointF point )
{
  if ( mDrawing )
  {
    return;
  }

  //find out map coordinates of position
  double mapX = mExtent.xMinimum() + ( point.x() / rect().width() ) * ( mExtent.xMaximum() - mExtent.xMinimum() );
  double mapY = mExtent.yMinimum() + ( 1 - ( point.y() / rect().height() ) ) * ( mExtent.yMaximum() - mExtent.yMinimum() );

  //find out new center point
  double centerX = ( mExtent.xMaximum() + mExtent.xMinimum() ) / 2;
  double centerY = ( mExtent.yMaximum() + mExtent.yMinimum() ) / 2;

  centerX = mapX + ( centerX - mapX ) * ( 1.0 / factor );
  centerY = mapY + ( centerY - mapY ) * ( 1.0 / factor );

  double newIntervalX, newIntervalY;

  if ( factor > 0 )
  {
    newIntervalX = ( mExtent.xMaximum() - mExtent.xMinimum() ) / factor;
    newIntervalY = ( mExtent.yMaximum() - mExtent.yMinimum() ) / factor;
  }
  else //no need to zoom
  {
    return;
  }

  mExtent.setXMaximum( centerX + newIntervalX / 2 );
  mExtent.setXMinimum( centerX - newIntervalX / 2 );
  mExtent.setYMaximum( centerY + newIntervalY / 2 );
  mExtent.setYMinimum( centerY - newIntervalY / 2 );

  if ( mAtlasDriven && mAtlasScalingMode == Fixed )
  {
    //if map is atlas controlled and set to fixed scaling mode, then scale changes should be treated as permanent
    //and also apply to the map's original extent (see #9602)
    //we can't use the scaleRatio calculated earlier, as the scale can vary depending on extent for geographic coordinate systems
    QgsScaleCalculator calculator;
    calculator.setMapUnits( crs().mapUnits() );
    calculator.setDpi( 25.4 );  //QGraphicsView units are mm
    double scaleRatio = scale() / calculator.calculate( mExtent, rect().width() );
    mExtent.scale( scaleRatio );
  }

  //recalculate data defined scale and extents, since that may override zoom
  refreshMapExtents();

  invalidateCache();
  emit changed();
  emit extentChanged();
}

bool QgsLayoutItemMap::containsWmsLayer() const
{
  const QList< QgsMapLayer * > layers = layersToRender();
  for ( QgsMapLayer *layer : layers )
  {
    if ( layer->dataProvider() && layer->providerType() == QLatin1String( "wms" ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsLayoutItemMap::requiresRasterization() const
{
  if ( QgsLayoutItem::requiresRasterization() )
    return true;

  // we MUST force the whole layout to render as a raster if any map item
  // uses blend modes, and we are not drawing on a solid opaque background
  // because in this case the map item needs to be rendered as a raster, but
  // it also needs to interact with items below it
  if ( !containsAdvancedEffects() )
    return false;

  if ( hasBackground() && qgsDoubleNear( backgroundColor().alphaF(), 1.0 ) )
    return false;

  return true;
}

bool QgsLayoutItemMap::containsAdvancedEffects() const
{
  if ( QgsLayoutItem::containsAdvancedEffects() )
    return true;

  //check easy things first

  //overviews
  if ( mOverviewStack->containsAdvancedEffects() )
  {
    return true;
  }

  //grids
  if ( mGridStack->containsAdvancedEffects() )
  {
    return true;
  }

  QgsMapSettings ms;
  ms.setLayers( layersToRender() );
  return ( !QgsMapSettingsUtils::containsAdvancedEffects( ms ).isEmpty() );
}

void QgsLayoutItemMap::setMapRotation( double rotation )
{
  mMapRotation = rotation;
  mEvaluatedMapRotation = mMapRotation;
  invalidateCache();
  emit mapRotationChanged( rotation );
  emit changed();
}

double QgsLayoutItemMap::mapRotation( QgsLayoutObject::PropertyValueType valueType ) const
{
  return valueType == QgsLayoutObject::EvaluatedValue ? mEvaluatedMapRotation : mMapRotation;

}

void QgsLayoutItemMap::setAtlasDriven( bool enabled )
{
  mAtlasDriven = enabled;

  if ( !enabled )
  {
    //if not enabling the atlas, we still need to refresh the map extents
    //so that data defined extents and scale are recalculated
    refreshMapExtents();
  }
}

double QgsLayoutItemMap::atlasMargin( const QgsLayoutObject::PropertyValueType valueType )
{
  if ( valueType == QgsLayoutObject::EvaluatedValue )
  {
    //evaluate data defined atlas margin

    //start with user specified margin
    double margin = mAtlasMargin;
    QgsExpressionContext context = createExpressionContext();

    bool ok = false;
    double ddMargin = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapAtlasMargin, context, 0.0, &ok );
    if ( ok )
    {
      //divide by 100 to convert to 0 -> 1.0 range
      margin = ddMargin / 100;
    }
    return margin;
  }
  else
  {
    return mAtlasMargin;
  }
}

QgsLayoutItemMapGrid *QgsLayoutItemMap::grid()
{
  if ( mGridStack->size() < 1 )
  {
    QgsLayoutItemMapGrid *grid = new QgsLayoutItemMapGrid( tr( "Grid %1" ).arg( 1 ), this );
    mGridStack->addGrid( grid );
  }
  return mGridStack->grid( 0 );
}

QgsLayoutItemMapOverview *QgsLayoutItemMap::overview()
{
  if ( mOverviewStack->size() < 1 )
  {
    QgsLayoutItemMapOverview *overview = new QgsLayoutItemMapOverview( tr( "Overview %1" ).arg( 1 ), this );
    mOverviewStack->addOverview( overview );
  }
  return mOverviewStack->overview( 0 );
}

void QgsLayoutItemMap::draw( QgsLayoutItemRenderContext & )
{
}

bool QgsLayoutItemMap::writePropertiesToElement( QDomElement &mapElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( mKeepLayerSet )
  {
    mapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "true" ) );
  }
  else
  {
    mapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "false" ) );
  }

  if ( mDrawAnnotations )
  {
    mapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  }
  else
  {
    mapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "false" ) );
  }

  //extent
  QDomElement extentElem = doc.createElement( QStringLiteral( "Extent" ) );
  extentElem.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mExtent.xMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mExtent.xMaximum() ) );
  extentElem.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mExtent.yMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mExtent.yMaximum() ) );
  mapElem.appendChild( extentElem );

  if ( mCrs.isValid() )
  {
    QDomElement crsElem = doc.createElement( QStringLiteral( "crs" ) );
    mCrs.writeXml( crsElem, doc );
    mapElem.appendChild( crsElem );
  }

  // follow map theme
  mapElem.setAttribute( QStringLiteral( "followPreset" ), mFollowVisibilityPreset ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  mapElem.setAttribute( QStringLiteral( "followPresetName" ), mFollowVisibilityPresetName );

  //map rotation
  mapElem.setAttribute( QStringLiteral( "mapRotation" ), QString::number( mMapRotation ) );

  //layer set
  QDomElement layerSetElem = doc.createElement( QStringLiteral( "LayerSet" ) );
  for ( const QgsMapLayerRef &layerRef : mLayers )
  {
    if ( !layerRef )
      continue;
    QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );
    QDomText layerIdText = doc.createTextNode( layerRef.layerId );
    layerElem.appendChild( layerIdText );

    layerElem.setAttribute( QStringLiteral( "name" ), layerRef.name );
    layerElem.setAttribute( QStringLiteral( "source" ), layerRef.source );
    layerElem.setAttribute( QStringLiteral( "provider" ), layerRef.provider );

    layerSetElem.appendChild( layerElem );
  }
  mapElem.appendChild( layerSetElem );

  // override styles
  if ( mKeepLayerStyles )
  {
    QDomElement stylesElem = doc.createElement( QStringLiteral( "LayerStyles" ) );
    for ( auto styleIt = mLayerStyleOverrides.constBegin(); styleIt != mLayerStyleOverrides.constEnd(); ++styleIt )
    {
      QDomElement styleElem = doc.createElement( QStringLiteral( "LayerStyle" ) );

      QgsMapLayerRef ref( styleIt.key() );
      ref.resolve( mLayout->project() );

      styleElem.setAttribute( QStringLiteral( "layerid" ), ref.layerId );
      styleElem.setAttribute( QStringLiteral( "name" ), ref.name );
      styleElem.setAttribute( QStringLiteral( "source" ), ref.source );
      styleElem.setAttribute( QStringLiteral( "provider" ), ref.provider );

      QgsMapLayerStyle style( styleIt.value() );
      style.writeXml( styleElem );
      stylesElem.appendChild( styleElem );
    }
    mapElem.appendChild( stylesElem );
  }

  //grids
  mGridStack->writeXml( mapElem, doc, context );

  //overviews
  mOverviewStack->writeXml( mapElem, doc, context );

  //atlas
  QDomElement atlasElem = doc.createElement( QStringLiteral( "AtlasMap" ) );
  atlasElem.setAttribute( QStringLiteral( "atlasDriven" ), mAtlasDriven );
  atlasElem.setAttribute( QStringLiteral( "scalingMode" ), mAtlasScalingMode );
  atlasElem.setAttribute( QStringLiteral( "margin" ), qgsDoubleToString( mAtlasMargin ) );
  mapElem.appendChild( atlasElem );

  mapElem.setAttribute( QStringLiteral( "labelMargin" ), mLabelMargin.encodeMeasurement() );
  mapElem.setAttribute( QStringLiteral( "mapFlags" ), static_cast< int>( mMapFlags ) );

  QDomElement labelBlockingItemsElem = doc.createElement( QStringLiteral( "labelBlockingItems" ) );
  for ( const auto &item : std::as_const( mBlockingLabelItems ) )
  {
    if ( !item )
      continue;

    QDomElement blockingItemElem = doc.createElement( QStringLiteral( "item" ) );
    blockingItemElem.setAttribute( QStringLiteral( "uuid" ), item->uuid() );
    labelBlockingItemsElem.appendChild( blockingItemElem );
  }
  mapElem.appendChild( labelBlockingItemsElem );

  //temporal settings
  mapElem.setAttribute( QStringLiteral( "isTemporal" ), isTemporal() ? 1 : 0 );
  if ( isTemporal() )
  {
    mapElem.setAttribute( QStringLiteral( "temporalRangeBegin" ), temporalRange().begin().toString( Qt::ISODate ) );
    mapElem.setAttribute( QStringLiteral( "temporalRangeEnd" ), temporalRange().end().toString( Qt::ISODate ) );
  }

  mAtlasClippingSettings->writeXml( mapElem, doc, context );
  mItemClippingSettings->writeXml( mapElem, doc, context );

  return true;
}

bool QgsLayoutItemMap::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  mUpdatesEnabled = false;

  //extent
  QDomNodeList extentNodeList = itemElem.elementsByTagName( QStringLiteral( "Extent" ) );
  if ( !extentNodeList.isEmpty() )
  {
    QDomElement extentElem = extentNodeList.at( 0 ).toElement();
    double xmin, xmax, ymin, ymax;
    xmin = extentElem.attribute( QStringLiteral( "xmin" ) ).toDouble();
    xmax = extentElem.attribute( QStringLiteral( "xmax" ) ).toDouble();
    ymin = extentElem.attribute( QStringLiteral( "ymin" ) ).toDouble();
    ymax = extentElem.attribute( QStringLiteral( "ymax" ) ).toDouble();
    setExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );
  }

  QDomNodeList crsNodeList = itemElem.elementsByTagName( QStringLiteral( "crs" ) );
  QgsCoordinateReferenceSystem crs;
  if ( !crsNodeList.isEmpty() )
  {
    QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    crs.readXml( crsElem );
  }
  setCrs( crs );

  //map rotation
  mMapRotation = itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble();
  mEvaluatedMapRotation = mMapRotation;

  // follow map theme
  mFollowVisibilityPreset = itemElem.attribute( QStringLiteral( "followPreset" ) ).compare( QLatin1String( "true" ) ) == 0;
  mFollowVisibilityPresetName = itemElem.attribute( QStringLiteral( "followPresetName" ) );

  //mKeepLayerSet flag
  QString keepLayerSetFlag = itemElem.attribute( QStringLiteral( "keepLayerSet" ) );
  if ( keepLayerSetFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mKeepLayerSet = true;
  }
  else
  {
    mKeepLayerSet = false;
  }

  QString drawCanvasItemsFlag = itemElem.attribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  if ( drawCanvasItemsFlag.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mDrawAnnotations = true;
  }
  else
  {
    mDrawAnnotations = false;
  }

  mLayerStyleOverrides.clear();

  //mLayers
  mLayers.clear();
  QDomNodeList layerSetNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerSet" ) );
  if ( !layerSetNodeList.isEmpty() )
  {
    QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( QStringLiteral( "Layer" ) );
    mLayers.reserve( layerIdNodeList.size() );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      QDomElement layerElem = layerIdNodeList.at( i ).toElement();
      QString layerId = layerElem.text();
      QString layerName = layerElem.attribute( QStringLiteral( "name" ) );
      QString layerSource = layerElem.attribute( QStringLiteral( "source" ) );
      QString layerProvider = layerElem.attribute( QStringLiteral( "provider" ) );

      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( mLayout->project() );
      mLayers << ref;
    }
  }

  // override styles
  QDomNodeList layerStylesNodeList = itemElem.elementsByTagName( QStringLiteral( "LayerStyles" ) );
  mKeepLayerStyles = !layerStylesNodeList.isEmpty();
  if ( mKeepLayerStyles )
  {
    QDomElement layerStylesElem = layerStylesNodeList.at( 0 ).toElement();
    QDomNodeList layerStyleNodeList = layerStylesElem.elementsByTagName( QStringLiteral( "LayerStyle" ) );
    for ( int i = 0; i < layerStyleNodeList.size(); ++i )
    {
      const QDomElement &layerStyleElement = layerStyleNodeList.at( i ).toElement();
      QString layerId = layerStyleElement.attribute( QStringLiteral( "layerid" ) );
      QString layerName = layerStyleElement.attribute( QStringLiteral( "name" ) );
      QString layerSource = layerStyleElement.attribute( QStringLiteral( "source" ) );
      QString layerProvider = layerStyleElement.attribute( QStringLiteral( "provider" ) );
      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      ref.resolveWeakly( mLayout->project() );

      QgsMapLayerStyle style;
      style.readXml( layerStyleElement );
      mLayerStyleOverrides.insert( ref.layerId, style.xmlData() );
    }
  }

  mDrawing = false;
  mNumCachedLayers = 0;
  mCacheInvalidated = true;

  //overviews
  mOverviewStack->readXml( itemElem, doc, context );

  //grids
  mGridStack->readXml( itemElem, doc, context );

  //atlas
  QDomNodeList atlasNodeList = itemElem.elementsByTagName( QStringLiteral( "AtlasMap" ) );
  if ( !atlasNodeList.isEmpty() )
  {
    QDomElement atlasElem = atlasNodeList.at( 0 ).toElement();
    mAtlasDriven = ( atlasElem.attribute( QStringLiteral( "atlasDriven" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
    if ( atlasElem.hasAttribute( QStringLiteral( "fixedScale" ) ) ) // deprecated XML
    {
      mAtlasScalingMode = ( atlasElem.attribute( QStringLiteral( "fixedScale" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) ) ? Fixed : Auto;
    }
    else if ( atlasElem.hasAttribute( QStringLiteral( "scalingMode" ) ) )
    {
      mAtlasScalingMode = static_cast<AtlasScalingMode>( atlasElem.attribute( QStringLiteral( "scalingMode" ) ).toInt() );
    }
    mAtlasMargin = atlasElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "0.1" ) ).toDouble();
  }

  setLabelMargin( QgsLayoutMeasurement::decodeMeasurement( itemElem.attribute( QStringLiteral( "labelMargin" ), QStringLiteral( "0" ) ) ) );

  mMapFlags = static_cast< MapItemFlags>( itemElem.attribute( QStringLiteral( "mapFlags" ), nullptr ).toInt() );

  // label blocking items
  mBlockingLabelItems.clear();
  mBlockingLabelItemUuids.clear();
  QDomNodeList labelBlockingNodeList = itemElem.elementsByTagName( QStringLiteral( "labelBlockingItems" ) );
  if ( !labelBlockingNodeList.isEmpty() )
  {
    QDomElement blockingItems = labelBlockingNodeList.at( 0 ).toElement();
    QDomNodeList labelBlockingNodeList = blockingItems.childNodes();
    for ( int i = 0; i < labelBlockingNodeList.size(); ++i )
    {
      const QDomElement &itemBlockingElement = labelBlockingNodeList.at( i ).toElement();
      const QString itemUuid = itemBlockingElement.attribute( QStringLiteral( "uuid" ) );
      mBlockingLabelItemUuids << itemUuid;
    }
  }

  mAtlasClippingSettings->readXml( itemElem, doc, context );
  mItemClippingSettings->readXml( itemElem, doc, context );

  updateBoundingRect();

  //temporal settings
  setIsTemporal( itemElem.attribute( QStringLiteral( "isTemporal" ) ).toInt() );
  if ( isTemporal() )
  {
    const QDateTime begin = QDateTime::fromString( itemElem.attribute( QStringLiteral( "temporalRangeBegin" ) ), Qt::ISODate );
    const QDateTime end = QDateTime::fromString( itemElem.attribute( QStringLiteral( "temporalRangeEnd" ) ), Qt::ISODate );
    setTemporalRange( QgsDateTimeRange( begin, end, true, begin == end ) );
  }

  mUpdatesEnabled = true;
  return true;
}

QPainterPath QgsLayoutItemMap::framePath() const
{
  if ( mItemClippingSettings->isActive() )
  {
    const QgsGeometry g = mItemClippingSettings->clipPathInMapItemCoordinates();
    if ( !g.isNull() )
      return g.constGet()->asQPainterPath();
  }
  return QgsLayoutItem::framePath();
}

void QgsLayoutItemMap::paint( QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget * )
{
  if ( !mLayout || !painter || !painter->device() || !mUpdatesEnabled )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  QRectF thisPaintRect = rect();
  if ( qgsDoubleNear( thisPaintRect.width(), 0.0 ) || qgsDoubleNear( thisPaintRect.height(), 0 ) )
    return;

  //TODO - try to reduce the amount of duplicate code here!

  if ( mLayout->renderContext().isPreviewRender() )
  {
    QgsScopedQPainterState painterState( painter );
    painter->setClipRect( thisPaintRect );
    if ( !mCacheFinalImage || mCacheFinalImage->isNull() )
    {
      // No initial render available - so draw some preview text alerting user
      painter->setBrush( QBrush( QColor( 125, 125, 125, 125 ) ) );
      painter->drawRect( thisPaintRect );
      painter->setBrush( Qt::NoBrush );
      QFont messageFont;
      messageFont.setPointSize( 12 );
      painter->setFont( messageFont );
      painter->setPen( QColor( 255, 255, 255, 255 ) );
      painter->drawText( thisPaintRect, Qt::AlignCenter | Qt::AlignHCenter, tr( "Rendering map" ) );
      if ( mPainterJob && mCacheInvalidated && !mDrawingPreview )
      {
        // current job was invalidated - start a new one
        mPreviewScaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( style, painter );
        mBackgroundUpdateTimer->start( 1 );
      }
      else if ( !mPainterJob && !mDrawingPreview )
      {
        // this is the map's very first paint - trigger a cache update
        mPreviewScaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( style, painter );
        mBackgroundUpdateTimer->start( 1 );
      }
    }
    else
    {
      if ( mCacheInvalidated && !mDrawingPreview )
      {
        // cache was invalidated - trigger a background update
        mPreviewScaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( style, painter );
        mBackgroundUpdateTimer->start( 1 );
      }

      //Background color is already included in cached image, so no need to draw

      double imagePixelWidth = mCacheFinalImage->width(); //how many pixels of the image are for the map extent?
      double scale = rect().width() / imagePixelWidth;

      QgsScopedQPainterState rotatedPainterState( painter );

      painter->translate( mLastRenderedImageOffsetX + mXOffset, mLastRenderedImageOffsetY + mYOffset );
      painter->scale( scale, scale );
      painter->drawImage( 0, 0, *mCacheFinalImage );

      //restore rotation
    }

    painter->setClipRect( thisPaintRect, Qt::NoClip );

    mOverviewStack->drawItems( painter, false );
    mGridStack->drawItems( painter );
    drawAnnotations( painter );
    drawMapFrame( painter );
  }
  else
  {
    if ( mDrawing )
      return;

    mDrawing = true;
    QPaintDevice *paintDevice = painter->device();
    if ( !paintDevice )
      return;

    QgsRectangle cExtent = extent();
    QSizeF size( cExtent.width() * mapUnitsToLayoutUnits(), cExtent.height() * mapUnitsToLayoutUnits() );

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    if ( mLayout && mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering )
      painter->setRenderHint( QPainter::LosslessImageRendering, true );
#endif

    if ( containsAdvancedEffects() && ( !mLayout || !( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagForceVectorOutput ) ) )
    {
      // rasterize
      double destinationDpi = QgsLayoutUtils::scaleFactorFromItemStyle( style, painter ) * 25.4;
      double layoutUnitsInInches = mLayout ? mLayout->convertFromLayoutUnits( 1, QgsUnitTypes::LayoutInches ).length() : 1;
      int widthInPixels = static_cast< int >( std::round( boundingRect().width() * layoutUnitsInInches * destinationDpi ) );
      int heightInPixels = static_cast< int >( std::round( boundingRect().height() * layoutUnitsInInches * destinationDpi ) );
      QImage image = QImage( widthInPixels, heightInPixels, QImage::Format_ARGB32 );

      image.fill( Qt::transparent );
      image.setDotsPerMeterX( static_cast< int >( std::round( 1000 * destinationDpi / 25.4 ) ) );
      image.setDotsPerMeterY( static_cast< int >( std::round( 1000 * destinationDpi / 25.4 ) ) );
      double dotsPerMM = destinationDpi / 25.4;
      QPainter p( &image );

      QPointF tl = -boundingRect().topLeft();
      QRect imagePaintRect( static_cast< int >( std::round( tl.x() * dotsPerMM ) ),
                            static_cast< int >( std::round( tl.y() * dotsPerMM ) ),
                            static_cast< int >( std::round( thisPaintRect.width() * dotsPerMM ) ),
                            static_cast< int >( std::round( thisPaintRect.height() * dotsPerMM ) ) );
      p.setClipRect( imagePaintRect );

      p.translate( imagePaintRect.topLeft() );

      // Fill with background color - must be drawn onto the flattened image
      // so that layers with opacity or blend modes can correctly interact with it
      if ( shouldDrawPart( Background ) )
      {
        p.scale( dotsPerMM, dotsPerMM );
        drawMapBackground( &p );
        p.scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );
      }

      drawMap( &p, cExtent, imagePaintRect.size(), image.logicalDpiX() );

      // important - all other items, overviews, grids etc must be rendered to the
      // flattened image, in case these have blend modes must need to interact
      // with the map
      p.scale( dotsPerMM, dotsPerMM );

      if ( shouldDrawPart( OverviewMapExtent ) )
      {
        mOverviewStack->drawItems( &p, false );
      }
      if ( shouldDrawPart( Grid ) )
      {
        mGridStack->drawItems( &p );
      }
      drawAnnotations( &p );

      QgsScopedQPainterState painterState( painter );
      painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
      painter->drawImage( QPointF( -tl.x()* dotsPerMM, -tl.y() * dotsPerMM ), image );
      painter->scale( dotsPerMM, dotsPerMM );
    }
    else
    {
      // Fill with background color
      if ( shouldDrawPart( Background ) )
      {
        drawMapBackground( painter );
      }

      QgsScopedQPainterState painterState( painter );
      painter->setClipRect( thisPaintRect );

      if ( shouldDrawPart( Layer ) && !qgsDoubleNear( size.width(), 0.0 ) && !qgsDoubleNear( size.height(), 0.0 ) )
      {
        QgsScopedQPainterState stagedPainterState( painter );
        painter->translate( mXOffset, mYOffset );

        double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
        size *= dotsPerMM; // output size will be in dots (pixels)
        painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

        if ( mCurrentExportPart != NotLayered )
        {
          if ( !mStagedRendererJob )
          {
            createStagedRenderJob( cExtent, size, paintDevice->logicalDpiX() );
          }

          mStagedRendererJob->renderCurrentPart( painter );
        }
        else
        {
          drawMap( painter, cExtent, size, paintDevice->logicalDpiX() );
        }
      }

      painter->setClipRect( thisPaintRect, Qt::NoClip );

      if ( shouldDrawPart( OverviewMapExtent ) )
      {
        mOverviewStack->drawItems( painter, false );
      }
      if ( shouldDrawPart( Grid ) )
      {
        mGridStack->drawItems( painter );
      }
      drawAnnotations( painter );
    }

    if ( shouldDrawPart( Frame ) )
    {
      drawMapFrame( painter );
    }

    mDrawing = false;
  }
}

int QgsLayoutItemMap::numberExportLayers() const
{
  const int layerCount = layersToRender().length();
  return ( hasBackground() ? 1 : 0 )
         + ( layerCount + ( layerCount ? 1 : 0 ) ) // +1 for label layer, if labels present
         + ( mGridStack->hasEnabledItems() ? 1 : 0 )
         + ( mOverviewStack->hasEnabledItems() ? 1 : 0 )
         + ( frameEnabled() ? 1 : 0 );
}

void QgsLayoutItemMap::startLayeredExport()
{
  mCurrentExportPart = Start;
  // only follow export themes if the map isn't set to follow a fixed theme
  mExportThemes = !mFollowVisibilityPreset ? mLayout->renderContext().exportThemes() : QStringList();
  mExportThemeIt = mExportThemes.begin();
}

void QgsLayoutItemMap::stopLayeredExport()
{
  mCurrentExportPart = NotLayered;
  mExportThemes.clear();
  mExportThemeIt = mExportThemes.begin();
}

bool QgsLayoutItemMap::nextExportPart()
{
  switch ( mCurrentExportPart )
  {
    case Start:
      if ( hasBackground() )
      {
        mCurrentExportPart = Background;
        return true;
      }
      FALLTHROUGH

    case Background:
      mCurrentExportPart = Layer;
      return true;

    case Layer:
      if ( mStagedRendererJob )
      {
        if ( mStagedRendererJob->nextPart() )
          return true;
        else
        {
          mExportLabelingResults.reset( mStagedRendererJob->takeLabelingResults() );
          mStagedRendererJob.reset(); // no more map layer parts
        }
      }

      if ( mExportThemeIt != mExportThemes.end() && ++mExportThemeIt != mExportThemes.end() )
      {
        // move to next theme and continue exporting map layers
        return true;
      }

      if ( mGridStack->hasEnabledItems() )
      {
        mCurrentExportPart = Grid;
        return true;
      }
      FALLTHROUGH

    case Grid:
      for ( int i = 0; i < mOverviewStack->size(); ++i )
      {
        QgsLayoutItemMapItem *item = mOverviewStack->item( i );
        if ( item->enabled() && item->stackingPosition() == QgsLayoutItemMapItem::StackAboveMapLabels )
        {
          mCurrentExportPart = OverviewMapExtent;
          return true;
        }
      }
      FALLTHROUGH

    case OverviewMapExtent:
      if ( frameEnabled() )
      {
        mCurrentExportPart = Frame;
        return true;
      }

      FALLTHROUGH

    case Frame:
      if ( isSelected() && !mLayout->renderContext().isPreviewRender() )
      {
        mCurrentExportPart = SelectionBoxes;
        return true;
      }
      FALLTHROUGH

    case SelectionBoxes:
      mCurrentExportPart = End;
      return false;

    case End:
      return false;

    case NotLayered:
      return false;
  }
  return false;
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutItemMap::exportLayerBehavior() const
{
  return ItemContainsSubLayers;
}

QgsLayoutItem::ExportLayerDetail QgsLayoutItemMap::exportLayerDetails() const
{
  ExportLayerDetail detail;

  switch ( mCurrentExportPart )
  {
    case Start:
      break;

    case Background:
      detail.name = tr( "%1: Background" ).arg( displayName() );
      return detail;

    case Layer:
      if ( !mExportThemes.empty() && mExportThemeIt != mExportThemes.end() )
        detail.mapTheme = *mExportThemeIt;

      if ( mStagedRendererJob )
      {
        switch ( mStagedRendererJob->currentStage() )
        {
          case QgsMapRendererStagedRenderJob::Symbology:
          {
            detail.mapLayerId = mStagedRendererJob->currentLayerId();
            detail.compositionMode = mStagedRendererJob->currentLayerCompositionMode();
            detail.opacity = mStagedRendererJob->currentLayerOpacity();
            if ( const QgsMapLayer *layer = mLayout->project()->mapLayer( detail.mapLayerId ) )
            {
              if ( !detail.mapTheme.isEmpty() )
                detail.name = QStringLiteral( "%1 (%2): %3" ).arg( displayName(), detail.mapTheme, layer->name() );
              else
                detail.name = QStringLiteral( "%1: %2" ).arg( displayName(), layer->name() );
            }
            else if ( mLayout->project()->mainAnnotationLayer()->id() == detail.mapLayerId )
            {
              // master annotation layer
              if ( !detail.mapTheme.isEmpty() )
                detail.name = QStringLiteral( "%1 (%2): %3" ).arg( displayName(), detail.mapTheme, tr( "Annotations" ) );
              else
                detail.name = QStringLiteral( "%1: %2" ).arg( displayName(), tr( "Annotations" ) );
            }
            else
            {
              // might be an item based layer
              const QList<QgsLayoutItemMapOverview *> res = mOverviewStack->asList();
              for ( QgsLayoutItemMapOverview  *item : res )
              {
                if ( !item || !item->enabled() || item->stackingPosition() == QgsLayoutItemMapItem::StackAboveMapLabels )
                  continue;

                if ( item->mapLayer() && detail.mapLayerId == item->mapLayer()->id() )
                {
                  if ( !detail.mapTheme.isEmpty() )
                    detail.name = QStringLiteral( "%1 (%2): %3" ).arg( displayName(), detail.mapTheme, item->mapLayer()->name() );
                  else
                    detail.name = QStringLiteral( "%1: %2" ).arg( displayName(), item->mapLayer()->name() );
                  break;
                }
              }
            }
            return detail;
          }

          case QgsMapRendererStagedRenderJob::Labels:
            detail.mapLayerId  = mStagedRendererJob->currentLayerId();
            if ( const QgsMapLayer *layer = mLayout->project()->mapLayer( detail.mapLayerId ) )
            {
              if ( !detail.mapTheme.isEmpty() )
                detail.name = QStringLiteral( "%1 (%2): %3 (Labels)" ).arg( displayName(), detail.mapTheme, layer->name() );
              else
                detail.name = tr( "%1: %2 (Labels)" ).arg( displayName(), layer->name() );
            }
            else
            {
              if ( !detail.mapTheme.isEmpty() )
                detail.name = tr( "%1 (%2): Labels" ).arg( displayName(), detail.mapTheme );
              else
                detail.name = tr( "%1: Labels" ).arg( displayName() );
            }
            return detail;

          case QgsMapRendererStagedRenderJob::Finished:
            break;
        }
      }
      else
      {
        // we must be on the first layer, not having had a chance to create the render job yet
        const QList< QgsMapLayer * > layers = layersToRender();
        if ( !layers.isEmpty() )
        {
          const QgsMapLayer *layer = layers.constLast();
          if ( !detail.mapTheme.isEmpty() )
            detail.name = QStringLiteral( "%1 (%2): %3" ).arg( displayName(), detail.mapTheme, layer->name() );
          else
            detail.name = QStringLiteral( "%1: %2" ).arg( displayName(), layer->name() );
          detail.mapLayerId = layer->id();
        }
      }
      break;

    case Grid:
      detail.name = tr( "%1: Grids" ).arg( displayName() );
      return detail;

    case OverviewMapExtent:
      detail.name =  tr( "%1: Overviews" ).arg( displayName() );
      return detail;

    case Frame:
      detail.name = tr( "%1: Frame" ).arg( displayName() );
      return detail;

    case SelectionBoxes:
    case End:
    case NotLayered:
      break;
  }

  return detail;
}

void QgsLayoutItemMap::setFrameStrokeWidth( const QgsLayoutMeasurement width )
{
  QgsLayoutItem::setFrameStrokeWidth( width );
  updateBoundingRect();
}

void QgsLayoutItemMap::drawMap( QPainter *painter, const QgsRectangle &extent, QSizeF size, double dpi )
{
  if ( !painter )
  {
    return;
  }
  if ( qgsDoubleNear( size.width(), 0.0 ) || qgsDoubleNear( size.height(), 0.0 ) )
  {
    //don't attempt to draw if size is invalid
    return;
  }

  // render
  QgsMapSettings ms( mapSettings( extent, size, dpi, true ) );
  if ( shouldDrawPart( OverviewMapExtent ) )
  {
    ms.setLayers( mOverviewStack->modifyMapLayerList( ms.layers() ) );
  }

  QgsMapRendererCustomPainterJob job( ms, painter );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  job.setFeatureFilterProvider( mLayout->renderContext().featureFilterProvider() );
#endif

  // Render the map in this thread. This is done because of problems
  // with printing to printer on Windows (printing to PDF is fine though).
  // Raster images were not displayed - see #10599
  job.renderSynchronously();

  mExportLabelingResults.reset( job.takeLabelingResults() );

  mRenderingErrors = job.errors();
}

void QgsLayoutItemMap::recreateCachedImageInBackground()
{
  if ( mPainterJob )
  {
    disconnect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsLayoutItemMap::painterJobFinished );
    QgsMapRendererCustomPainterJob *oldJob = mPainterJob.release();
    QPainter *oldPainter = mPainter.release();
    QImage *oldImage = mCacheRenderingImage.release();
    connect( oldJob, &QgsMapRendererCustomPainterJob::finished, this, [oldPainter, oldJob, oldImage]
    {
      oldJob->deleteLater();
      delete oldPainter;
      delete oldImage;
    } );
    oldJob->cancelWithoutBlocking();
  }
  else
  {
    mCacheRenderingImage.reset( nullptr );
    emit backgroundTaskCountChanged( 1 );
  }

  Q_ASSERT( !mPainterJob );
  Q_ASSERT( !mPainter );
  Q_ASSERT( !mCacheRenderingImage );

  QgsRectangle ext = extent();
  double widthLayoutUnits = ext.width() * mapUnitsToLayoutUnits();
  double heightLayoutUnits = ext.height() * mapUnitsToLayoutUnits();

  int w = static_cast< int >( std::round( widthLayoutUnits * mPreviewScaleFactor ) );
  int h = static_cast< int >( std::round( heightLayoutUnits * mPreviewScaleFactor ) );

  // limit size of image for better performance
  if ( w > 5000 || h > 5000 )
  {
    if ( w > h )
    {
      w = 5000;
      h = static_cast< int>( std::round( w * heightLayoutUnits / widthLayoutUnits ) );
    }
    else
    {
      h = 5000;
      w = static_cast< int >( std::round( h * widthLayoutUnits / heightLayoutUnits ) );
    }
  }

  if ( w <= 0 || h <= 0 )
    return;

  mCacheRenderingImage.reset( new QImage( w, h, QImage::Format_ARGB32 ) );

  // set DPI of the image
  mCacheRenderingImage->setDotsPerMeterX( static_cast< int >( std::round( 1000 * w / widthLayoutUnits ) ) );
  mCacheRenderingImage->setDotsPerMeterY( static_cast< int >( std::round( 1000 * h / heightLayoutUnits ) ) );

  //start with empty fill to avoid artifacts
  mCacheRenderingImage->fill( QColor( 255, 255, 255, 0 ).rgba() );
  if ( hasBackground() )
  {
    //Initially fill image with specified background color. This ensures that layers with blend modes will
    //preview correctly
    if ( mItemClippingSettings->isActive() )
    {
      QPainter p( mCacheRenderingImage.get() );
      const QPainterPath path = framePath();
      p.setPen( Qt::NoPen );
      p.setBrush( backgroundColor() );
      p.scale( mCacheRenderingImage->width() / widthLayoutUnits, mCacheRenderingImage->height() / heightLayoutUnits );
      p.drawPath( path );
      p.end();
    }
    else
    {
      mCacheRenderingImage->fill( backgroundColor().rgba() );
    }
  }

  mCacheInvalidated = false;
  mPainter.reset( new QPainter( mCacheRenderingImage.get() ) );
  QgsMapSettings settings( mapSettings( ext, QSizeF( w, h ), mCacheRenderingImage->logicalDpiX(), true ) );

  if ( shouldDrawPart( OverviewMapExtent ) )
  {
    settings.setLayers( mOverviewStack->modifyMapLayerList( settings.layers() ) );
  }

  mPainterJob.reset( new QgsMapRendererCustomPainterJob( settings, mPainter.get() ) );
  connect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsLayoutItemMap::painterJobFinished );
  mPainterJob->start();

  // from now on we can accept refresh requests again
  // this must be reset only after the job has been started, because
  // some providers (yes, it's you WCS and AMS!) during preparation
  // do network requests and start an internal event loop, which may
  // end up calling refresh() and would schedule another refresh,
  // deleting the one we have just started.

  // ^^ that comment was directly copied from a similar fix in QgsMapCanvas. And
  // with little surprise, both those providers are still badly behaved and causing
  // annoying bugs for us to deal with...
  mDrawingPreview = false;
}

QgsLayoutItemMap::MapItemFlags QgsLayoutItemMap::mapFlags() const
{
  return mMapFlags;
}

void QgsLayoutItemMap::setMapFlags( QgsLayoutItemMap::MapItemFlags mapFlags )
{
  mMapFlags = mapFlags;
}

QgsMapSettings QgsLayoutItemMap::mapSettings( const QgsRectangle &extent, QSizeF size, double dpi, bool includeLayerSettings ) const
{
  QgsExpressionContext expressionContext = createExpressionContext();
  QgsCoordinateReferenceSystem renderCrs = crs();

  QgsMapSettings jobMapSettings;
  jobMapSettings.setDestinationCrs( renderCrs );
  jobMapSettings.setExtent( extent );
  jobMapSettings.setOutputSize( size.toSize() );
  jobMapSettings.setOutputDpi( dpi );
  if ( layout()->renderContext().isPreviewRender() )
    jobMapSettings.setDpiTarget( layout()->renderContext().dpi() );
  jobMapSettings.setBackgroundColor( Qt::transparent );
  jobMapSettings.setRotation( mEvaluatedMapRotation );
  if ( mLayout )
    jobMapSettings.setEllipsoid( mLayout->project()->ellipsoid() );

  if ( includeLayerSettings )
  {
    //set layers to render
    QList<QgsMapLayer *> layers = layersToRender( &expressionContext );

    if ( !mLayout->project()->mainAnnotationLayer()->isEmpty() )
    {
      // render main annotation layer above all other layers
      layers.insert( 0, mLayout->project()->mainAnnotationLayer() );
    }

    jobMapSettings.setLayers( layers );
    jobMapSettings.setLayerStyleOverrides( layerStyleOverridesToRender( expressionContext ) );
  }

  if ( !mLayout->renderContext().isPreviewRender() )
  {
    //if outputting layout, we disable optimisations like layer simplification by default, UNLESS the context specifically tells us to use them
    jobMapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, mLayout->renderContext().simplifyMethod().simplifyHints() != QgsVectorSimplifyMethod::NoSimplification );
    jobMapSettings.setSimplifyMethod( mLayout->renderContext().simplifyMethod() );
    jobMapSettings.setRendererUsage( Qgis::RendererUsage::Export );
  }
  else
  {
    // preview render - always use optimization
    jobMapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, true );
    jobMapSettings.setRendererUsage( Qgis::RendererUsage::View );
  }

  jobMapSettings.setExpressionContext( expressionContext );

  // layout-specific overrides of flags
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true ); // force vector output (no caching of marker images etc.)
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagAntialiasing );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagAntialiasing );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::LosslessImageRendering, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::DrawEditingInfo, false );
  jobMapSettings.setSelectionColor( mLayout->renderContext().selectionColor() );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSelection, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagDrawSelection );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::RenderPartialOutput, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagDisableTiledRasterLayerRenders );
  jobMapSettings.setFlag( Qgis::MapSettingsFlag::UseAdvancedEffects, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagUseAdvancedEffects );
  jobMapSettings.setTransformContext( mLayout->project()->transformContext() );
  jobMapSettings.setPathResolver( mLayout->project()->pathResolver() );

  QgsLabelingEngineSettings labelSettings = mLayout->project()->labelingEngineSettings();

  // override project "show partial labels" setting with this map's setting
  labelSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, mMapFlags & ShowPartialLabels );
  labelSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, mMapFlags & ShowUnplacedLabels );
  labelSettings.setFlag( QgsLabelingEngineSettings::CollectUnplacedLabels, true );
  jobMapSettings.setLabelingEngineSettings( labelSettings );

  // override the default text render format inherited from the labeling engine settings using the layout's render context setting
  jobMapSettings.setTextRenderFormat( mLayout->renderContext().textRenderFormat() );

  QgsGeometry labelBoundary;
  if ( mEvaluatedLabelMargin.length() > 0 )
  {
    QPolygonF visiblePoly = jobMapSettings.visiblePolygon();
    visiblePoly.append( visiblePoly.at( 0 ) ); //close polygon
    const double layoutLabelMargin = mLayout->convertToLayoutUnits( mEvaluatedLabelMargin );
    const double layoutLabelMarginInMapUnits = layoutLabelMargin / rect().width() * jobMapSettings.extent().width();
    QgsGeometry mapBoundaryGeom = QgsGeometry::fromQPolygonF( visiblePoly );
    mapBoundaryGeom = mapBoundaryGeom.buffer( -layoutLabelMarginInMapUnits, 0 );
    labelBoundary = mapBoundaryGeom;
  }

  if ( !mBlockingLabelItems.isEmpty() )
  {
    jobMapSettings.setLabelBlockingRegions( createLabelBlockingRegions( jobMapSettings ) );
  }

  for ( QgsRenderedFeatureHandlerInterface *handler : std::as_const( mRenderedFeatureHandlers ) )
  {
    jobMapSettings.addRenderedFeatureHandler( handler );
  }

  if ( isTemporal() )
    jobMapSettings.setTemporalRange( temporalRange() );

  if ( mAtlasClippingSettings->enabled() && mLayout->reportContext().feature().isValid() )
  {
    QgsGeometry clipGeom( mLayout->reportContext().currentGeometry( jobMapSettings.destinationCrs() ) );
    QgsMapClippingRegion region( clipGeom );
    region.setFeatureClip( mAtlasClippingSettings->featureClippingType() );
    region.setRestrictedLayers( mAtlasClippingSettings->layersToClip() );
    region.setRestrictToLayers( mAtlasClippingSettings->restrictToLayers() );
    jobMapSettings.addClippingRegion( region );

    if ( mAtlasClippingSettings->forceLabelsInsideFeature() )
    {
      if ( !labelBoundary.isEmpty() )
      {
        labelBoundary = clipGeom.intersection( labelBoundary );
      }
      else
      {
        labelBoundary = clipGeom;
      }
    }
  }

  if ( mItemClippingSettings->isActive() )
  {
    const QgsGeometry clipGeom = mItemClippingSettings->clippedMapExtent();
    if ( !clipGeom.isEmpty() )
    {
      jobMapSettings.addClippingRegion( mItemClippingSettings->toMapClippingRegion() );

      if ( mItemClippingSettings->forceLabelsInsideClipPath() )
      {
        const double layoutLabelMargin = mLayout->convertToLayoutUnits( mEvaluatedLabelMargin );
        const double layoutLabelMarginInMapUnits = layoutLabelMargin / rect().width() * jobMapSettings.extent().width();
        QgsGeometry mapBoundaryGeom = clipGeom;
        mapBoundaryGeom = mapBoundaryGeom.buffer( -layoutLabelMarginInMapUnits, 0 );
        if ( !labelBoundary.isEmpty() )
        {
          labelBoundary = mapBoundaryGeom.intersection( labelBoundary );
        }
        else
        {
          labelBoundary = mapBoundaryGeom;
        }
      }
    }
  }

  if ( !labelBoundary.isNull() )
    jobMapSettings.setLabelBoundaryGeometry( labelBoundary );

  return jobMapSettings;
}

void QgsLayoutItemMap::finalizeRestoreFromXml()
{
  assignFreeId();

  mBlockingLabelItems.clear();
  for ( const QString &uuid : std::as_const( mBlockingLabelItemUuids ) )
  {
    QgsLayoutItem *item = mLayout->itemByUuid( uuid, true );
    if ( item )
    {
      addLabelBlockingItem( item );
    }
  }

  mOverviewStack->finalizeRestoreFromXml();
  mGridStack->finalizeRestoreFromXml();
  mItemClippingSettings->finalizeRestoreFromXml();
}

void QgsLayoutItemMap::setMoveContentPreviewOffset( double xOffset, double yOffset )
{
  mXOffset = xOffset;
  mYOffset = yOffset;
}

QRectF QgsLayoutItemMap::boundingRect() const
{
  return mCurrentRectangle;
}

QgsExpressionContext QgsLayoutItemMap::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutItem::createExpressionContext();

  //Can't utilize QgsExpressionContextUtils::mapSettingsScope as we don't always
  //have a QgsMapSettings object available when the context is required, so we manually
  //add the same variables here
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( tr( "Map Settings" ) );

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_id" ), id(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_rotation" ), mMapRotation, true ) );
  const double mapScale = scale();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_scale" ), mapScale, true ) );

  scope->setVariable( QStringLiteral( "zoom_level" ), QgsVectorTileUtils::scaleToZoomLevel( mapScale, 0, 99999 ), true );
  scope->setVariable( QStringLiteral( "vector_tile_zoom" ), QgsVectorTileUtils::scaleToZoom( mapScale ), true );

  QgsRectangle currentExtent( extent() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent" ), QVariant::fromValue( QgsGeometry::fromRect( currentExtent ) ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_width" ), currentExtent.width(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_height" ), currentExtent.height(), true ) );
  QgsGeometry centerPoint = QgsGeometry::fromPointXY( currentExtent.center() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_center" ), QVariant::fromValue( centerPoint ), true ) );

  QgsCoordinateReferenceSystem mapCrs = crs();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs" ), mapCrs.authid(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_definition" ), mapCrs.toProj(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_description" ), mapCrs.description(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_units" ), QgsUnitTypes::toString( mapCrs.mapUnits() ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_acronym" ), mapCrs.projectionAcronym(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_projection" ), mapCrs.operation().description(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_ellipsoid" ), mapCrs.ellipsoidAcronym(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_proj4" ), mapCrs.toProj(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_wkt" ), mapCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ), true ) );

  QVariantList layersIds;
  QVariantList layers;
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layer_ids" ), layersIds, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layers" ), layers, true ) );

  context.appendScope( scope );

  // The scope map_layer_ids and map_layers variables have been added to the context, only now we can
  // call layersToRender (just in case layersToRender relies on evaluating an expression which uses
  // other variables contained within the map settings scope
  const QList<QgsMapLayer *> layersInMap = layersToRender( &context );

  layersIds.reserve( layersInMap.count() );
  layers.reserve( layersInMap.count() );
  for ( QgsMapLayer *layer : layersInMap )
  {
    layersIds << layer->id();
    layers << QVariant::fromValue<QgsWeakMapLayerPointer>( QgsWeakMapLayerPointer( layer ) );
  }
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layer_ids" ), layersIds, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layers" ), layers, true ) );

  scope->addFunction( QStringLiteral( "is_layer_visible" ), new QgsExpressionContextUtils::GetLayerVisibility( layersInMap, scale() ) );

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_start_time" ), isTemporal() ? temporalRange().begin() : QVariant(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_end_time" ), isTemporal() ? temporalRange().end() : QVariant(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_interval" ), isTemporal() ? ( temporalRange().end() - temporalRange().begin() ) : QVariant(), true ) );

  return context;
}

double QgsLayoutItemMap::mapUnitsToLayoutUnits() const
{
  double extentWidth = extent().width();
  if ( extentWidth <= 0 )
  {
    return 1;
  }
  return rect().width() / extentWidth;
}

QPolygonF QgsLayoutItemMap::transformedMapPolygon() const
{
  double dx = mXOffset;
  double dy = mYOffset;
  transformShift( dx, dy );
  QPolygonF poly = calculateVisibleExtentPolygon( false );
  poly.translate( -dx, -dy );
  return poly;
}

void QgsLayoutItemMap::addLabelBlockingItem( QgsLayoutItem *item )
{
  if ( !mBlockingLabelItems.contains( item ) )
    mBlockingLabelItems.append( item );

  connect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItemMap::invalidateCache, Qt::UniqueConnection );
}

void QgsLayoutItemMap::removeLabelBlockingItem( QgsLayoutItem *item )
{
  mBlockingLabelItems.removeAll( item );
  if ( item )
    disconnect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItemMap::invalidateCache );
}

bool QgsLayoutItemMap::isLabelBlockingItem( QgsLayoutItem *item ) const
{
  return mBlockingLabelItems.contains( item );
}

QgsLabelingResults *QgsLayoutItemMap::previewLabelingResults() const
{
  return mPreviewLabelingResults.get();
}

bool QgsLayoutItemMap::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the item", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::LayoutItem, uuid(), displayName() ) ) )
    return true;

  if ( mOverviewStack )
  {
    for ( int i = 0; i < mOverviewStack->size(); ++i )
    {
      if ( mOverviewStack->item( i )->accept( visitor ) )
        return false;
    }
  }

  if ( mGridStack )
  {
    for ( int i = 0; i < mGridStack->size(); ++i )
    {
      if ( mGridStack->item( i )->accept( visitor ) )
        return false;
    }
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::LayoutItem, uuid(), displayName() ) ) )
    return false;

  return true;
}

void QgsLayoutItemMap::addRenderedFeatureHandler( QgsRenderedFeatureHandlerInterface *handler )
{
  mRenderedFeatureHandlers.append( handler );
}

void QgsLayoutItemMap::removeRenderedFeatureHandler( QgsRenderedFeatureHandlerInterface *handler )
{
  mRenderedFeatureHandlers.removeAll( handler );
}

QPointF QgsLayoutItemMap::mapToItemCoords( QPointF mapCoords ) const
{
  QPolygonF mapPoly = transformedMapPolygon();
  if ( mapPoly.empty() )
  {
    return QPointF( 0, 0 );
  }

  QgsRectangle tExtent = transformedExtent();
  QgsPointXY rotationPoint( ( tExtent.xMaximum() + tExtent.xMinimum() ) / 2.0, ( tExtent.yMaximum() + tExtent.yMinimum() ) / 2.0 );
  double dx = mapCoords.x() - rotationPoint.x();
  double dy = mapCoords.y() - rotationPoint.y();
  QgsLayoutUtils::rotate( -mEvaluatedMapRotation, dx, dy );
  QgsPointXY backRotatedCoords( rotationPoint.x() + dx, rotationPoint.y() + dy );

  QgsRectangle unrotatedExtent = transformedExtent();
  double xItem = rect().width() * ( backRotatedCoords.x() - unrotatedExtent.xMinimum() ) / unrotatedExtent.width();
  double yItem = rect().height() * ( 1 - ( backRotatedCoords.y() - unrotatedExtent.yMinimum() ) / unrotatedExtent.height() );
  return QPointF( xItem, yItem );
}

QgsRectangle QgsLayoutItemMap::requestedExtent() const
{
  QgsRectangle extent;
  QgsRectangle newExtent = mExtent;
  if ( qgsDoubleNear( mEvaluatedMapRotation, 0.0 ) )
  {
    extent = newExtent;
  }
  else
  {
    QPolygonF poly;
    mapPolygon( newExtent, poly );
    QRectF bRect = poly.boundingRect();
    extent.setXMinimum( bRect.left() );
    extent.setXMaximum( bRect.right() );
    extent.setYMinimum( bRect.top() );
    extent.setYMaximum( bRect.bottom() );
  }
  return extent;
}

void QgsLayoutItemMap::invalidateCache()
{
  if ( mDrawing )
    return;

  mCacheInvalidated = true;
  update();
}

void QgsLayoutItemMap::updateBoundingRect()
{
  QRectF rectangle = rect();
  double frameExtension = frameEnabled() ? pen().widthF() / 2.0 : 0.0;

  double topExtension = 0.0;
  double rightExtension = 0.0;
  double bottomExtension = 0.0;
  double leftExtension = 0.0;

  if ( mGridStack )
    mGridStack->calculateMaxGridExtension( topExtension, rightExtension, bottomExtension, leftExtension );

  topExtension = std::max( topExtension, frameExtension );
  rightExtension = std::max( rightExtension, frameExtension );
  bottomExtension = std::max( bottomExtension, frameExtension );
  leftExtension = std::max( leftExtension, frameExtension );

  rectangle.setLeft( rectangle.left() - leftExtension );
  rectangle.setRight( rectangle.right() + rightExtension );
  rectangle.setTop( rectangle.top() - topExtension );
  rectangle.setBottom( rectangle.bottom() + bottomExtension );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsLayoutItemMap::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  QgsExpressionContext context = createExpressionContext();
  if ( property == QgsLayoutObject::MapCrs || property == QgsLayoutObject::AllProperties )
  {
    bool ok;
    const QString crsVar = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapCrs, context, QString(), &ok );
    if ( ok && QgsCoordinateReferenceSystem( crsVar ).isValid() )
    {
      const QgsCoordinateReferenceSystem newCrs( crsVar );
      if ( newCrs.isValid() )
      {
        setCrs( newCrs );
      }
    }
  }
  //updates data defined properties and redraws item to match
  if ( property == QgsLayoutObject::MapRotation || property == QgsLayoutObject::MapScale ||
       property == QgsLayoutObject::MapXMin || property == QgsLayoutObject::MapYMin ||
       property == QgsLayoutObject::MapXMax || property == QgsLayoutObject::MapYMax ||
       property == QgsLayoutObject::MapAtlasMargin ||
       property == QgsLayoutObject::AllProperties )
  {
    QgsRectangle beforeExtent = mExtent;
    refreshMapExtents( &context );
    emit changed();
    if ( mExtent != beforeExtent )
    {
      emit extentChanged();
    }
  }
  if ( property == QgsLayoutObject::MapLabelMargin || property == QgsLayoutObject::AllProperties )
  {
    refreshLabelMargin( false );
  }
  if ( property == QgsLayoutObject::MapStylePreset || property == QgsLayoutObject::AllProperties )
  {
    const QString previousTheme = mLastEvaluatedThemeName.isEmpty() ? mFollowVisibilityPresetName : mLastEvaluatedThemeName;
    mLastEvaluatedThemeName = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapStylePreset, context, mFollowVisibilityPresetName );
    if ( mLastEvaluatedThemeName != previousTheme )
      emit themeChanged( mLastEvaluatedThemeName );
  }

  if ( isTemporal() && ( property == QgsLayoutObject::StartDateTime || property == QgsLayoutObject::EndDateTime || property == QgsLayoutObject::AllProperties ) )
  {
    QDateTime begin = temporalRange().begin();
    QDateTime end = temporalRange().end();

    if ( property == QgsLayoutObject::StartDateTime || property == QgsLayoutObject::AllProperties )
      begin = mDataDefinedProperties.valueAsDateTime( QgsLayoutObject::StartDateTime, context, temporalRange().begin() );
    if ( property == QgsLayoutObject::EndDateTime || property == QgsLayoutObject::AllProperties )
      end = mDataDefinedProperties.valueAsDateTime( QgsLayoutObject::EndDateTime, context, temporalRange().end() );

    setTemporalRange( QgsDateTimeRange( begin, end, true, begin == end ) );
  }

  //force redraw
  mCacheInvalidated = true;

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

void QgsLayoutItemMap::layersAboutToBeRemoved( const QList<QgsMapLayer *> &layers )
{
  if ( !mLayers.isEmpty() || mLayerStyleOverrides.isEmpty() )
  {
    for ( QgsMapLayer *layer : layers )
    {
      mLayerStyleOverrides.remove( layer->id() );
    }
    _qgis_removeLayers( mLayers, layers );
  }
}

void QgsLayoutItemMap::painterJobFinished()
{
  mPainter->end();
  mPreviewLabelingResults.reset( mPainterJob->takeLabelingResults() );
  mPainterJob.reset( nullptr );
  mPainter.reset( nullptr );
  mCacheFinalImage = std::move( mCacheRenderingImage );
  mLastRenderedImageOffsetX = 0;
  mLastRenderedImageOffsetY = 0;
  emit backgroundTaskCountChanged( 0 );
  update();
  emit previewRefreshed();
}

void QgsLayoutItemMap::shapeChanged()
{
  // keep center as center
  QgsPointXY oldCenter = mExtent.center();

  double w = rect().width();
  double h = rect().height();

  // keep same width as before
  double newWidth = mExtent.width();
  // but scale height to match item's aspect ratio
  double newHeight = newWidth * h / w;

  mExtent = QgsRectangle::fromCenterAndSize( oldCenter, newWidth, newHeight );

  //recalculate data defined scale and extents
  refreshMapExtents();
  updateBoundingRect();
  invalidateCache();
  emit changed();
  emit extentChanged();
}

void QgsLayoutItemMap::mapThemeChanged( const QString &theme )
{
  if ( theme == mCachedLayerStyleOverridesPresetName )
    mCachedLayerStyleOverridesPresetName.clear(); // force cache regeneration at next redraw
}

void QgsLayoutItemMap::currentMapThemeRenamed( const QString &theme, const QString &newTheme )
{
  if ( theme == mFollowVisibilityPresetName )
  {
    mFollowVisibilityPresetName = newTheme;
  }
}

void QgsLayoutItemMap::connectUpdateSlot()
{
  //connect signal from layer registry to update in case of new or deleted layers
  QgsProject *project = mLayout->project();
  if ( project )
  {
    // handles updating the stored layer state BEFORE the layers are removed
    connect( project, static_cast < void ( QgsProject::* )( const QList<QgsMapLayer *>& layers ) > ( &QgsProject::layersWillBeRemoved ),
             this, &QgsLayoutItemMap::layersAboutToBeRemoved );
    // redraws the map AFTER layers are removed
    connect( project->layerTreeRoot(), &QgsLayerTree::layerOrderChanged, this, [ = ]
    {
      if ( layers().isEmpty() )
      {
        //using project layers, and layer order has changed
        invalidateCache();
      }
    } );

    connect( project, &QgsProject::crsChanged, this, [ = ]
    {
      if ( !mCrs.isValid() )
      {
        //using project CRS, which just changed....
        invalidateCache();
        emit crsChanged();
      }
    } );

    // If project colors change, we need to redraw the map, as layer symbols may rely on project colors
    connect( project, &QgsProject::projectColorsChanged, this, [ = ]
    {
      invalidateCache();
    } );

    connect( project->mapThemeCollection(), &QgsMapThemeCollection::mapThemeChanged, this, &QgsLayoutItemMap::mapThemeChanged );
    connect( project->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &QgsLayoutItemMap::currentMapThemeRenamed );
  }
  connect( mLayout, &QgsLayout::refreshed, this, &QgsLayoutItemMap::invalidateCache );
  connect( &mLayout->renderContext(), &QgsLayoutRenderContext::predefinedScalesChanged, this, [ = ]
  {
    if ( mAtlasScalingMode == Predefined )
      updateAtlasFeature();
  } );
}

QTransform QgsLayoutItemMap::layoutToMapCoordsTransform() const
{
  QPolygonF thisExtent = calculateVisibleExtentPolygon( false );
  QTransform mapTransform;
  QPolygonF thisRectPoly = QPolygonF( QRectF( 0, 0, rect().width(), rect().height() ) );
  //workaround QT Bug #21329
  thisRectPoly.pop_back();
  thisExtent.pop_back();

  QPolygonF thisItemPolyInLayout = mapToScene( thisRectPoly );

  //create transform from layout coordinates to map coordinates
  QTransform::quadToQuad( thisItemPolyInLayout, thisExtent, mapTransform );
  return mapTransform;
}

QList<QgsLabelBlockingRegion> QgsLayoutItemMap::createLabelBlockingRegions( const QgsMapSettings & ) const
{
  const QTransform mapTransform = layoutToMapCoordsTransform();
  QList< QgsLabelBlockingRegion > blockers;
  blockers.reserve( mBlockingLabelItems.count() );
  for ( const auto &item : std::as_const( mBlockingLabelItems ) )
  {
    // invisible items don't block labels!
    if ( !item )
      continue;

    // layout items may be temporarily hidden during layered exports
    if ( item->property( "wasVisible" ).isValid() )
    {
      if ( !item->property( "wasVisible" ).toBool() )
        continue;
    }
    else if ( !item->isVisible() )
      continue;

    QPolygonF itemRectInMapCoordinates = mapTransform.map( item->mapToScene( item->rect() ) );
    itemRectInMapCoordinates.append( itemRectInMapCoordinates.at( 0 ) ); //close polygon
    QgsGeometry blockingRegion = QgsGeometry::fromQPolygonF( itemRectInMapCoordinates );
    blockers << QgsLabelBlockingRegion( blockingRegion );
  }
  return blockers;
}

QgsLayoutMeasurement QgsLayoutItemMap::labelMargin() const
{
  return mLabelMargin;
}

void QgsLayoutItemMap::setLabelMargin( const QgsLayoutMeasurement &margin )
{
  mLabelMargin = margin;
  refreshLabelMargin( false );
}

void QgsLayoutItemMap::updateToolTip()
{
  setToolTip( displayName() );
}

QString QgsLayoutItemMap::themeToRender( const QgsExpressionContext &context ) const
{
  QString presetName;

  if ( mFollowVisibilityPreset )
  {
    presetName = mFollowVisibilityPresetName;
    // preset name can be overridden by data-defined one
    presetName = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapStylePreset, context, presetName );
  }
  else if ( !mExportThemes.empty() && mExportThemeIt != mExportThemes.end() )
    presetName = *mExportThemeIt;
  return presetName;
}

QList<QgsMapLayer *> QgsLayoutItemMap::layersToRender( const QgsExpressionContext *context ) const
{
  QgsExpressionContext scopedContext;
  if ( !context )
    scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  QList<QgsMapLayer *> renderLayers;

  QString presetName = themeToRender( *evalContext );
  if ( !presetName.isEmpty() )
  {
    if ( mLayout->project()->mapThemeCollection()->hasMapTheme( presetName ) )
      renderLayers = mLayout->project()->mapThemeCollection()->mapThemeVisibleLayers( presetName );
    else  // fallback to using map canvas layers
      renderLayers = mLayout->project()->mapThemeCollection()->masterVisibleLayers();
  }
  else if ( !layers().isEmpty() )
  {
    renderLayers = layers();
  }
  else
  {
    renderLayers = mLayout->project()->mapThemeCollection()->masterVisibleLayers();
  }

  bool ok = false;
  QString ddLayers = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapLayers, *evalContext, QString(), &ok );
  if ( ok )
  {
    renderLayers.clear();

    const QStringList layerNames = ddLayers.split( '|' );
    //need to convert layer names to layer ids
    for ( const QString &name : layerNames )
    {
      const QList< QgsMapLayer * > matchingLayers = mLayout->project()->mapLayersByName( name );
      for ( QgsMapLayer *layer : matchingLayers )
      {
        renderLayers << layer;
      }
    }
  }

  //remove atlas coverage layer if required
  if ( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagHideCoverageLayer )
  {
    //hiding coverage layer
    int removeAt = renderLayers.indexOf( mLayout->reportContext().layer() );
    if ( removeAt != -1 )
    {
      renderLayers.removeAt( removeAt );
    }
  }

  // remove any invalid layers
  renderLayers.erase( std::remove_if( renderLayers.begin(), renderLayers.end(), []( QgsMapLayer * layer )
  {
    return !layer || !layer->isValid();
  } ), renderLayers.end() );

  return renderLayers;
}

QMap<QString, QString> QgsLayoutItemMap::layerStyleOverridesToRender( const QgsExpressionContext &context ) const
{
  QString presetName = themeToRender( context );
  if ( !presetName.isEmpty() )
  {
    if ( mLayout->project()->mapThemeCollection()->hasMapTheme( presetName ) )
    {
      if ( presetName != mCachedLayerStyleOverridesPresetName )
      {
        // have to regenerate cache of style overrides
        mCachedPresetLayerStyleOverrides = mLayout->project()->mapThemeCollection()->mapThemeStyleOverrides( presetName );
        mCachedLayerStyleOverridesPresetName = presetName;
      }

      return mCachedPresetLayerStyleOverrides;
    }
    else
      return QMap<QString, QString>();
  }
  else if ( mFollowVisibilityPreset )
  {
    QString presetName = mFollowVisibilityPresetName;
    // data defined preset name?
    presetName = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapStylePreset, context, presetName );
    if ( mLayout->project()->mapThemeCollection()->hasMapTheme( presetName ) )
    {
      if ( presetName.isEmpty() || presetName != mCachedLayerStyleOverridesPresetName )
      {
        // have to regenerate cache of style overrides
        mCachedPresetLayerStyleOverrides = mLayout->project()->mapThemeCollection()->mapThemeStyleOverrides( presetName );
        mCachedLayerStyleOverridesPresetName = presetName;
      }

      return mCachedPresetLayerStyleOverrides;
    }
    else
      return QMap<QString, QString>();
  }
  else if ( mKeepLayerStyles )
  {
    return mLayerStyleOverrides;
  }
  else
  {
    return QMap<QString, QString>();
  }
}

QgsRectangle QgsLayoutItemMap::transformedExtent() const
{
  double dx = mXOffset;
  double dy = mYOffset;
  transformShift( dx, dy );
  return QgsRectangle( mExtent.xMinimum() - dx, mExtent.yMinimum() - dy, mExtent.xMaximum() - dx, mExtent.yMaximum() - dy );
}

void QgsLayoutItemMap::mapPolygon( const QgsRectangle &extent, QPolygonF &poly ) const
{
  poly.clear();
  if ( qgsDoubleNear( mEvaluatedMapRotation, 0.0 ) )
  {
    poly << QPointF( extent.xMinimum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMaximum() );
    poly << QPointF( extent.xMaximum(), extent.yMinimum() );
    poly << QPointF( extent.xMinimum(), extent.yMinimum() );
    //ensure polygon is closed by readding first point
    poly << QPointF( poly.at( 0 ) );
    return;
  }

  //there is rotation
  QgsPointXY rotationPoint( ( extent.xMaximum() + extent.xMinimum() ) / 2.0, ( extent.yMaximum() + extent.yMinimum() ) / 2.0 );
  double dx, dy; //x-, y- shift from rotation point to corner point

  //top left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMaximum();
  QgsLayoutUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //top right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMaximum();
  QgsLayoutUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //bottom right point
  dx = rotationPoint.x() - extent.xMaximum();
  dy = rotationPoint.y() - extent.yMinimum();
  QgsLayoutUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //bottom left point
  dx = rotationPoint.x() - extent.xMinimum();
  dy = rotationPoint.y() - extent.yMinimum();
  QgsLayoutUtils::rotate( mEvaluatedMapRotation, dx, dy );
  poly << QPointF( rotationPoint.x() - dx, rotationPoint.y() - dy );

  //ensure polygon is closed by readding first point
  poly << QPointF( poly.at( 0 ) );
}

void QgsLayoutItemMap::transformShift( double &xShift, double &yShift ) const
{
  double mmToMapUnits = 1.0 / mapUnitsToLayoutUnits();
  double dxScaled = xShift * mmToMapUnits;
  double dyScaled = - yShift * mmToMapUnits;

  QgsLayoutUtils::rotate( mEvaluatedMapRotation, dxScaled, dyScaled );

  xShift = dxScaled;
  yShift = dyScaled;
}

void QgsLayoutItemMap::drawAnnotations( QPainter *painter )
{
  if ( !mLayout || !mLayout->project() || !mDrawAnnotations )
  {
    return;
  }

  const QList< QgsAnnotation * > annotations = mLayout->project()->annotationManager()->annotations();
  if ( annotations.isEmpty() )
    return;

  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForMap( this, painter );
  rc.setForceVectorOutput( true );
  rc.setExpressionContext( createExpressionContext() );
  QList< QgsMapLayer * > layers = layersToRender( &rc.expressionContext() );

  for ( QgsAnnotation *annotation : annotations )
  {
    if ( !annotation || !annotation->isVisible() )
    {
      continue;
    }
    if ( annotation->mapLayer() && !layers.contains( annotation->mapLayer() ) )
      continue;

    drawAnnotation( annotation, rc );
  }
}

void QgsLayoutItemMap::drawAnnotation( const QgsAnnotation *annotation, QgsRenderContext &context )
{
  if ( !annotation || !annotation->isVisible() || !context.painter() || !context.painter()->device() )
  {
    return;
  }

  QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();

  double itemX, itemY;
  if ( annotation->hasFixedMapPosition() )
  {
    QPointF mapPos = layoutMapPosForItem( annotation );
    itemX = mapPos.x();
    itemY = mapPos.y();
  }
  else
  {
    itemX = annotation->relativePosition().x() * rect().width();
    itemY = annotation->relativePosition().y() * rect().height();
  }
  context.painter()->translate( itemX, itemY );

  //setup painter scaling to dots so that symbology is drawn to scale
  double dotsPerMM = context.painter()->device()->logicalDpiX() / 25.4;
  context.painter()->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

  annotation->render( context );
}

QPointF QgsLayoutItemMap::layoutMapPosForItem( const QgsAnnotation *annotation ) const
{
  if ( !annotation )
    return QPointF( 0, 0 );

  double mapX = 0.0;
  double mapY = 0.0;

  mapX = annotation->mapPosition().x();
  mapY = annotation->mapPosition().y();
  QgsCoordinateReferenceSystem annotationCrs = annotation->mapPositionCrs();

  if ( annotationCrs != crs() )
  {
    //need to reproject
    QgsCoordinateTransform t( annotationCrs, crs(), mLayout->project() );
    double z = 0.0;
    try
    {
      t.transformInPlace( mapX, mapY, z );
    }
    catch ( const QgsCsException & )
    {
    }
  }

  return mapToItemCoords( QPointF( mapX, mapY ) );
}

void QgsLayoutItemMap::drawMapFrame( QPainter *p )
{
  if ( frameEnabled() && p )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForMap( this, p );
    rc.setExpressionContext( createExpressionContext() );

    QgsLayoutItem::drawFrame( rc );
  }
}

void QgsLayoutItemMap::drawMapBackground( QPainter *p )
{
  if ( hasBackground() && p )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForMap( this, p );
    rc.setExpressionContext( createExpressionContext() );

    QgsLayoutItem::drawBackground( rc );
  }
}

bool QgsLayoutItemMap::shouldDrawPart( QgsLayoutItemMap::PartType part ) const
{
  if ( mCurrentExportPart == NotLayered )
  {
    //all parts of the map are visible
    return true;
  }

  switch ( part )
  {
    case NotLayered:
      return true;

    case Start:
      return false;

    case Background:
      return mCurrentExportPart == Background && hasBackground();

    case Layer:
      return mCurrentExportPart == Layer;

    case Grid:
      return mCurrentExportPart == Grid && mGridStack->hasEnabledItems();

    case OverviewMapExtent:
      return mCurrentExportPart == OverviewMapExtent && mOverviewStack->hasEnabledItems();

    case Frame:
      return mCurrentExportPart == Frame && frameEnabled();

    case SelectionBoxes:
      return mCurrentExportPart == SelectionBoxes && isSelected();

    case End:
      return false;
  }

  return false;
}

void QgsLayoutItemMap::refreshMapExtents( const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext;
  if ( !context )
    scopedContext = createExpressionContext();

  bool ok = false;
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;


  //data defined map extents set?
  QgsRectangle newExtent = extent();
  bool useDdXMin = false;
  bool useDdXMax = false;
  bool useDdYMin = false;
  bool useDdYMax = false;
  double minXD = 0;
  double minYD = 0;
  double maxXD = 0;
  double maxYD = 0;

  minXD = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapXMin, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdXMin = true;
    newExtent.setXMinimum( minXD );
  }
  minYD = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapYMin, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdYMin = true;
    newExtent.setYMinimum( minYD );
  }
  maxXD = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapXMax, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdXMax = true;
    newExtent.setXMaximum( maxXD );
  }
  maxYD = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapYMax, *evalContext, 0.0, &ok );
  if ( ok )
  {
    useDdYMax = true;
    newExtent.setYMaximum( maxYD );
  }

  if ( newExtent != mExtent )
  {
    //calculate new extents to fit data defined extents

    //Make sure the width/height ratio is the same as in current map extent.
    //This is to keep the map item frame and the page layout fixed
    double currentWidthHeightRatio = mExtent.width() / mExtent.height();
    double newWidthHeightRatio = newExtent.width() / newExtent.height();

    if ( currentWidthHeightRatio < newWidthHeightRatio )
    {
      //enlarge height of new extent, ensuring the map center stays the same
      double newHeight = newExtent.width() / currentWidthHeightRatio;
      double deltaHeight = newHeight - newExtent.height();
      newExtent.setYMinimum( newExtent.yMinimum() - deltaHeight / 2 );
      newExtent.setYMaximum( newExtent.yMaximum() + deltaHeight / 2 );
    }
    else
    {
      //enlarge width of new extent, ensuring the map center stays the same
      double newWidth = currentWidthHeightRatio * newExtent.height();
      double deltaWidth = newWidth - newExtent.width();
      newExtent.setXMinimum( newExtent.xMinimum() - deltaWidth / 2 );
      newExtent.setXMaximum( newExtent.xMaximum() + deltaWidth / 2 );
    }

    mExtent = newExtent;
  }

  //now refresh scale, as this potentially overrides extents

  //data defined map scale set?
  double scaleD = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapScale, *evalContext, 0.0, &ok );
  if ( ok )
  {
    setScale( scaleD, false );
    newExtent = mExtent;
  }

  if ( useDdXMax || useDdXMin || useDdYMax || useDdYMin )
  {
    //if only one of min/max was set for either x or y, then make sure our extent is locked on that value
    //as we can do this without altering the scale
    if ( useDdXMin && !useDdXMax )
    {
      double xMax = mExtent.xMaximum() - ( mExtent.xMinimum() - minXD );
      newExtent.setXMinimum( minXD );
      newExtent.setXMaximum( xMax );
    }
    else if ( !useDdXMin && useDdXMax )
    {
      double xMin = mExtent.xMinimum() - ( mExtent.xMaximum() - maxXD );
      newExtent.setXMinimum( xMin );
      newExtent.setXMaximum( maxXD );
    }
    if ( useDdYMin && !useDdYMax )
    {
      double yMax = mExtent.yMaximum() - ( mExtent.yMinimum() - minYD );
      newExtent.setYMinimum( minYD );
      newExtent.setYMaximum( yMax );
    }
    else if ( !useDdYMin && useDdYMax )
    {
      double yMin = mExtent.yMinimum() - ( mExtent.yMaximum() - maxYD );
      newExtent.setYMinimum( yMin );
      newExtent.setYMaximum( maxYD );
    }

    if ( newExtent != mExtent )
    {
      mExtent = newExtent;
    }
  }

  //lastly, map rotation overrides all
  double mapRotation = mMapRotation;

  //data defined map rotation set?
  mapRotation = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapRotation, *evalContext, mapRotation );

  if ( !qgsDoubleNear( mEvaluatedMapRotation, mapRotation ) )
  {
    mEvaluatedMapRotation = mapRotation;
    emit mapRotationChanged( mapRotation );
  }
}

void QgsLayoutItemMap::refreshLabelMargin( bool updateItem )
{
  //data defined label margin set?
  double labelMargin = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MapLabelMargin, createExpressionContext(), mLabelMargin.length() );
  mEvaluatedLabelMargin.setLength( labelMargin );
  mEvaluatedLabelMargin.setUnits( mLabelMargin.units() );

  if ( updateItem )
  {
    update();
  }
}

void QgsLayoutItemMap::updateAtlasFeature()
{
  if ( !atlasDriven() || !mLayout->reportContext().layer() )
    return; // nothing to do

  QgsRectangle bounds = computeAtlasRectangle();
  if ( bounds.isNull() )
    return;

  double xa1 = bounds.xMinimum();
  double xa2 = bounds.xMaximum();
  double ya1 = bounds.yMinimum();
  double ya2 = bounds.yMaximum();
  QgsRectangle newExtent = bounds;
  QgsRectangle originalExtent = mExtent;

  //sanity check - only allow fixed scale mode for point layers
  bool isPointLayer = QgsWkbTypes::geometryType( mLayout->reportContext().layer()->wkbType() ) == QgsWkbTypes::PointGeometry;

  if ( mAtlasScalingMode == Fixed || mAtlasScalingMode == Predefined || isPointLayer )
  {
    QgsScaleCalculator calc;
    calc.setMapUnits( crs().mapUnits() );
    calc.setDpi( 25.4 );
    double originalScale = calc.calculate( originalExtent, rect().width() );
    double geomCenterX = ( xa1 + xa2 ) / 2.0;
    double geomCenterY = ( ya1 + ya2 ) / 2.0;
    QVector<qreal> scales;
    Q_NOWARN_DEPRECATED_PUSH
    if ( !mLayout->reportContext().predefinedScales().empty() ) // remove when deprecated method is removed
      scales = mLayout->reportContext().predefinedScales();
    else
      scales = mLayout->renderContext().predefinedScales();
    Q_NOWARN_DEPRECATED_POP
    if ( mAtlasScalingMode == Fixed || scales.isEmpty() || ( isPointLayer && mAtlasScalingMode != Predefined ) )
    {
      // only translate, keep the original scale (i.e. width x height)
      double xMin = geomCenterX - originalExtent.width() / 2.0;
      double yMin = geomCenterY - originalExtent.height() / 2.0;
      newExtent = QgsRectangle( xMin,
                                yMin,
                                xMin + originalExtent.width(),
                                yMin + originalExtent.height() );

      //scale newExtent to match original scale of map
      //this is required for geographic coordinate systems, where the scale varies by extent
      double newScale = calc.calculate( newExtent, rect().width() );
      newExtent.scale( originalScale / newScale );
    }
    else if ( mAtlasScalingMode == Predefined )
    {
      // choose one of the predefined scales
      double newWidth = originalExtent.width();
      double newHeight = originalExtent.height();
      for ( int i = 0; i < scales.size(); i++ )
      {
        double ratio = scales[i] / originalScale;
        newWidth = originalExtent.width() * ratio;
        newHeight = originalExtent.height() * ratio;

        // compute new extent, centered on feature
        double xMin = geomCenterX - newWidth / 2.0;
        double yMin = geomCenterY - newHeight / 2.0;
        newExtent = QgsRectangle( xMin,
                                  yMin,
                                  xMin + newWidth,
                                  yMin + newHeight );

        //scale newExtent to match desired map scale
        //this is required for geographic coordinate systems, where the scale varies by extent
        double newScale = calc.calculate( newExtent, rect().width() );
        newExtent.scale( scales[i] / newScale );

        if ( ( newExtent.width() >= bounds.width() ) && ( newExtent.height() >= bounds.height() ) )
        {
          // this is the smallest extent that embeds the feature, stop here
          break;
        }
      }
    }
  }
  else if ( mAtlasScalingMode == Auto )
  {
    // auto scale

    double geomRatio = bounds.width() / bounds.height();
    double mapRatio = originalExtent.width() / originalExtent.height();

    // geometry height is too big
    if ( geomRatio < mapRatio )
    {
      // extent the bbox's width
      double adjWidth = ( mapRatio * bounds.height() - bounds.width() ) / 2.0;
      xa1 -= adjWidth;
      xa2 += adjWidth;
    }
    // geometry width is too big
    else if ( geomRatio > mapRatio )
    {
      // extent the bbox's height
      double adjHeight = ( bounds.width() / mapRatio - bounds.height() ) / 2.0;
      ya1 -= adjHeight;
      ya2 += adjHeight;
    }
    newExtent = QgsRectangle( xa1, ya1, xa2, ya2 );

    const double evaluatedAtlasMargin = atlasMargin();
    if ( evaluatedAtlasMargin > 0.0 )
    {
      newExtent.scale( 1 + evaluatedAtlasMargin );
    }
  }

  // set the new extent (and render)
  setExtent( newExtent );
  emit preparedForAtlas();
}

QgsRectangle QgsLayoutItemMap::computeAtlasRectangle()
{
  // QgsGeometry::boundingBox is expressed in the geometry"s native CRS
  // We have to transform the geometry to the destination CRS and ask for the bounding box
  // Note: we cannot directly take the transformation of the bounding box, since transformations are not linear
  QgsGeometry g = mLayout->reportContext().currentGeometry( crs() );
  // Rotating the geometry, so the bounding box is correct wrt map rotation
  if ( mEvaluatedMapRotation != 0.0 )
  {
    QgsPointXY prevCenter = g.boundingBox().center();
    g.rotate( mEvaluatedMapRotation, g.boundingBox().center() );
    // Rotation center will be still the bounding box center of an unrotated geometry.
    // Which means, if the center of bbox moves after rotation, the viewport will
    // also be offset, and part of the geometry will fall out of bounds.
    // Here we compensate for that roughly: by extending the rotated bounds
    // so that its center is the same as the original.
    QgsRectangle bounds = g.boundingBox();
    double dx = std::max( std::abs( prevCenter.x() - bounds.xMinimum() ),
                          std::abs( prevCenter.x() - bounds.xMaximum() ) );
    double dy = std::max( std::abs( prevCenter.y() - bounds.yMinimum() ),
                          std::abs( prevCenter.y() - bounds.yMaximum() ) );
    QgsPointXY center = g.boundingBox().center();
    return QgsRectangle( center.x() - dx, center.y() - dy,
                         center.x() + dx, center.y() + dy );
  }
  else
  {
    return g.boundingBox();
  }
}

void QgsLayoutItemMap::createStagedRenderJob( const QgsRectangle &extent, const QSizeF size, double dpi )
{
  QgsMapSettings settings = mapSettings( extent, size, dpi, true );
  settings.setLayers( mOverviewStack->modifyMapLayerList( settings.layers() ) );

  mStagedRendererJob = std::make_unique< QgsMapRendererStagedRenderJob >( settings,
                       mLayout && mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagRenderLabelsByMapLayer
                       ? QgsMapRendererStagedRenderJob::RenderLabelsByMapLayer
                       : QgsMapRendererStagedRenderJob::Flags() );
  mStagedRendererJob->start();
}



//
// QgsLayoutItemMapAtlasClippingSettings
//

QgsLayoutItemMapAtlasClippingSettings::QgsLayoutItemMapAtlasClippingSettings( QgsLayoutItemMap *map )
  : QObject( map )
  , mMap( map )
{
  if ( mMap->layout() && mMap->layout()->project() )
  {
    connect( mMap->layout()->project(), static_cast < void ( QgsProject::* )( const QList<QgsMapLayer *>& layers ) > ( &QgsProject::layersWillBeRemoved ),
             this, &QgsLayoutItemMapAtlasClippingSettings::layersAboutToBeRemoved );
  }
}

bool QgsLayoutItemMapAtlasClippingSettings::enabled() const
{
  return mClipToAtlasFeature;
}

void QgsLayoutItemMapAtlasClippingSettings::setEnabled( bool enabled )
{
  if ( enabled == mClipToAtlasFeature )
    return;

  mClipToAtlasFeature = enabled;
  emit changed();
}

QgsMapClippingRegion::FeatureClippingType QgsLayoutItemMapAtlasClippingSettings::featureClippingType() const
{
  return mFeatureClippingType;
}

void QgsLayoutItemMapAtlasClippingSettings::setFeatureClippingType( QgsMapClippingRegion::FeatureClippingType type )
{
  if ( mFeatureClippingType == type )
    return;

  mFeatureClippingType = type;
  emit changed();
}

bool QgsLayoutItemMapAtlasClippingSettings::forceLabelsInsideFeature() const
{
  return mForceLabelsInsideFeature;
}

void QgsLayoutItemMapAtlasClippingSettings::setForceLabelsInsideFeature( bool forceInside )
{
  if ( forceInside == mForceLabelsInsideFeature )
    return;

  mForceLabelsInsideFeature = forceInside;
  emit changed();
}

bool QgsLayoutItemMapAtlasClippingSettings::restrictToLayers() const
{
  return mRestrictToLayers;
}

void QgsLayoutItemMapAtlasClippingSettings::setRestrictToLayers( bool enabled )
{
  if ( mRestrictToLayers == enabled )
    return;

  mRestrictToLayers = enabled;
  emit changed();
}

QList<QgsMapLayer *> QgsLayoutItemMapAtlasClippingSettings::layersToClip() const
{
  return _qgis_listRefToRaw( mLayersToClip );
}

void QgsLayoutItemMapAtlasClippingSettings::setLayersToClip( const QList< QgsMapLayer * > &layersToClip )
{
  mLayersToClip = _qgis_listRawToRef( layersToClip );
  emit changed();
}

bool QgsLayoutItemMapAtlasClippingSettings::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement settingsElem = document.createElement( QStringLiteral( "atlasClippingSettings" ) );
  settingsElem.setAttribute( QStringLiteral( "enabled" ), mClipToAtlasFeature ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  settingsElem.setAttribute( QStringLiteral( "forceLabelsInside" ), mForceLabelsInsideFeature ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  settingsElem.setAttribute( QStringLiteral( "clippingType" ), QString::number( static_cast<int>( mFeatureClippingType ) ) );
  settingsElem.setAttribute( QStringLiteral( "restrictLayers" ), mRestrictToLayers ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  //layer set
  QDomElement layerSetElem = document.createElement( QStringLiteral( "layersToClip" ) );
  for ( const QgsMapLayerRef &layerRef : mLayersToClip )
  {
    if ( !layerRef )
      continue;
    QDomElement layerElem = document.createElement( QStringLiteral( "Layer" ) );
    QDomText layerIdText = document.createTextNode( layerRef.layerId );
    layerElem.appendChild( layerIdText );

    layerElem.setAttribute( QStringLiteral( "name" ), layerRef.name );
    layerElem.setAttribute( QStringLiteral( "source" ), layerRef.source );
    layerElem.setAttribute( QStringLiteral( "provider" ), layerRef.provider );

    layerSetElem.appendChild( layerElem );
  }
  settingsElem.appendChild( layerSetElem );

  element.appendChild( settingsElem );
  return true;
}

bool QgsLayoutItemMapAtlasClippingSettings::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  const QDomElement settingsElem = element.firstChildElement( QStringLiteral( "atlasClippingSettings" ) );

  mClipToAtlasFeature = settingsElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mForceLabelsInsideFeature = settingsElem.attribute( QStringLiteral( "forceLabelsInside" ), QStringLiteral( "0" ) ).toInt();
  mFeatureClippingType = static_cast< QgsMapClippingRegion::FeatureClippingType >( settingsElem.attribute( QStringLiteral( "clippingType" ), QStringLiteral( "0" ) ).toInt() );
  mRestrictToLayers = settingsElem.attribute( QStringLiteral( "restrictLayers" ), QStringLiteral( "0" ) ).toInt();

  mLayersToClip.clear();
  QDomNodeList layerSetNodeList = settingsElem.elementsByTagName( QStringLiteral( "layersToClip" ) );
  if ( !layerSetNodeList.isEmpty() )
  {
    QDomElement layerSetElem = layerSetNodeList.at( 0 ).toElement();
    QDomNodeList layerIdNodeList = layerSetElem.elementsByTagName( QStringLiteral( "Layer" ) );
    mLayersToClip.reserve( layerIdNodeList.size() );
    for ( int i = 0; i < layerIdNodeList.size(); ++i )
    {
      QDomElement layerElem = layerIdNodeList.at( i ).toElement();
      QString layerId = layerElem.text();
      QString layerName = layerElem.attribute( QStringLiteral( "name" ) );
      QString layerSource = layerElem.attribute( QStringLiteral( "source" ) );
      QString layerProvider = layerElem.attribute( QStringLiteral( "provider" ) );

      QgsMapLayerRef ref( layerId, layerName, layerSource, layerProvider );
      if ( mMap->layout() && mMap->layout()->project() )
        ref.resolveWeakly( mMap->layout()->project() );
      mLayersToClip << ref;
    }
  }

  return true;
}

void QgsLayoutItemMapAtlasClippingSettings::layersAboutToBeRemoved( const QList<QgsMapLayer *> &layers )
{
  if ( !mLayersToClip.isEmpty() )
  {
    _qgis_removeLayers( mLayersToClip, layers );
  }
}

//
// QgsLayoutItemMapItemClipPathSettings
//
QgsLayoutItemMapItemClipPathSettings::QgsLayoutItemMapItemClipPathSettings( QgsLayoutItemMap *map )
  : QObject( map )
  , mMap( map )
{
}

bool QgsLayoutItemMapItemClipPathSettings::isActive() const
{
  return mEnabled && mClipPathSource;
}

bool QgsLayoutItemMapItemClipPathSettings::enabled() const
{
  return mEnabled;
}

void QgsLayoutItemMapItemClipPathSettings::setEnabled( bool enabled )
{
  if ( enabled == mEnabled )
    return;

  mEnabled = enabled;

  if ( mClipPathSource )
  {
    // may need to refresh the clip source in order to get it to render/not render depending on enabled state
    mClipPathSource->refresh();
  }
  emit changed();
}

QgsGeometry QgsLayoutItemMapItemClipPathSettings::clippedMapExtent() const
{
  if ( isActive() )
  {
    QgsGeometry clipGeom( mClipPathSource->clipPath() );
    clipGeom.transform( mMap->layoutToMapCoordsTransform() );
    return clipGeom;
  }
  return QgsGeometry();
}

QgsGeometry QgsLayoutItemMapItemClipPathSettings::clipPathInMapItemCoordinates() const
{
  if ( isActive() )
  {
    QgsGeometry clipGeom( mClipPathSource->clipPath() );
    clipGeom.transform( mMap->sceneTransform().inverted() );
    return clipGeom;
  }
  return QgsGeometry();
}

QgsMapClippingRegion QgsLayoutItemMapItemClipPathSettings::toMapClippingRegion() const
{
  QgsMapClippingRegion region( clippedMapExtent() );
  region.setFeatureClip( mFeatureClippingType );
  return region;
}

void QgsLayoutItemMapItemClipPathSettings::setSourceItem( QgsLayoutItem *item )
{
  if ( mClipPathSource == item )
    return;

  if ( mClipPathSource )
  {
    disconnect( mClipPathSource, &QgsLayoutItem::clipPathChanged, mMap, &QgsLayoutItemMap::refresh );
    disconnect( mClipPathSource, &QgsLayoutItem::rotationChanged, mMap, &QgsLayoutItemMap::refresh );
    disconnect( mClipPathSource, &QgsLayoutItem::clipPathChanged, mMap, &QgsLayoutItemMap::extentChanged );
    disconnect( mClipPathSource, &QgsLayoutItem::rotationChanged, mMap, &QgsLayoutItemMap::extentChanged );
  }

  QgsLayoutItem *oldItem = mClipPathSource;
  mClipPathSource = item;

  if ( mClipPathSource )
  {
    // if item size or rotation changes, we need to redraw this map
    connect( mClipPathSource, &QgsLayoutItem::clipPathChanged, mMap, &QgsLayoutItemMap::refresh );
    connect( mClipPathSource, &QgsLayoutItem::rotationChanged, mMap, &QgsLayoutItemMap::refresh );
    // and if clip item size or rotation changes, then effectively we've changed the visible extent of the map
    connect( mClipPathSource, &QgsLayoutItem::clipPathChanged, mMap, &QgsLayoutItemMap::extentChanged );
    connect( mClipPathSource, &QgsLayoutItem::rotationChanged, mMap, &QgsLayoutItemMap::extentChanged );
    // trigger a redraw of the clip source, so that it becomes invisible
    mClipPathSource->refresh();
  }

  if ( oldItem )
  {
    // may need to refresh the previous item in order to get it to render
    oldItem->refresh();
  }

  emit changed();
}

QgsLayoutItem *QgsLayoutItemMapItemClipPathSettings::sourceItem()
{
  return mClipPathSource;
}

QgsMapClippingRegion::FeatureClippingType QgsLayoutItemMapItemClipPathSettings::featureClippingType() const
{
  return mFeatureClippingType;
}

void QgsLayoutItemMapItemClipPathSettings::setFeatureClippingType( QgsMapClippingRegion::FeatureClippingType type )
{
  if ( mFeatureClippingType == type )
    return;

  mFeatureClippingType = type;
  emit changed();
}

bool QgsLayoutItemMapItemClipPathSettings::forceLabelsInsideClipPath() const
{
  return mForceLabelsInsideClipPath;
}

void QgsLayoutItemMapItemClipPathSettings::setForceLabelsInsideClipPath( bool forceInside )
{
  if ( forceInside == mForceLabelsInsideClipPath )
    return;

  mForceLabelsInsideClipPath = forceInside;
  emit changed();
}

bool QgsLayoutItemMapItemClipPathSettings::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement settingsElem = document.createElement( QStringLiteral( "itemClippingSettings" ) );
  settingsElem.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  settingsElem.setAttribute( QStringLiteral( "forceLabelsInside" ), mForceLabelsInsideClipPath ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  settingsElem.setAttribute( QStringLiteral( "clippingType" ), QString::number( static_cast<int>( mFeatureClippingType ) ) );
  if ( mClipPathSource )
    settingsElem.setAttribute( QStringLiteral( "clipSource" ), mClipPathSource->uuid() );
  else
    settingsElem.setAttribute( QStringLiteral( "clipSource" ), QString() );

  element.appendChild( settingsElem );
  return true;
}

bool QgsLayoutItemMapItemClipPathSettings::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  const QDomElement settingsElem = element.firstChildElement( QStringLiteral( "itemClippingSettings" ) );

  mEnabled = settingsElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mForceLabelsInsideClipPath = settingsElem.attribute( QStringLiteral( "forceLabelsInside" ), QStringLiteral( "0" ) ).toInt();
  mFeatureClippingType = static_cast< QgsMapClippingRegion::FeatureClippingType >( settingsElem.attribute( QStringLiteral( "clippingType" ), QStringLiteral( "0" ) ).toInt() );
  mClipPathUuid = settingsElem.attribute( QStringLiteral( "clipSource" ) );

  return true;
}

void QgsLayoutItemMapItemClipPathSettings::finalizeRestoreFromXml()
{
  if ( !mClipPathUuid.isEmpty() )
  {
    if ( QgsLayoutItem *item = mMap->layout()->itemByUuid( mClipPathUuid, true ) )
    {
      setSourceItem( item );
    }
  }
}
