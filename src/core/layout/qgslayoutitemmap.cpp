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
#include "qgslayoutcontext.h"
#include "qgslayoututils.h"
#include "qgslayoutmodel.h"
#include "qgsmapthemecollection.h"
#include "qgsannotationmanager.h"
#include "qgsannotation.h"
#include "qgsmapsettingsutils.h"
#include "qgslayertree.h"
#include "qgsmaplayerref.h"
#include "qgsmaplayerlistutils.h"
#include "qgsmaplayerstylemanager.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

QgsLayoutItemMap::QgsLayoutItemMap( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  assignFreeId();


  if ( layout )
  {
    if ( QgsProject *project = layout->project() )
    {
      //get the color for map canvas background and set map background color accordingly
      int bgRedInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
      int bgGreenInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
      int bgBlueInt = project->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
      setBackgroundColor( QColor( bgRedInt, bgGreenInt, bgBlueInt ) );
    }
  }

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [ = ]
  {
    shapeChanged();
  } );

  mGridStack = qgis::make_unique< QgsLayoutItemMapGridStack >( this );
  mOverviewStack = qgis::make_unique< QgsLayoutItemMapOverviewStack >( this );

  if ( layout )
    connectUpdateSlot();
}

QgsLayoutItemMap::~QgsLayoutItemMap()
{
  if ( mPainterJob )
  {
    disconnect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsLayoutItemMap::painterJobFinished );
    mPainterJob->cancel(); // blocks
    mPainter->end();
  }
}

int QgsLayoutItemMap::type() const
{
  return QgsLayoutItemRegistry::LayoutMap;
}

QString QgsLayoutItemMap::stringType() const
{
  return QStringLiteral( "ItemMap" );
}

void QgsLayoutItemMap::assignFreeId()
{
  if ( !mLayout )
    return;

  QList<QgsLayoutItemMap *> mapsList;
  mLayout->layoutItems( mapsList );

  int maxId = -1;
  bool used = false;
  for ( QgsLayoutItemMap *map : qgis::as_const( mapsList ) )
  {
    if ( map == this )
      continue;

    if ( map->mMapId == mMapId )
      used = true;

    maxId = std::max( maxId, map->mMapId );
  }
  if ( !used )
    return;
  mMapId = maxId + 1;
  mLayout->itemsModel()->updateItemDisplayName( this );
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

double QgsLayoutItemMap::scale() const
{
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

#if 0 //TODO
  if ( mAtlasDriven && mAtlasScalingMode == Fixed && mComposition->atlasMode() != QgsComposition::AtlasOff )
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
#endif

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
  //Make sure the width/height ratio is the same as the current composer map extent.
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

QPolygonF QgsLayoutItemMap::visibleExtentPolygon() const
{
  QPolygonF poly;
  mapPolygon( mExtent, poly );
  return poly;
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
  mCrs = crs;
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
  for ( const QgsMapLayerRef &layerRef : qgis::as_const( mLayers ) )
  {
    if ( QgsMapLayer *layer = layerRef.get() )
    {
      QgsMapLayerStyle style;
      style.readFromLayer( layer );
      mLayerStyleOverrides.insert( layer->id(), style.xmlData() );
    }
  }
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

#if 0 //TODO
  if ( mAtlasDriven && mAtlasScalingMode == Fixed && mComposition->atlasMode() != QgsComposition::AtlasOff )
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
#endif

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
    if ( layer->dataProvider() && layer->dataProvider()->name() == QLatin1String( "wms" ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsLayoutItemMap::containsAdvancedEffects() const
{
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

void QgsLayoutItemMap::draw( QgsRenderContext &, const QStyleOptionGraphicsItem * )
{
}

bool QgsLayoutItemMap::writePropertiesToElement( QDomElement &composerMapElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
#if 0 //TODO - is this needed?
  composerMapElem.setAttribute( QStringLiteral( "id" ), mId );
#endif

  if ( mKeepLayerSet )
  {
    composerMapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerMapElem.setAttribute( QStringLiteral( "keepLayerSet" ), QStringLiteral( "false" ) );
  }

  if ( mDrawAnnotations )
  {
    composerMapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerMapElem.setAttribute( QStringLiteral( "drawCanvasItems" ), QStringLiteral( "false" ) );
  }

  //extent
  QDomElement extentElem = doc.createElement( QStringLiteral( "Extent" ) );
  extentElem.setAttribute( QStringLiteral( "xmin" ), qgsDoubleToString( mExtent.xMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "xmax" ), qgsDoubleToString( mExtent.xMaximum() ) );
  extentElem.setAttribute( QStringLiteral( "ymin" ), qgsDoubleToString( mExtent.yMinimum() ) );
  extentElem.setAttribute( QStringLiteral( "ymax" ), qgsDoubleToString( mExtent.yMaximum() ) );
  composerMapElem.appendChild( extentElem );

  if ( mCrs.isValid() )
  {
    QDomElement crsElem = doc.createElement( QStringLiteral( "crs" ) );
    mCrs.writeXml( crsElem, doc );
    composerMapElem.appendChild( crsElem );
  }

  // follow map theme
  composerMapElem.setAttribute( QStringLiteral( "followPreset" ), mFollowVisibilityPreset ? "true" : "false" );
  composerMapElem.setAttribute( QStringLiteral( "followPresetName" ), mFollowVisibilityPresetName );

  //map rotation
  composerMapElem.setAttribute( QStringLiteral( "mapRotation" ), QString::number( mMapRotation ) );

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
  composerMapElem.appendChild( layerSetElem );

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
    composerMapElem.appendChild( stylesElem );
  }

  //grids
  mGridStack->writeXml( composerMapElem, doc, context );

  //overviews
  mOverviewStack->writeXml( composerMapElem, doc, context );

  //atlas
  QDomElement atlasElem = doc.createElement( QStringLiteral( "AtlasMap" ) );
  atlasElem.setAttribute( QStringLiteral( "atlasDriven" ), mAtlasDriven );
  atlasElem.setAttribute( QStringLiteral( "scalingMode" ), mAtlasScalingMode );
  atlasElem.setAttribute( QStringLiteral( "margin" ), qgsDoubleToString( mAtlasMargin ) );
  composerMapElem.appendChild( atlasElem );

  return true;
}

bool QgsLayoutItemMap::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  mUpdatesEnabled = false;
#if 0 //TODO
  QString idRead = itemElem.attribute( QStringLiteral( "id" ), QStringLiteral( "not found" ) );
  if ( idRead != QLatin1String( "not found" ) )
  {
    mId = idRead.toInt();
    updateToolTip();
  }
#endif

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
  if ( !crsNodeList.isEmpty() )
  {
    QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    mCrs.readXml( crsElem );
  }
  else
  {
    mCrs = QgsCoordinateReferenceSystem();
  }

  //map rotation
  mMapRotation = itemElem.attribute( QStringLiteral( "mapRotation" ), QStringLiteral( "0" ) ).toDouble();

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
  mUpdatesEnabled = true;
  return true;
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
  if ( thisPaintRect.width() == 0 || thisPaintRect.height() == 0 )
    return;

  painter->save();
  painter->setClipRect( thisPaintRect );

  if ( mLayout->context().isPreviewRender() )
  {
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
      if ( !mPainterJob )
      {
        // this is the map's very first paint - trigger a cache update
        recreateCachedImageInBackground( style->matrix.m11() );
      }
    }
    else
    {
      if ( mCacheInvalidated )
      {
        // cache was invalidated - trigger a background update
        recreateCachedImageInBackground( style->matrix.m11() );
      }

      //Background color is already included in cached image, so no need to draw

      double imagePixelWidth = mCacheFinalImage->width(); //how many pixels of the image are for the map extent?
      double scale = rect().width() / imagePixelWidth;

      painter->save();

      painter->translate( mLastRenderedImageOffsetX + mXOffset, mLastRenderedImageOffsetY + mYOffset );
      painter->scale( scale, scale );
      painter->drawImage( 0, 0, *mCacheFinalImage );

      //restore rotation
      painter->restore();
    }
  }
  else
  {
    if ( mDrawing )
      return;

    mDrawing = true;
    QPaintDevice *paintDevice = painter->device();
    if ( !paintDevice )
      return;

    // Fill with background color
    if ( shouldDrawPart( Background ) )
    {
      drawMapBackground( painter );
    }

    QgsRectangle cExtent = extent();

    QSizeF size( cExtent.width() * mapUnitsToLayoutUnits(), cExtent.height() * mapUnitsToLayoutUnits() );

    painter->save();
    painter->translate( mXOffset, mYOffset );

    double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
    size *= dotsPerMM; // output size will be in dots (pixels)
    painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
    drawMap( painter, cExtent, size, paintDevice->logicalDpiX() );

    //restore rotation
    painter->restore();
    mDrawing = false;
  }

  painter->setClipRect( thisPaintRect, Qt::NoClip );

  if ( shouldDrawPart( OverviewMapExtent ) )
  {
    mOverviewStack->drawItems( painter );
  }
  if ( shouldDrawPart( Grid ) )
  {
    mGridStack->drawItems( painter );
  }

  //draw canvas items
  drawAnnotations( painter );

  if ( shouldDrawPart( Frame ) )
  {
    drawMapFrame( painter );
  }

  painter->restore();
}

int QgsLayoutItemMap::numberExportLayers() const
{
  return ( hasBackground() ? 1 : 0 )
         + layersToRender().length()
         + 1 // for grids, if they exist
         + 1 // for overviews, if they exist
         + ( hasFrame() ? 1 : 0 );
}

void QgsLayoutItemMap::setFrameStrokeWidth( const QgsLayoutMeasurement &width )
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
  QgsMapRendererCustomPainterJob job( mapSettings( extent, size, dpi ), painter );
  // Render the map in this thread. This is done because of problems
  // with printing to printer on Windows (printing to PDF is fine though).
  // Raster images were not displayed - see #10599
  job.renderSynchronously();
}

void QgsLayoutItemMap::recreateCachedImageInBackground( double viewScaleFactor )
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
  }

  Q_ASSERT( !mPainterJob );
  Q_ASSERT( !mPainter );
  Q_ASSERT( !mCacheRenderingImage );

  QgsRectangle ext = extent();
  double widthLayoutUnits = ext.width() * mapUnitsToLayoutUnits();
  double heightLayoutUnits = ext.height() * mapUnitsToLayoutUnits();

  int w = widthLayoutUnits * viewScaleFactor;
  int h = heightLayoutUnits * viewScaleFactor;

  // limit size of image for better performance
  if ( w > 5000 || h > 5000 )
  {
    if ( w > h )
    {
      w = 5000;
      h = w * heightLayoutUnits / widthLayoutUnits;
    }
    else
    {
      h = 5000;
      w = h * widthLayoutUnits / heightLayoutUnits;
    }
  }

  if ( w <= 0 || h <= 0 )
    return;

  mCacheRenderingImage.reset( new QImage( w, h, QImage::Format_ARGB32 ) );

  // set DPI of the image
  mCacheRenderingImage->setDotsPerMeterX( 1000 * w / widthLayoutUnits );
  mCacheRenderingImage->setDotsPerMeterY( 1000 * h / heightLayoutUnits );

  if ( hasBackground() )
  {
    //Initially fill image with specified background color. This ensures that layers with blend modes will
    //preview correctly
    mCacheRenderingImage->fill( backgroundColor().rgba() );
  }
  else
  {
    //no background, but start with empty fill to avoid artifacts
    mCacheRenderingImage->fill( QColor( 255, 255, 255, 0 ).rgba() );
  }

  mCacheInvalidated = false;
  mPainter.reset( new QPainter( mCacheRenderingImage.get() ) );
  QgsMapSettings settings( mapSettings( ext, QSizeF( w, h ), mCacheRenderingImage->logicalDpiX() ) );
  mPainterJob.reset( new QgsMapRendererCustomPainterJob( settings, mPainter.get() ) );
  connect( mPainterJob.get(), &QgsMapRendererCustomPainterJob::finished, this, &QgsLayoutItemMap::painterJobFinished );
  mPainterJob->start();
}

QgsMapSettings QgsLayoutItemMap::mapSettings( const QgsRectangle &extent, QSizeF size, int dpi ) const
{
  QgsExpressionContext expressionContext = createExpressionContext();
  QgsCoordinateReferenceSystem renderCrs = crs();

  QgsMapSettings jobMapSettings;
  jobMapSettings.setDestinationCrs( renderCrs );
  jobMapSettings.setExtent( extent );
  jobMapSettings.setOutputSize( size.toSize() );
  jobMapSettings.setOutputDpi( dpi );
  jobMapSettings.setBackgroundColor( Qt::transparent );
  jobMapSettings.setRotation( mEvaluatedMapRotation );
  if ( mLayout )
    jobMapSettings.setEllipsoid( mLayout->project()->ellipsoid() );

  //set layers to render
  QList<QgsMapLayer *> layers = layersToRender( &expressionContext );
  if ( mLayout && -1 != mLayout->context().currentExportLayer() )
  {
    const int layerIdx = mLayout->context().currentExportLayer() - ( hasBackground() ? 1 : 0 );
    if ( layerIdx >= 0 && layerIdx < layers.length() )
    {
      // exporting with separate layers (e.g., to svg layers), so we only want to render a single map layer
      QgsMapLayer *ml = layers[ layers.length() - layerIdx - 1 ];
      layers.clear();
      layers << ml;
    }
    else
    {
      // exporting decorations such as map frame/grid/overview, so no map layers required
      layers.clear();
    }
  }
  jobMapSettings.setLayers( layers );
  jobMapSettings.setLayerStyleOverrides( layerStyleOverridesToRender( expressionContext ) );

  if ( !mLayout->context().isPreviewRender() )
  {
    //if outputting layout, disable optimisations like layer simplification
    jobMapSettings.setFlag( QgsMapSettings::UseRenderingOptimization, false );
  }

  jobMapSettings.setExpressionContext( expressionContext );

  // layout-specific overrides of flags
  jobMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true ); // force vector output (no caching of marker images etc.)
  jobMapSettings.setFlag( QgsMapSettings::Antialiasing, true );
  jobMapSettings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  jobMapSettings.setFlag( QgsMapSettings::DrawSelection, false );
  jobMapSettings.setFlag( QgsMapSettings::UseAdvancedEffects, mLayout->context().flags() & QgsLayoutContext::FlagUseAdvancedEffects );

  jobMapSettings.datumTransformStore().setDestinationCrs( renderCrs );

  jobMapSettings.setLabelingEngineSettings( mLayout->project()->labelingEngineSettings() );

  return jobMapSettings;
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
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_scale" ), scale(), true ) );

  QgsRectangle currentExtent( extent() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent" ), QVariant::fromValue( QgsGeometry::fromRect( currentExtent ) ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_width" ), currentExtent.width(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_height" ), currentExtent.height(), true ) );
  QgsGeometry centerPoint = QgsGeometry::fromPointXY( currentExtent.center() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_center" ), QVariant::fromValue( centerPoint ), true ) );

  QgsCoordinateReferenceSystem mapCrs = crs();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs" ), mapCrs.authid(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_definition" ), mapCrs.toProj4(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_units" ), QgsUnitTypes::toString( mapCrs.mapUnits() ), true ) );

  context.appendScope( scope );

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
  QPolygonF poly = visibleExtentPolygon();
  poly.translate( -dx, -dy );
  return poly;
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
  double frameExtension = hasFrame() ? pen().widthF() / 2.0 : 0.0;

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
  mPainterJob.reset( nullptr );
  mPainter.reset( nullptr );
  mCacheFinalImage = std::move( mCacheRenderingImage );
  mLastRenderedImageOffsetX = 0;
  mLastRenderedImageOffsetY = 0;
  update();
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
      }
    } );

  }
  connect( mLayout, &QgsLayout::refreshed, this, &QgsLayoutItemMap::invalidateCache );
}

void QgsLayoutItemMap::updateToolTip()
{
  setToolTip( tr( "Map %1" ).arg( displayName() ) );
}

QList<QgsMapLayer *> QgsLayoutItemMap::layersToRender( const QgsExpressionContext *context ) const
{
  QgsExpressionContext scopedContext;
  if ( !context )
    scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  QList<QgsMapLayer *> renderLayers;

  if ( mFollowVisibilityPreset )
  {
    QString presetName = mFollowVisibilityPresetName;

    // preset name can be overridden by data-defined one
    presetName = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapStylePreset, *evalContext, presetName );

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

#if 0 //TODO
  //remove atlas coverage layer if required
  //TODO - move setting for hiding coverage layer to map item properties
  if ( mLayout->atlasMode() != QgsComposition::AtlasOff )
  {
    if ( mComposition->atlasComposition().hideCoverage() )
    {
      //hiding coverage layer
      int removeAt = renderLayers.indexOf( mComposition->atlasComposition().coverageLayer() );
      if ( removeAt != -1 )
      {
        renderLayers.removeAt( removeAt );
      }
    }
  }
#endif
  return renderLayers;
}

QMap<QString, QString> QgsLayoutItemMap::layerStyleOverridesToRender( const QgsExpressionContext &context ) const
{
  if ( mFollowVisibilityPreset )
  {
    QString presetName = mFollowVisibilityPresetName;

    // data defined preset name?
    presetName = mDataDefinedProperties.valueAsString( QgsLayoutObject::MapStylePreset, context, presetName );

    if ( mLayout->project()->mapThemeCollection()->hasMapTheme( presetName ) )
      return mLayout->project()->mapThemeCollection()->mapThemeStyleOverrides( presetName );
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

  context.painter()->save();
  context.painter()->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

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
  context.painter()->restore();
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
    QgsCoordinateTransform t( annotationCrs, crs() );
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
  if ( hasFrame() && p )
  {
    p->save();
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
    p->restore();
  }
}

void QgsLayoutItemMap::drawMapBackground( QPainter *p )
{
  if ( hasBackground() && p )
  {
    p->save();
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
    p->restore();
  }
}

bool QgsLayoutItemMap::shouldDrawPart( QgsLayoutItemMap::PartType part ) const
{
  int currentExportLayer = mLayout->context().currentExportLayer();

  if ( -1 == currentExportLayer )
  {
    //all parts of the composer map are visible
    return true;
  }

  int idx = numberExportLayers();
  if ( isSelected() )
  {
    --idx;
    if ( SelectionBoxes == part )
    {
      return currentExportLayer == idx;
    }
  }

  if ( hasFrame() )
  {
    --idx;
    if ( Frame == part )
    {
      return currentExportLayer == idx;
    }
  }
  --idx;
  if ( OverviewMapExtent == part )
  {
    return currentExportLayer == idx;
  }
  --idx;
  if ( Grid == part )
  {
    return currentExportLayer == idx;
  }
  if ( hasBackground() )
  {
    if ( Background == part )
    {
      return currentExportLayer == 0;
    }
  }

  return true; // for Layer
}

void QgsLayoutItemMap::refreshMapExtents( const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext;
  if ( !context )
    scopedContext = createExpressionContext();
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

  bool ok = false;
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
